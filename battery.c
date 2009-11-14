#include "local.h"

void batterybar (void)
{
	char buf[256];
	int total, left;
	double perc;

	rewind (global.battery.info.file);
	rewind (global.battery.state.file);
	
	while (fgets (buf, sizeof buf, global.battery.info.file))
	{
		if (!strncmp (buf, "design capacity:", 16))
		{
			if (1 != sscanf (buf, "design capacity: %d mAh", &total))
			{
				fprintf (stderr, "battery(): Wrong field count in %s.\n",
						global.battery.state.location);
				return;
			}
		}
	}

	while (fgets (buf, sizeof buf, global.battery.state.file))
	{
		if (!strncmp (buf, "remaining capacity:", 19))
		{
			if (1 != sscanf (buf, "remaining capacity: %d mAh", &left))
			{
				fprintf (stderr, "battery(): Wrong field count in %s.\n",
						global.battery.state.location);
				return;
			}
		}
	}

	perc = (double) left * 100.0 / (double) total;
	printbar ("Bat", perc, 0);
}
