#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include <ifaddrs.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/wireless.h>

extern void die(const char *fmt, ...);
extern const char *retprintf(const char *fmt, ...);

const char *
netspeed(const char *wlan)
{
	static long int in1, in2, out1, out2;
	static struct timespec now, old;
	char path[50];
	FILE *file;

	old=now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	in1=in2; out1=out2;
	
	snprintf(path, sizeof(path), "/sys/class/net/%s/statistics/rx_bytes", wlan);
	file=fopen(path, "r");
	if (file == NULL) {
		die("File does not exist (%s). Check the argument (%s).", path, wlan);
		return NULL;
	}
	fscanf(file, "%ld", &in2);
	fclose(file);

	snprintf(path, sizeof(path), "/sys/class/net/%s/statistics/tx_bytes", wlan);
	file=fopen(path, "r");
	if (file == NULL) {
		die("File does not exist (%s). Check the argument (%s).", path, wlan);
		return NULL;
	}
	fscanf(file, "%ld", &out2);
	fclose(file);

	return retprintf("%.2lf↓↑%.2lf", ((in2 - in1)>>10) / 1024.0 \
	    / (now.tv_sec + now.tv_nsec*1e-9 - old.tv_sec - old.tv_nsec*1e-9), \
	    ((out2 - out1)>>10) / 1024.0 / (now.tv_sec + now.tv_nsec*1e-9 \
	      - old.tv_sec - old.tv_nsec*1e-9));
}

const char *
essid(const char *wlan)
{
	static char name[IW_ESSID_MAX_SIZE+1];
	int skfd;
	struct iwreq wrq;
	memset(&wrq, 0, sizeof(struct iwreq));
	wrq.u.essid.length = IW_ESSID_MAX_SIZE + 1;
	snprintf(wrq.ifr_name, sizeof(wrq.ifr_name), "%s", wlan);
	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (skfd < 0) {
		die("Failed to open socket to get essid. Check the argument (%s).", wlan);
		return NULL;
	}
	memset(name, 0, sizeof(name));
	wrq.u.essid.pointer = name;
	if (ioctl(skfd, SIOCGIWESSID, &wrq) < 0)
		die("Failed to do something using ioctl(). Check the argument (%s)", wlan);
	close(skfd);
	return name;
}
