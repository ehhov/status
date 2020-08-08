#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include <X11/Xlib.h>

#include "dwms.h"
#include "tuning.h"

int done = 0;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond;

void
refresh(int a)
{
	if (a) {
		pthread_mutex_lock(&mutex);
	} else {
		pthread_cond_signal(&cond);
		pthread_mutex_unlock(&mutex);
	}
}

void
die(const char *fmt, ...)
{
	va_list arg;

	va_start(arg, fmt);
	vfprintf(stderr, fmt, arg);
	va_end(arg);

	putc('\n', stderr);
	done = -1;
	refresh(1);
	refresh(0);
}

const char *
retprintf(const char *fmt, ...)
{
	static char str[1024];
	int r;
	va_list arg;

	va_start(arg, fmt);
	r = vsnprintf(str, sizeof(str), fmt, arg);
	va_end(arg);

	return (r < 0)? NULL : str;
}

static void *
waitsignals(void *sigset)
{
	int signal;

	sigwait(sigset, &signal);
	done = 1;
	refresh(1);
	refresh(0);
	return NULL;
}

static void
donothing(int signal)
{ }

static size_t
strcpypos(char *dest, size_t n, const char *src)
{
	dest[n - 1] = '\0';
	strncpy(dest, src, n);
	if (dest[n - 1]) {
		dest[n - 1] = '\0';
		return n;
	}
	return strlen(src);
}


int
main()
{
	const int max = 1024;
	size_t pos;
	char status[max];
	Display *dpy;
	Window root;

	pthread_t kb_thread, vol_thread, sig_thread;
	pthread_attr_t attr;
	pthread_condattr_t cattr;
	struct timespec wait;
	sigset_t sigset;
	struct sigaction action;

	sigemptyset(&sigset);
	sigaddset(&sigset, SIGINT);
	sigaddset(&sigset, SIGTERM);
	pthread_sigmask(SIG_SETMASK, &sigset, NULL);

	memset(&action, 0, sizeof(struct sigaction));
	action.sa_handler = donothing;
	sigaction(SIGUSR1, &action, NULL);

	pthread_condattr_init(&cattr);
	pthread_condattr_setclock(&cattr, CLOCK_REALTIME);
	pthread_cond_init(&cond, &cattr);
	pthread_condattr_destroy(&cattr);

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	if (pthread_create(&kb_thread, &attr, layout_start, NULL) \
	   || pthread_create(&vol_thread, &attr, volume_start, NULL) \
	   || pthread_create(&sig_thread, &attr, waitsignals, &sigset)) {
		pthread_attr_destroy(&attr);
		fputs("Failed to create additional threads.\n", stderr);
		done = -1;
		goto end;
	}
	pthread_attr_destroy(&attr);

	if (!(dpy = XOpenDisplay(NULL))) {
		fputs("Failed to open display.\n", stderr);
		pthread_kill(sig_thread, SIGTERM);
		done = -1;
		goto join;
	}
	root = DefaultRootWindow(dpy);

	netspeed(wlan); /* needed to save initial values */

	/* Infinite loop begins */
	pthread_mutex_lock(&mutex);
	while (!done) {
		pos = 0;
		pos += strcpypos(status+pos, max-pos, beginning);
		pos += snprintf(status+pos, max-pos, "%s:%s", essid(wlan), netspeed(wlan));
		pos += strcpypos(status+pos, max-pos, separator);
		pos += strcpypos(status+pos, max-pos, volume_text());
		if (show_description)
			pos += strcpypos(status+pos, max-pos, volume_description());
		pos += strcpypos(status+pos, max-pos, separator);
		pos += strcpypos(status+pos, max-pos, layout_text());
		pos += strcpypos(status+pos, max-pos, separator);
		pos += strcpypos(status+pos, max-pos, battery(bat));
		pos += strcpypos(status+pos, max-pos, separator);
		pos += strcpypos(status+pos, max-pos, datetime());
		pos += strcpypos(status+pos, max-pos, ending);

		XStoreName(dpy, root, status);
		XFlush(dpy);

		clock_gettime(CLOCK_REALTIME, &wait);
		wait.tv_sec += interval - wait.tv_sec % interval;
		wait.tv_nsec = 0;
		pthread_cond_timedwait(&cond, &mutex, &wait);
	}
	pthread_mutex_unlock(&mutex);
	/* Infinite loop ends */

	XStoreName(dpy, root, NULL);
	XCloseDisplay(dpy);

join:
	pthread_kill(kb_thread, SIGUSR1);
	pthread_kill(vol_thread, SIGUSR1);
	pthread_join(kb_thread, NULL);
	pthread_join(vol_thread, NULL);
	pthread_join(sig_thread, NULL);

end:
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&cond);

	return done < 0 ? 1 : 0;
}
