/* Appearance */
static const char beginning[] = "[", separator[] = "][", ending[] = "]";
static const char* Months[12]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
static const char* WeekDays[7]={"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
static const char* ampmstyle[2]={"am","pm"};
static const char* net_downup="↓↑";
static const char* Xkb_group_text[2]={"us","ru"};
static const int print_description = 1;
static const int interval = 5;

static const char* Xkb_group_icon[2]={"🇺🇸","🇷🇺"};
static const char* volume_icon[4]={"🔇","🔈","🔉","🔊"};

/* Variables */
static const char* batn = "BAT0";
static const char* wlan = "wlp58s0";

/*
--------------------- NOTES ---------------------
battery instance can be found in
	/sys/class/power_supply/
wlan interface can be found in
	/sys/class/net/

for many symbols see
  https://jrgraphix.net/r/Unicode/
	https://emojipedia.org
useful symbols:
  🔇🔈🔉🔊 -- emoji style speaker
       -- text style speaker
  ░▒▓█
	• ·┄
  🇺🇸 🇷🇺
  ☀🔅🔆
  ↓↑ ⇅ ⇵
-------------------------------------------------
*/
