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

/* global variables */
int done = 0;
static pthread_t status_thread;

void
refresh()
{
	pthread_kill(status_thread, SIGUSR1);
}

void
die(const char* fmt, ...)
{
	va_list arg;
	va_start (arg, fmt);
	vfprintf (stderr, fmt, arg);
	va_end (arg);
	fprintf(stderr, "\n");
	done = 1;
	refresh();
}

static void
finish(int signal)
{
	done = 1;
}

static void
sigusr(int signal)
{	/* do nothing, but still interrupt sleep */ }

int
main()
{
	const int max = 1024;
	size_t pos;
	char status[max];
	unsigned int count = 0;
	Display* display;

	pthread_t kb_thread, vol_thread;
	struct timespec wait;
	sigset_t sigset;
	struct sigaction action;

	static Battery bat;
	static Clock now;
	static int kb_layout;
	static Network net;
	static Volume vol;

	if (sigemptyset(&sigset) ||\
			sigaddset(&sigset, SIGINT) ||\
			sigaddset(&sigset, SIGTERM) ||\
			pthread_sigmask(SIG_SETMASK, &sigset, NULL)) {
		die("failed to set signal mask for secondary threads.");
		goto end;
	}

	status_thread = pthread_self();
	if (pthread_create(&kb_thread, NULL, capture_layout, &kb_layout) ||\
			pthread_create(&vol_thread, NULL, volume, &vol)) {
		die("failed to create layout and volume threads.");
		goto end;
	}
	net = netspeed(wlan);
	
	if (sigemptyset(&sigset) ||\
			pthread_sigmask(SIG_SETMASK, &sigset, NULL)) {
		die("failed to set signal mask for the main thread.");
		goto join;
	}
	memset(&action, 0, sizeof(struct sigaction));
  action.sa_handler = sigusr;
  if (sigaction(SIGUSR1, &action, NULL)) {
		die("failed to set signal handler for SIGUSR1.");
		goto join;
	}
	memset(&action, 0, sizeof(struct sigaction));
	action.sa_handler = finish;
	if (sigaction(SIGINT, &action, NULL) ||\
			sigaction(SIGTERM, &action, NULL)) {
		die("failed to set signal handler for SIGINT and SIGTERM.");
		goto join;
	}

	display = XOpenDisplay(NULL);
	if (display == NULL) {
		die("failed to open display.");
		goto join;
	}

	/* Infinite loop begins */
	while (!done)
	{
		bat = battery(batn);
		now = datetime();
		net = netspeed(wlan);
		count++;

		pos = 0;
		pos += snprintf(status+pos, max-pos, beginning);
		pos += snprintf(status+pos, max-pos, "%s:%.2lf%s%.2lf", essid(wlan), \
				(net.in>0)? net.in:0, net_downup, (net.out>0)?net.out:0);
		pos += snprintf(status+pos, max-pos, separator);		
		pos += snprintf(status+pos, max-pos, "%s:%2d%s", vol.is_on?"V":"M", \
				vol.percent, vol.percent<100?"%":"");
		if (print_description)
			pos += snprintf(status+pos, max-pos, "%s%s%s",\
					strlen(vol.desc)>0?" (":"", vol.desc, strlen(vol.desc)>0?")":"");
		pos += snprintf(status+pos, max-pos, separator);		
		pos += snprintf(status+pos, max-pos, "%s", Xkb_group_text[kb_layout]);
		pos += snprintf(status+pos, max-pos, separator);		
		pos += snprintf(status+pos, max-pos, "%.2lf%%%s %02i:%02i%s", \
				bat.percent, (bat.is_chr? "+" : ""), bat.hours, bat.mins, \
				(bat.percent>10 || bat.is_chr)?"":(count%2==0?"":" LOW!") );
		pos += snprintf(status+pos, max-pos, separator);		
		pos += snprintf(status+pos, max-pos, "%s %s %d %d:%02d %s", \
				WeekDays[now.wday], Months[now.mon], now.mday, \
				now.hour, now.min, ampmstyle[now.is_pm]);
		pos += snprintf(status+pos, max-pos, ending);		
		
		XStoreName(display, DefaultRootWindow(display), status);
		XFlush(display);

		clock_gettime(CLOCK_REALTIME, &wait);
		wait.tv_sec = interval - 1 - (wait.tv_sec % interval);
		wait.tv_nsec = 1e9 - wait.tv_nsec;
		nanosleep(&wait, NULL);
	}
	/* Infinite loop ends */
	
	XStoreName(display, DefaultRootWindow(display), NULL);
	XCloseDisplay(display);

join:
	pthread_kill(kb_thread, SIGUSR1);
	pthread_kill(vol_thread, SIGUSR1);
	pthread_join(kb_thread, NULL);
	pthread_join(vol_thread, NULL);

end:

	return 0;
}
