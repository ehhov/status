#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>

#include "tint2s.h"
#include "tuning.h"

void refresh(int a);
void die(const char *fmt, ...);
const char *retprintf(const char *fmt, ...);
static void *waitsignals(void *sigset);
static void color(const char *thecolor);

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
	if (!done)
		done = 1;
	refresh(1);
	refresh(0);
	return NULL;
}

static void
color(const char *thecolor)
{
	printf(" foreground=\"#%s\"", thecolor);
}


int
main()
{
	pthread_t kb_thread, sig_thread;
	pthread_attr_t attr;
	pthread_condattr_t cattr;
	struct timespec wait;
	sigset_t sigset;
	int pipefd[2];

	sigemptyset(&sigset);
	sigaddset(&sigset, SIGINT);
	sigaddset(&sigset, SIGTERM);
	pthread_sigmask(SIG_SETMASK, &sigset, NULL);

	pthread_condattr_init(&cattr);
	pthread_condattr_setclock(&cattr, CLOCK_REALTIME);
	pthread_cond_init(&cond, &cattr);
	pthread_condattr_destroy(&cattr);

	pipe(pipefd);

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	if (pthread_create(&kb_thread, &attr, layout_start, &pipefd[0]) \
	   || pthread_create(&sig_thread, &attr, waitsignals, &sigset)) {
		pthread_attr_destroy(&attr);
		fputs("Failed to create additional threads.\n", stderr);
		done = -1;
		goto end;
	}
	pthread_attr_destroy(&attr);
	volume_start(); /* threaded */

	netspeed(wlan); /* needed to save initial values */
	battery(bat); /* tint2s needs to know percent in advance */

	/* Infinite loop begins */
	pthread_mutex_lock(&mutex);
	while (!done) {
		printf(SP); color("666666");
		printf("> %s: %s "PS, essid(wlan), netspeed(wlan));

		fputs(volume_icon(), stdout);
		if (show_description)
			printf("%s ", volume_description());

		fputs(layout_icon(), stdout);

		printf(SP);
		if (battery_percent() > 75)
			color("00ff00");
		else if (battery_percent() > 40)
			color("aadd00");
		else if (battery_percent() > 15)
			color("ffcc00");
		else
			color("ff5500");
		printf("> %s"PS, battery(bat));

		printf(" %s ", datetime());

		putc('\n', stdout);
		fflush(stdout);

		clock_gettime(CLOCK_REALTIME, &wait);
		wait.tv_sec += interval - wait.tv_sec % interval;
		wait.tv_nsec = 0;
		pthread_cond_timedwait(&cond, &mutex, &wait);
	}
	pthread_mutex_unlock(&mutex);
	/* Infinite loop ends */

	close(pipefd[0]);
	close(pipefd[1]);
	volume_stop();
	pthread_kill(sig_thread, SIGTERM);
	pthread_join(kb_thread, NULL);
	pthread_join(sig_thread, NULL);

end:
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&cond);

	return done < 0 ? 1 : 0;
}
