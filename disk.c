#include "local.h"

void diskbar (void)
{
	struct statfs disk;
	double total, used, perc;
	char buf[256];
	char hd[128];
	char mount[128];

	rewind (global.mtab);
	while (fgets (buf, sizeof buf, global.mtab))
	{
		if (!strncmp (buf, "/dev/", 5))
		{
			if (2 == sscanf (buf, "/dev/%s %s", hd, mount))
			{
				if (!strcmp (mount, "/"))
					continue;

				if (statfs (mount, &disk) == -1 || disk.f_blocks == 0)
					continue;

				total = (double) disk.f_blocks;
				used = total - (double) disk.f_bfree;
				perc = used / total * 100.0;

				printbar (hd, perc, 0);
			}
		}
		else if (!strncmp (buf, "rootfs", 6))
		{
			if (1 == sscanf (buf, "rootfs %s", mount))
			{
				if (statfs (mount, &disk) == -1 || disk.f_blocks == 0)
					continue;

				total = (double) disk.f_blocks;
				used = total - (double) disk.f_bfree;
				perc = used / total * 100.0;

				printbar ("RootFS", perc, 0);
			}
		}
	}
}
