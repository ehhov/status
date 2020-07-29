#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <pthread.h>
#include <signal.h>

#include "tint2s.h"
#include "tuning.h"

int done = 0;
static int changed = 0;
static pthread_t status_thread;

void
refresh()
{
	changed = 1; /* in case something happens outside of the nanosleep() waiting time */
	pthread_kill(status_thread, SIGUSR1);
}

void
die(const char *fmt, ...)
{
	va_list arg;

	va_start(arg, fmt);
	vfprintf(stderr, fmt, arg);
	va_end(arg);

	fprintf(stderr, "\n");
	done = -1;
	refresh();
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

static void
finish(int signal)
{
	done = 1;
}

static void
sigusr(int signal)
{ /* do nothing, but still interrupt sleep */ }

static void
color(const char thecolor[7])
{
	printf(" foreground=\"#%s\"", thecolor);
}


int
main()
{
	pthread_t kb_thread, vol_thread;
	pthread_attr_t attr;
	struct timespec wait;
	sigset_t sigset;
	struct sigaction action;

	memset(&action, 0, sizeof(struct sigaction));
	action.sa_handler = sigusr;
	if (sigaction(SIGUSR1, &action, NULL)) {
		die("failed to set signal handler for SIGUSR1.");
		goto end;
	}
	memset(&action, 0, sizeof(struct sigaction));
	action.sa_handler = finish;
	if (sigaction(SIGINT, &action, NULL) \
	   || sigaction(SIGTERM, &action, NULL)) {
		die("failed to set signal handler for SIGINT and SIGTERM.");
		goto end;
	}

	if (sigemptyset(&sigset) \
	   || sigaddset(&sigset, SIGINT) \
	   || sigaddset(&sigset, SIGTERM) \
	   || pthread_sigmask(SIG_SETMASK, &sigset, NULL)) {
		die("failed to set signal mask for secondary threads.");
		goto end;
	}

	status_thread = pthread_self();
	if (pthread_attr_init(&attr) \
	   || pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE)) {
		die("failed to configure thread attributes (joinable).");
		goto end;
	}
	if (pthread_create(&kb_thread, NULL, layout_start, NULL) \
	   || pthread_create(&vol_thread, NULL, volume_start, NULL)) {
		die("failed to create layout and volume threads.");
		goto end;
	}
	if (pthread_attr_destroy(&attr)) {
		die("failed to free thread attributes.");
		goto join;
	}
	
	if (sigemptyset(&sigset) \
	   || pthread_sigmask(SIG_SETMASK, &sigset, NULL)) {
		die("failed to set signal mask for the main thread.");
		goto join;
	}

	netspeed(wlan); /* needed to save initial values */
	battery(bat); /* tint2s needs to know percent in advance */

	/* Infinite loop begins */
	while (!done)
	{
		printf(SP);	color("666666");
		printf("> %s: %s "PS, essid(wlan), netspeed(wlan));
		
		printf("%s", volume_icon());
		if (show_description)
			printf("%s ", volume_description());
		
		printf("%s", layout_icon());
		
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

		printf("\n");
		fflush(stdout);

		if (!changed) {
			clock_gettime(CLOCK_REALTIME, &wait);
			wait.tv_sec = interval - 1 - (wait.tv_sec % interval);
			wait.tv_nsec = 1e9 - wait.tv_nsec;
			nanosleep(&wait, NULL);
		}
		changed = 0;
	}
	/* Infinite loop ends */
	
join:
	pthread_kill(kb_thread, SIGUSR1);
	pthread_kill(vol_thread, SIGUSR1);
	pthread_join(kb_thread, NULL);
	pthread_join(vol_thread, NULL);

end:

	return done < 0 ? 1 : 0;
}
