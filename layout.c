#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <stdlib.h>
#include <sys/select.h>

extern int done;
extern void refresh(void);
extern void die(const char*, ...);
extern const char* retprintf(const char*, ...);

static const char* Xkb_text[]={"us","ru"};
static const char* Xkb_icon[]={"🇺🇸","🇷🇺"};
static int layout = 0; /* we don't know it before the first event... */

const char*
layout_text(void)
{
	return Xkb_text[layout];
}

const char*
layout_icon(void)
{
	return Xkb_icon[layout];
}

void*
layout_start(void* unused)
{
	Display* d;
	XEvent e;
	XkbEvent* ke;
	int fd;
	fd_set fds;

	d = XOpenDisplay(NULL);
	if (d == NULL) {
		die("layout thread failed to open display.");
		return NULL;
	}
	if (!XkbQueryExtension(d, 0, 0, 0, 0, 0)) {
		die("X Keyboard Extension is not present.");
		goto close;
	}
	if (!XkbSelectEventDetails(d, XkbUseCoreKbd, XkbStateNotify, \
	        XkbGroupStateMask, XkbGroupStateMask)) {
		die("layout thread failed to select X event details.");
		goto close;
	}
	XSync(d, False);

	fd = ConnectionNumber(d);
	
	while (!done)
	{
		FD_ZERO(&fds);
		FD_SET(fd, &fds);
		select(fd + 1, &fds, NULL, NULL, NULL);
		while (XPending(d)) {
			XNextEvent(d, &e);
			ke = (XkbEvent*) &e;
			if (ke->state.group != layout) {
				layout = ke->state.group;
				refresh();
			}
		}
	}

close:
	XCloseDisplay(d);

	return NULL;
}
