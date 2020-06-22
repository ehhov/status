#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include <ifaddrs.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/wireless.h>

#include "structs.h"

extern void die(const char*, ...);

static const int len = 50;
const int id_len = IW_ESSID_MAX_SIZE+1;

Network
netspeed(const char* wlan)
{
	Network ret;
	static long int in1, in2, out1, out2;
	static struct timespec now, old;
	char path[len];
	FILE* file;

	old=now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	in1=in2; out1=out2;
	
	snprintf(path, len, "/sys/class/net/%s/statistics/rx_bytes", wlan);
	file=fopen(path, "r");
	if (file == NULL) {
		die("file does not exist (%s).", path);
		return ret;
	}
	fscanf(file, "%ld", &in2);
	fclose(file);

	snprintf(path, len, "/sys/class/net/%s/statistics/tx_bytes", wlan);
	file=fopen(path, "r");
	if (file == NULL) {
		die("file does not exist (%s).", path);
		return ret;
	}
	fscanf(file, "%ld", &out2);
	fclose(file);

	ret.in  = (in2-in1)>>10;
	ret.in /= 1024*(now.tv_sec+now.tv_nsec*1e-9-old.tv_sec-old.tv_nsec*1e-9);
	ret.out = (out2-out1)>>10;
	ret.out/= 1024*(now.tv_sec+now.tv_nsec*1e-9-old.tv_sec-old.tv_nsec*1e-9);

	return ret;
}

const char*
essid(const char* wlan)
{
	static char name[IW_ESSID_MAX_SIZE+1];
	int skfd;
	struct iwreq wrq;
	memset(&wrq, 0, sizeof(struct iwreq));
  wrq.u.essid.length = IW_ESSID_MAX_SIZE + 1;
	snprintf(wrq.ifr_name, sizeof(wrq.ifr_name), "%s", wlan);
	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (skfd < 0) {
		die("failed to open socket to get essid.");
		return NULL;
	}
  memset(name, 0, sizeof(name));
	wrq.u.essid.pointer = name;
	if (ioctl(skfd, SIOCGIWESSID, &wrq) < 0)
		die("failed to do something using ioctl()...");
	close(skfd);
	return name;
}
