#include <time.h>

#include "structs.h"

Clock
datetime()
{
	Clock ret;
	struct timespec precise;
	clock_gettime(CLOCK_REALTIME, &precise);
	struct tm local;
	local = *localtime(&precise.tv_sec);
	ret.mon = local.tm_mon;
	ret.mday = local.tm_mday;
	ret.wday = local.tm_wday;
	ret.hour = local.tm_hour;
	ret.min = local.tm_min;
	ret.sec = local.tm_sec;
	ret.is_pm = ret.hour>=12 ? 1:0;
	ret.hour -= 12*ret.is_pm;
	if (ret.hour == 0)
		ret.hour = 12;
	return ret;
}

