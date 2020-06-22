#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <stdlib.h>
#include <sys/select.h>

extern int done;
extern void die(const char*, ...);
extern void refresh(void);

void*
capture_layout(void* voidlayout)
{
	Display* d;
	XEvent e;
	XkbEvent* ke;
	int fd;
	fd_set fds;
	int* layout = voidlayout;

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
			if (ke->state.group != *layout) {
				*layout = ke->state.group;
				refresh();
			}
		}
	}

close:
	XCloseDisplay(d);

	return NULL;
}
