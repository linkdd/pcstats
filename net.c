#include "local.h"

#define __NET__
#include "config.h"

void netbar (void)
{
	struct net_t net;
	char buf[256];
	char *p;
	int i;

	rewind (global.net);
	while (fgets (buf, sizeof buf, global.net))
	{
		if (strchr (buf, ':') == NULL)
			continue;

		for (p = buf; *p == ' '; ++p);

		net.name = strdup (p);
		p = strchr (net.name, ':');
		*p = 0;

		for (i = 0; ethernet[i].name != NULL; ++i)
		{
			if (!strcmp (net.name, ethernet[i].name))
			{
				p = strchr (buf, ':') + 1;

				if (2 == sscanf (p, "%lu %*d %*d %*d %*d %*d %*d %*d %lu %*d %*d %*d %*d %*d %*d %*d",
							&net.rx, &net.tx))
				{
					char tmp[256];
					net.time = time (NULL);

					if (ethernet[i].onet.name != NULL)
					{
						double interval = difftime (net.time, ethernet[i].onet.time);
						double down = ((double) net.rx - (double) ethernet[i].onet.rx) / interval;
						double up = ((double) net.tx - (double) ethernet[i].onet.tx) / interval;

						snprintf (tmp, 256, "%s down: $4%.1f$0kB/s\n", net.name, down / 1024);
						print_color (global.line++, 0, tmp);
						snprintf (tmp, 256, "%s up:   $4%.1f$0kB/s\n", net.name, up / 1024);
						print_color (global.line++, 0, tmp);
					
					}
					else
					{
						snprintf (tmp, 256, "%s down: $3n/a$0\n", net.name);
						print_color (global.line++, 0, tmp);
						snprintf (tmp, 256, "%s up:   $3n/a$0\n", net.name);
						print_color (global.line++, 0, tmp);
					}

					ethernet[i].onet = net;
					break;
				}
			}
		}
	}
}

void wirelessbar (void)
{
	char buf[256];
	char *p, *name;
	int perc, i;

	rewind (global.wireless);
	while (fgets (buf, sizeof buf, global.wireless))
	{
		if (strchr (buf, ':') == NULL)
			continue;

		for (p = buf; *p == ' '; ++p);
		name = strdup (p);
		p = strchr (name, ':');
		*p = 0;

		for (i = 0; wireless[i] != NULL; ++i)
		{
			if (!strcmp (name, wireless[i]))
			{
				p = strchr (buf, ':') + 1;
				if (1 == sscanf (p, " %*s %d", &perc))
				{
					printbar (name, (double) perc, 1);
					break;
				}
			}
		}
	}
}
