typedef struct Battery_
{
	double percent;
	int hours, mins;
	int is_chr;
} Battery;

typedef struct Clock_
{
	int mon;
	int mday;
	int wday;
	int hour;
	int min;
	int sec;
	int is_pm;
} Clock;

typedef struct Network_
{
	double in, out;
} Network;

typedef struct Volume_
{
	int percent;
	int is_on;
	int icon;
	const char* desc;
} Volume;
