#include "structs.h"

Battery battery(const char*);
Clock datetime();
void* capture_layout(void*);
Network netspeed(const char*);
const char* essid(const char*);
void* volume(void*);

#define SP "<span"
#define PS "</span>"

static void color (const char[7]);
