#ifndef __LOCAL_H
#define __LOCAL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <signal.h>

#include <limits.h>
#include <time.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <sys/sysctl.h>
#include <sys/statfs.h>
#include <unistd.h>
#include <dirent.h>

#include <ncurses.h>

enum { GREEN = 1, YELLOW, RED, CYAN };

void coloron (int id);
void coloroff (int id);

struct SystemInfo
{
	int nprocs;
	int line;
	int colors;

	FILE *cpu;
	FILE *mtab;
	FILE *net;
	FILE *wireless;

	struct
	{
		struct
		{
			FILE *file;
			char location[128];
		} state, info;

		struct
		{
			DIR *dir;
			char location[128];
			struct dirent *content;
		} dir;

		int ok;
	} battery;

	struct
	{
		FILE *file;
		int prun;
		int ptotal;
		int pid;
		double loads[3];
	} loadavg;

	unsigned long long totalcpu;

	char processor[256];
	struct utsname uname;
	struct sysinfo sys;
};

extern struct SystemInfo global;

void loadavg (void);
void printbar (const char *str, double perc, int reverse);

/* cpu.c */
struct cpu_t
{
	unsigned long long user;
	unsigned long long system;
	unsigned long long idle;
	unsigned long long iowait;
};

void cpubar (int id);

/* ram.c */
void rambar (void);
void swapbar (void);

/* disk.c */
void diskbar (void);

/* battery.c */
void batterybar (void);

/* net.c */
struct net_t
{
	char *name;			/* name of interface */
	time_t time;		/* time when data were exchanged */
	unsigned long rx;	/* data received in bytes */
	unsigned long tx;	/* data transmitted in bytes */
};

void netbar (void);
void wirelessbar (void);

/* top.c */
struct process
{
	pid_t pid;
	char *name;
	time_t timestamp;

	unsigned long utime;
	unsigned long ktime;
	unsigned long o_utime;
	unsigned long o_ktime;
	unsigned long cpu_time;

	unsigned int vsize;
	unsigned int rss;

	double cpu;
	double mem;

	struct process *next;
	struct process *prev;
};

void print_proc (void);
void free_all_proc (void);

/* print.c */
void print_color (int y, int x, char *string);

/* CONFIG */

struct NetworkInterface
{
	struct net_t onet;
	char *name;
};

#endif /* __LOCAL_H */
