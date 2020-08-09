#include <stdlib.h>
#include <string.h>
#include <pulse/pulseaudio.h>

#define LEN 128

extern void refresh(int a);
extern void die(const char *fmt, ...);
extern const char *retprintf(const char *fmt, ...);

const char *volume_text(void);
const char *volume_icon(void);
const char *volume_description(void);
void volume_start(void);
void volume_stop(void);
static void save_info(pa_context *c, const pa_sink_info *i, int eol, void *unused);
static void context_subscribe_callback(pa_context *c, pa_subscription_event_type_t t, uint32_t idx, void *unused);
static void context_state_callback(pa_context *c, void *unused);

static const char *volume_icons[4] = {"🔇", "🔈", "🔉", "🔊"};

static int percent, mute, icon = 0;
static char descriptionstr[LEN + 3] = "\0(";
static pa_threaded_mainloop *loop;
static pa_mainloop_api *api;
static pa_context *context;

const char *
volume_text(void)
{
	return retprintf("%s:%2d%s", mute ? "M" : "V", percent, percent < 100 ? "%" : "");
}

const char *
volume_icon(void)
{
	return volume_icons[icon];
}

const char *
volume_description(void)
{
	return descriptionstr;
}

static void
save_info(pa_context *c, const pa_sink_info *i, int eol, void *unused)
{
	if (eol > 0 || !i)
		return;
	if (eol < 0) {
		die("PulseAudio sink event callback got eol=%d.", eol);
		return;
	}

	static int dlen, isfirst = 1;
	int volume = pa_cvolume_avg(&i->volume) * 100.0 / PA_VOLUME_NORM + 0.5;
	char *description = descriptionstr + 2;
	int cmp = (dlen > 0) ? strncmp(i->description, description, dlen) : 1;
	
	if (volume != percent || i->mute != mute || cmp) {
		refresh(1);
		if (cmp) {
			strncpy(description, i->description, LEN);
			dlen = strlen(i->description);
			if (dlen > LEN)
				dlen = LEN;
			descriptionstr[0] = isfirst ? (isfirst = 0, '\0') : ' ';
			descriptionstr[dlen + 2] = ')';
			descriptionstr[dlen + 3] = '\0';
		} else {
			descriptionstr[0] = '\0';
		}
		percent = volume;
		mute = i->mute;
		if (mute) icon = 0;
		else if (volume < 30) icon = 1;
		else if (volume < 90) icon = 2;
		else icon = 3;
		refresh(0);
	}
}

static void
context_subscribe_callback(pa_context *c, pa_subscription_event_type_t t, uint32_t idx, void *unused)
{
	if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) != PA_SUBSCRIPTION_EVENT_CHANGE)
		return;

	pa_subscription_event_type_t actual = t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK;
	pa_operation *o;

	switch (actual) {
	case PA_SUBSCRIPTION_EVENT_SINK:
		o = pa_context_get_sink_info_by_index(c, idx, save_info, NULL /* user */);
		pa_operation_unref(o);
		break;
	case PA_SUBSCRIPTION_EVENT_SERVER:
		o = pa_context_get_sink_info_by_name(c, "@DEFAULT_SINK@", save_info, NULL /* user */);
		pa_operation_unref(o);
		break;
	default:
		break;
	}
}

static void
context_state_callback(pa_context *c, void *unused)
{
	pa_operation *o;
	switch (pa_context_get_state(c)) {
	case PA_CONTEXT_READY:
		pa_context_set_subscribe_callback(c, context_subscribe_callback, NULL /* user */);
		o = pa_context_get_sink_info_by_name(c, "@DEFAULT_SINK@", save_info, NULL /* user */);
		pa_operation_unref(o);
		o = pa_context_subscribe(c, PA_SUBSCRIPTION_MASK_SINK | PA_SUBSCRIPTION_MASK_SERVER, NULL, NULL);
		pa_operation_unref(o);
		break;
	case PA_CONTEXT_FAILED:
		die("PulseAudio context failed.");
		break;
	default:
		break;
	}
}

void
volume_start(void)
{
	loop = pa_threaded_mainloop_new();
	api = pa_threaded_mainloop_get_api(loop);

	context = pa_context_new(api, NULL);
	pa_context_set_state_callback(context, context_state_callback, NULL /* user */);
	if (pa_context_connect(context, NULL, PA_CONTEXT_NOAUTOSPAWN | PA_CONTEXT_NOFAIL, NULL) < 0)
		die("PulseAudio context connection unsuccessful.");
	if (pa_threaded_mainloop_start(loop) < 0)
		die("PulseAudio failed to start main loop.");
}

void
volume_stop(void)
{
	pa_context_disconnect(context);
	pa_context_unref(context);
	pa_threaded_mainloop_stop(loop);
	pa_threaded_mainloop_free(loop);
}
