#include <stdio.h>
#include <string.h>

extern void die(const char *fmt, ...);
extern const char *retprintf(const char *fmt, ...);

static int percent; /* only for tint2 */

const char *
battery(const char *bat)
{
	int now, full, current;
	char path[50], chr[12];
	FILE *file;

	snprintf(path, sizeof(path), "/sys/class/power_supply/%s/charge_now", bat);
	file = fopen(path, "r");
	if (file == NULL) {
		die("file does not exist (%s). check the argument (%s).", path, bat);
		return NULL;
	}
	fscanf(file, "%d", &now);
	fclose(file);

	snprintf(path, sizeof(path), "/sys/class/power_supply/%s/charge_full", bat);
	file = fopen(path, "r");
	if (file == NULL) {
		die("file does not exist (%s). check the argument (%s).", path, bat);
		return NULL;
	}
	fscanf(file, "%d", &full);
	fclose(file);

	snprintf(path, sizeof(path), "/sys/class/power_supply/%s/current_now", bat);
	file = fopen(path, "r");
	if (file == NULL) {
		die("file does not exist (%s). check the argument (%s).", path, bat);
		return NULL;
	}
	fscanf(file, "%d", &current);
	fclose(file);
	if (current == 0) current = 1;

	snprintf(path, sizeof(path), "/sys/class/power_supply/%s/status", bat);
	file = fopen(path, "r");
	if (file == NULL) {
		die("file does not exist (%s). check the argument (%s).", path, bat);
		return NULL;
	}
	fscanf(file, "%12s", &chr[0]);
	fclose(file);

	if (!strcmp(chr, "Full")) {
		strcpy(chr, " Charged");
	} else if (!strcmp(chr, "Charging")) {
		snprintf(chr, 12, "+ %02d:%02d", (full-now) / current, \
		    (full-now) % current * 60 / current);
	} else {
		snprintf(chr, 12, " %02d:%02d", now / current, \
		    now % current * 60 / current);
	}

	percent = 100 * now / full; /* only for tint2 */

	return retprintf("%.2lf%%%s", 100.0 * now / full, chr);
}

/* only for tint2 */
int
battery_percent(void)
{
	return percent;
}
