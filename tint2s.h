const char *battery(const char *bat);
int battery_percent(void); /* only in tint2s.h */

const char *datetime(void);

const char *layout_text(void);
const char *layout_icon(void);
void *layout_start(void *fd);

const char *netspeed(const char *wlan);
const char *essid(const char *wlan);

const char *volume_text(void);
const char *volume_icon(void);
const char *volume_description(void);
void volume_start(void);
void volume_stop(void);

#define SP "<span"
#define PS "</span>"
