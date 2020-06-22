#include <stdio.h>
#include <string.h>

#include "structs.h"

extern void die(const char*, ...);

static const int len = 50;

Battery
battery(const char* bat)
{
	FILE* file;
	int now, full, current;
	char path[len], chr[12];
	Battery ret;
	
	snprintf(path, len, "/sys/class/power_supply/%s/charge_now", bat);
	file = fopen(path, "r");
	if (file == NULL) {
		die("file does not exist (%s).", path);
		return ret;
	}
	fscanf(file, "%d", &now);
	fclose(file);
	
	snprintf(path, len, "/sys/class/power_supply/%s/charge_full", bat);
	file = fopen(path, "r");
	if (file == NULL) {
		die("file does not exist (%s).", path);
		return ret;
	}
	fscanf(file, "%d", &full);
	fclose(file);

	snprintf(path, len, "/sys/class/power_supply/%s/current_now", bat);
	file = fopen(path, "r");
	if (file == NULL) {
		die("file does not exist (%s).", path);
		return ret;
	}
	fscanf(file, "%d", &current);
	fclose(file);
	if (current == 0) current = 1;

	snprintf(path, len, "/sys/class/power_supply/%s/status", bat);
	file = fopen(path, "r");
	if (file == NULL) {
		die("file does not exist (%s).", path);
		return ret;
	}
	fscanf(file, "%12s", &chr[0]);
	fclose(file);
	if (strcmp(chr, "Charging")) ret.is_chr = 0;
	else ret.is_chr = 1;

	if (ret.is_chr == 1) {
		ret.hours = (full-now) / current;
		ret.mins = (full-now) % current * 60 / current;
	}	else {
		ret.hours = now / current;
		ret.mins = now % current * 60 / current;
	}
	ret.percent = now * 100.0 / full;

	return ret;
}
