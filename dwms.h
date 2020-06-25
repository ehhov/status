const char *battery(const char *bat);

const char *datetime(void);

const char *layout_text(void);
const char *layout_icon(void);
void *layout_start(void *unused);

const char *netspeed(const char *wlan);
const char *essid(const char *wlan);

const char *volume_text(void);
const char *volume_icon(void);
const char *volume_description(void);
void *volume_start(void *unused);
