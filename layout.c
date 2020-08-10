#include <stdlib.h>
#include <poll.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>

extern int done;
extern void refresh(int a);
extern void die(const char *fmt, ...);

const char *layout_text(void);
const char *layout_icon(void);
void *layout_start(void *fd);
static int getlayout(Display *dpy);

static const char *Xkb_text[]={"us","ru"};
static const char *Xkb_icon[]={"🇺🇸","🇷🇺"};
static int layout = 0;

const char *
layout_text(void)
{
	return Xkb_text[layout];
}

const char *
layout_icon(void)
{
	return Xkb_icon[layout];
}

static int
getlayout(Display *dpy)
{
	XkbStateRec state;

	if (XkbGetState(dpy, XkbUseCoreKbd, &state))
		return 0;
	return state.group;
}

void *
layout_start(void *fd)
{
	Display *d;
	XEvent e;
	XkbEvent *ke = (XkbEvent *) &e;
	struct pollfd fds[2];

	if (!(d = XOpenDisplay(NULL))) {
		die("Layout thread failed to open display.");
		return NULL;
	}
	if (!XkbQueryExtension(d, 0, 0, 0, 0, 0)) {
		die("X Keyboard Extension is not present.");
		goto close;
	}
	if (!XkbSelectEventDetails(d, XkbUseCoreKbd, XkbStateNotify, \
	                           XkbGroupStateMask, XkbGroupStateMask)) {
		die("Layout thread failed to select Xkb event details.");
		goto close;
	}
	XSync(d, False);

	refresh(1);
	layout = getlayout(d);
	refresh(0);

	fds[0].fd = ConnectionNumber(d);
	fds[0].events = POLLIN;
	fds[1].fd = *(int *)fd;
	fds[1].events = POLLIN;

	while (!done) {
		poll(fds, 2, -1);
		while (XPending(d)) {
			XNextEvent(d, &e);
			if (ke->state.group != layout) {
				refresh(1);
				layout = ke->state.group;
				refresh(0);
			}
		}
	}

close:
	XCloseDisplay(d);

	return NULL;
}
