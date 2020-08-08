#include <stdio.h>
#include <string.h>

extern void die(const char *fmt, ...);
extern const char *retprintf(const char *fmt, ...);

const char *battery(const char *bat);
int battery_percent(void); /* only for tint2 */
static void readvar(const char *bat, const char *name, const char *fmt, void *var);

static int percent; /* only for tint2 */

static void
readvar(const char *bat, const char *name, const char *fmt, void *var)
{
	FILE *file;
	char path[50];

	snprintf(path, sizeof(path), "/sys/class/power_supply/%s/%s", bat, name);
	if (!(file = fopen(path, "r"))) {
		die("File does not exist: %s. Check the argument: %s.", path, bat);
		return;
	}
	fscanf(file, fmt, var);
	fclose(file);
}

const char *
battery(const char *bat)
{
	int now, full, current;
	char chr[12];

	readvar(bat, "charge_now", "%d", &now);
	readvar(bat, "charge_full", "%d", &full);
	readvar(bat, "current_now", "%d", &current);
	readvar(bat, "status", "%12s", chr);

	if (chr[0] == 'F')
		strcpy(chr, " Charged");
	else if (chr[0] == 'C')
		snprintf(chr, 12, "+ %02d:%02d", (full-now) / current, \
		         (full-now) % current * 60 / current);
	else
		snprintf(chr, 12, " %02d:%02d", now / current, \
		         now % current * 60 / current);

	percent = 100 * now / full; /* only for tint2 */

	return retprintf("%.2lf%%%s", 100.0 * now / full, chr);
}

/* only for tint2 */
int
battery_percent(void)
{
	return percent;
}
