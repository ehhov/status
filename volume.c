#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pulse/pulseaudio.h>

#define LEN 128

extern int done;
extern void refresh(void);
extern void die(const char*, ...);
extern const char* retprintf(const char*, ...);

static const char* volume_icons[4]={"🔇","🔈","🔉","🔊"};

static int percent, mute, icon = 0;
static char descriptionstr[LEN+3] = "";
static pa_mainloop* loop;
static pa_mainloop_api* api;
static pa_context* context;

static void context_state_callback(pa_context*, void*);
static void context_subscribe_callback(pa_context*, pa_subscription_event_type_t, uint32_t, void*);
static void save_info(pa_context*, const pa_sink_info*, int, void*);

const char*
volume_text(void)
{
	return retprintf("%s:%2d%s", mute?"M":"V", percent, percent<100?"%":"");
}

const char*
volume_icon(void)
{
	return volume_icons[icon];
}

const char*
volume_description(void)
{
	return descriptionstr;
}

void
save_info(pa_context* c, const pa_sink_info* i, int eol, void* unused)
{
	if (eol>0 || !i)
		return;
	if (eol<0) {
		die("pulseaudio sink event callback got eol=%d.", eol);
		return;
	}

	int volume = pa_cvolume_avg(&i->volume) * 100.0 / PA_VOLUME_NORM + 0.5;
	static char description[LEN];
	int cmp = strcmp(i->description, description);
	
	if (volume != percent || i->mute != mute || cmp) {
		if (cmp) {
			strncpy(description, i->description, LEN);
			description[LEN-1] = '\0';
			snprintf(descriptionstr, LEN+3, " (%s)", description);
		} else {
			descriptionstr[0] = '\0';
		}
		percent = volume;
		mute = i->mute;
		if (mute) icon=0;
		else if (volume < 30) icon=1;
		else if (volume < 90) icon=2;
		else icon=3;
		refresh();
	}
}

void
context_subscribe_callback(pa_context* c, pa_subscription_event_type_t t, uint32_t idx, void* unused)
{
	if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) != PA_SUBSCRIPTION_EVENT_CHANGE)
		return;
	pa_subscription_event_type_t actual = t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK;
	switch (actual) {
		case PA_SUBSCRIPTION_EVENT_SINK:
			{
				pa_operation* o = pa_context_get_sink_info_by_index(c, idx, save_info, NULL /* user */);
				pa_operation_unref(o);
			} break;
		case PA_SUBSCRIPTION_EVENT_SERVER:
			{
				pa_operation* o = pa_context_get_sink_info_by_name(c, "@DEFAULT_SINK@", save_info, NULL /* user */);
				pa_operation_unref(o);
			} break;
		default:
			break;
	}
}

void
context_state_callback(pa_context* c, void* unused)
{
	switch (pa_context_get_state(c)) {
		case PA_CONTEXT_READY:
			{
				pa_context_set_subscribe_callback(c, context_subscribe_callback, NULL /* user */);
				pa_operation* o;
				o = pa_context_get_sink_info_by_name(c, "@DEFAULT_SINK@", save_info, NULL /* user */);
				pa_operation_unref(o);
				o = pa_context_subscribe(c, PA_SUBSCRIPTION_MASK_SINK | PA_SUBSCRIPTION_MASK_SERVER, NULL, NULL);
				pa_operation_unref(o);
			} break;
		case PA_CONTEXT_FAILED:
			die("pulseaudio context failed.");
			break;
		default:
			break;
	}
}

void*
volume_start(void* unused)
{
	loop = pa_mainloop_new();
	api = pa_mainloop_get_api(loop);

	context = pa_context_new(api, NULL);
	pa_context_set_state_callback(context, context_state_callback, NULL /* user */);
	if (pa_context_connect(context, NULL, PA_CONTEXT_NOAUTOSPAWN | PA_CONTEXT_NOFAIL, NULL) < 0)
		die("pulseaudio context connection unsuccessful.");

	while (!done) {
		if (pa_mainloop_iterate(loop, 1, NULL) < 0)
			die("pulseaudio main loop iteration error.");
	}

	pa_context_disconnect(context);
	pa_context_unref(context);
	pa_mainloop_free(loop);

	return NULL;
}
