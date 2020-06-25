const char* battery(const char*);
int battery_percent(void); /* only in tint2s.h */

const char* datetime(void);

const char* layout_text(void);
const char* layout_icon(void);
void* layout_start(void*);

const char* netspeed(const char*);
const char* essid(const char*);

const char* volume_text(void);
const char* volume_icon(void);
const char* volume_description(void);
void* volume_start(void*);

#define SP "<span"
#define PS "</span>"

static void color (const char[7]);
