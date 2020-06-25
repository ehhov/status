/* Appearance */
static const char beginning[] = "[", separator[] = "][", ending[] = "]";
static const int show_description = 1;
static const int interval = 5;

/* Variables */
static const char *bat = "BAT0";
static const char *wlan = "wlp58s0";

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
