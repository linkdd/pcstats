#include "local.h"

struct cpu_t *ocpu, *ncpu;

void cpubar (int id)
{
	char buf[256];
	char tmp[10];

	double total, load;
	unsigned long long unice;
	struct cpu_t mcpu;

	snprintf (tmp, 9, "CPU%d", id);
	
	rewind (global.cpu);
	while (fgets (buf, sizeof buf, global.cpu))
	{
		if (!strncasecmp (buf, tmp, strlen (tmp)))
		{
			if (5 != sscanf (buf, "%*s %llu %llu %llu %llu %llu",
						&ncpu[id].user, &unice, &ncpu[id].system,
						&ncpu[id].idle, &ncpu[id].iowait))
			{
				fprintf (stderr, "cpu(): Wrong field count in /proc/stat\n");
				continue;
			}
			ncpu[id].user += unice;

			mcpu.user   = ncpu[id].user - ocpu[id].user;
			mcpu.system = ncpu[id].system - ocpu[id].system;
			mcpu.idle   = ncpu[id].idle - ocpu[id].idle;
			mcpu.iowait = ncpu[id].iowait - ocpu[id].iowait;

			global.totalcpu = (mcpu.user + mcpu.system + mcpu.idle + mcpu.iowait);
			total = global.totalcpu / 100.0;
			load = (mcpu.user + mcpu.system + mcpu.iowait) / total;

			printbar (tmp, load, 0);
			ocpu[id] = ncpu[id];
		}
	}
}
