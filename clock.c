#include <time.h>

extern const char *retprintf(const char *fmt, ...);

static const char *Months[12]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
static const char *WeekDays[7]={"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
static const char *ampmstyle[2]={"am","pm"};

const char *
datetime(void)
{
	int pm;
	struct timespec precise;
	struct tm local;
	/* usual time_t and time() is 0.0...01 second slow */
	clock_gettime(CLOCK_REALTIME, &precise);
	localtime_r(&precise.tv_sec, &local);
	pm = local.tm_hour>=12 ? 1:0;
	local.tm_hour -= 12*pm;
	if (local.tm_hour == 0)
		local.tm_hour = 12;

	return retprintf("%s %s %d %d:%02d %s", WeekDays[local.tm_wday], \
	    Months[local.tm_mon], local.tm_mday, local.tm_hour, local.tm_min, \
	    ampmstyle[pm]);
}
