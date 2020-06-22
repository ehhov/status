#include <stdlib.h>
#include <string.h>
#include <pulse/pulseaudio.h>

#include "structs.h"

#define LEN 128

extern int done;
extern void refresh(void);
extern void die(const char*, ...);

void* volume(void*);
static void context_state_callback(pa_context*, void*);
static void context_subscribe_callback(pa_context*, pa_subscription_event_type_t, uint32_t, void*);
static void save_info(pa_context*, const pa_sink_info*, int, void*);

static pa_mainloop* loop;
static pa_mainloop_api* api;
static pa_context* context;

void
save_info(pa_context* c, const pa_sink_info* i, int eol, void* user)
{
	if (eol>0 || !i)
		return;
	if (eol<0) {
		die("pulseaudio sink event callback got eol=%d.", eol);
		return;
	}

	Volume* data = user;
	int volume = pa_cvolume_avg(&i->volume) * 100.0 / PA_VOLUME_NORM + 0.5;
	static char olddescription[LEN];
	static const char none[] = "";
	int cmp = strcmp(i->description, olddescription);
	
	if (volume != data->percent || i->mute != (!data->is_on) || cmp) {
		if (cmp) {
			strncpy(olddescription, i->description, LEN);
			olddescription[LEN-1] = '\0';
			data->desc = olddescription;
		} else {
			data->desc = none;
		}
		data->percent = volume;
		data->is_on = !i->mute;
		if (i->mute) data->icon=0;
		else if (volume < 30) data->icon=1;
		else if (volume < 90) data->icon=2;
		else data->icon=3;
		refresh();
	}
}

void
context_subscribe_callback(pa_context* c, pa_subscription_event_type_t t, uint32_t idx, void* user)
{
	if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) != PA_SUBSCRIPTION_EVENT_CHANGE)
		return;
	pa_subscription_event_type_t actual = t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK;
	switch (actual) {
		case PA_SUBSCRIPTION_EVENT_SINK:
			{
				pa_operation* o = pa_context_get_sink_info_by_index(c, idx, save_info, user /* user */);
				pa_operation_unref(o);
			} break;
		case PA_SUBSCRIPTION_EVENT_SERVER:
			{
				pa_operation* o = pa_context_get_sink_info_by_name(c, "@DEFAULT_SINK@", save_info, user /* user */);
				pa_operation_unref(o);
			} break;
		default:
			break;
	}
}

void
context_state_callback(pa_context* c, void* user)
{
	switch (pa_context_get_state(c)) {
		case PA_CONTEXT_READY:
			{
				pa_context_set_subscribe_callback(c, context_subscribe_callback, user /* user */);
				pa_operation* o;
				o = pa_context_get_sink_info_by_name(c, "@DEFAULT_SINK@", save_info, user /* user */);
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
volume(void* data)
{
	loop = pa_mainloop_new();
	api = pa_mainloop_get_api(loop);

	context = pa_context_new(api, NULL);
	pa_context_set_state_callback(context, context_state_callback, data /* user */);
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
