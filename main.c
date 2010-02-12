#include "local.h"

#define __MAIN__
#include "config.h"

struct SystemInfo global;

#define color(id,c)		init_pair (id, c, bg)
static int bg = 0;

void coloron (int id)
{
	if (global.colors)
		attron (COLOR_PAIR (id));
}

void coloroff (int id)
{
	if (global.colors)
		attroff (COLOR_PAIR (id));
}

void handler (int signum)
{
	extern struct cpu_t *ocpu, *ncpu;

	if (ncpu != NULL) free (ncpu);
	if (ocpu != NULL) free (ocpu);

	if (global.loadavg.file != NULL)	fclose (global.loadavg.file);
	if (global.cpu != NULL)				fclose (global.cpu);
	if (global.mtab != NULL)			fclose (global.mtab);
	if (global.net != NULL)				fclose (global.net);
	if (global.wireless != NULL)		fclose (global.wireless);

	free_all_proc ();

	endwin ();

	if (signum == SIGSEGV)
	{
		printf ("Segmentation fault.\n");
		exit (EXIT_FAILURE);
	}
	else
	{
		exit (EXIT_SUCCESS);
	}
}

void loadavg (void)
{
	rewind (global.loadavg.file);
	while (!feof (global.loadavg.file))
	{
		if (6 == fscanf (global.loadavg.file, "%lf %lf %lf %d/%d %d",
					&global.loadavg.loads[0],
					&global.loadavg.loads[1],
					&global.loadavg.loads[2],
					&global.loadavg.prun,
					&global.loadavg.ptotal,
					&global.loadavg.pid))
		{
			break;
		}
	}
}

int main (void)
{
	int i, c;

	initscr ();
	noecho ();
	curs_set (0);

	if (has_colors () == FALSE)
	{
		fprintf (stderr, "Your terminal doesn't support colors.\n");
		global.colors = 0;
	}
	else
	{
		global.colors = 1;
		start_color ();

		if (use_default_colors () == OK)
			bg = -1;

		color (GREEN, COLOR_GREEN);
		color (YELLOW, COLOR_YELLOW);
		color (RED, COLOR_RED);
		color (CYAN, COLOR_CYAN);
	}


	signal (SIGINT, &handler);
	signal (SIGTERM, &handler);
	signal (SIGSEGV, &handler);

	global.loadavg.file = fopen ("/proc/loadavg", "r");
	global.cpu = fopen ("/proc/stat", "r");
	global.mtab = fopen ("/etc/mtab", "r");
	global.net = fopen ("/proc/net/dev", "r");
	global.wireless = fopen ("/proc/net/wireless", "r");

	/* Init global.battery */
	global.battery.ok = 1;
	strncpy (global.battery.dir.location, "/proc/acpi/battery/", 128);
	global.battery.dir.dir = opendir (global.battery.dir.location);

	if (global.battery.dir.dir != NULL)
	{
		while (1)
		{
			global.battery.dir.content = readdir (global.battery.dir.dir);

			if (!strcmp (global.battery.dir.content->d_name, "."))
				break;
			else if (!strcmp (global.battery.dir.content->d_name, ".."))
				break;
		}

		closedir (global.battery.dir.dir);

		if (global.battery.dir.content != NULL)
		{
			strcat (global.battery.dir.location, global.battery.dir.content->d_name);
			snprintf (global.battery.state.location, 128, "%s/state", global.battery.dir.location);
			snprintf (global.battery.info.location, 128, "%s/info", global.battery.dir.location);

			global.battery.state.file = fopen (global.battery.state.location, "r");
			global.battery.info.file = fopen (global.battery.info.location, "r");

			if (global.battery.state.file == NULL
				|| global.battery.info.file == NULL)
			{
				fprintf (stderr, "Can't open %s/state or %s/info while %s is present, check your ACPI configuration or remove the directory %s.\n",
						global.battery.dir.location, global.battery.dir.location,
						global.battery.dir.location, global.battery.dir.location);

				global.battery.ok = 0;
			}
		}
		else
		{
			fprintf (stderr, "Nothing in /proc/acpi/battery or can't list directory while directory present.\n");
			global.battery.ok = 0;
		}
	}
	else
	{
		fprintf (stderr, "No /proc/acpi/battery directory, skipping the battery part.\n");
		global.battery.ok = 0;
	}

	/* Get static system informations */
	global.nprocs = get_nprocs ();
	uname (&global.uname);

	{ /* get processor */
		FILE *f = fopen ("/proc/cpuinfo", "r");
		char buf[256];

		strncpy (global.processor, global.uname.machine, 256);

		if (f != NULL)
		{
			while (fgets (buf, sizeof buf, f))
			{
				if (!strncmp (buf, "model name", 10))
				{
					char *p = strchr (buf, ':');
					strncpy (global.processor, &p[2], 256);
					p = strchr (global.processor, '\n'); *p = 0;
					break;
				}
			}

			fclose (f);
		}

		global.processor[255] = 0;
	}

	if (global.cpu != NULL)
	{
		extern struct cpu_t *ocpu, *ncpu;

		ocpu = malloc (global.nprocs * sizeof (struct cpu_t));
		ncpu = malloc (global.nprocs * sizeof (struct cpu_t));

		if (ocpu == NULL || ncpu == NULL)
			fclose (global.cpu), global.cpu = NULL;
	}

	halfdelay (1);
	while ((c = getch ()) != '\n')
	{
		global.line = 0;

		switch (c)
		{
			case 'r': clear (); break;
		}

		/* Check system informations */
		if (0 != sysinfo (&global.sys))
			continue;

		/* Print informations */
		for (i = 0; show_fields[i] != EndOfFields; ++i)
		{
			if (show_fields[i] == Uname)
			{
				mvprintw (global.line++, 0, "%s %s\n%s\n", global.uname.nodename, global.uname.release, global.processor);
				global.line++;
			}

			if (global.loadavg.file != NULL && show_fields[i] == Loadavg)
			{
				char tmp[256];

				loadavg ();
				snprintf (tmp, 256, "Load average: 1min: $4%.2f$0 / 5min: $4%.2f$0 / 15min: $4%.2f$0\n",
						global.loadavg.loads[0],
						global.loadavg.loads[1],
						global.loadavg.loads[2]);
				print_color (global.line++, 0, tmp);
			
				snprintf (tmp, 256, "Tasks: $4%d$0 running, $4%d$0 total, last PID used : $4%d$0\n",
						global.loadavg.prun, global.loadavg.ptotal, global.loadavg.pid);
				print_color (global.line++, 0, tmp);
			}
	
			refresh ();
	
			if (show_fields[i] == Date) /* Date */
			{
				time_t timestamp= time (NULL);
				struct tm *t = localtime (&timestamp);
	
				mvprintw (global.line, 0, "Time: %02d:%02d:%02d |",
						t->tm_hour,
						t->tm_min,
						t->tm_sec);
				refresh ();
			}
	
			if (show_fields[i] == Date) /* Uptime */
			{
				int uptime = global.sys.uptime;
				int up, day, hour, min, sec;
	
				day = uptime / 86400;
				up = uptime - (day * 86400);
				hour = up / 3600;
				up -= hour * 3600;
				min = up / 60;
				sec = up - min * 60;
	
				mvprintw (global.line++, 17, "Uptime: %d day%s, %02d:%02d:%02d\n",
						day, (day > 1 ? "s" : ""),
						hour, min, sec);
				refresh ();
			}
	
			if (global.cpu != NULL && show_fields[i] == Cpu)
			{
				int i;
				for (i = 0; i < global.nprocs; ++i)
					cpubar (i);
			}
	
			if (show_fields[i] == Ram)  rambar ();
			if (show_fields[i] == Swap) swapbar ();
	
			if (global.mtab != NULL && show_fields[i] == Disk)
				diskbar ();
	
			if (global.wireless != NULL && show_fields[i] == Wireless)
				wirelessbar ();
	
			if (global.net != NULL && show_fields[i] == Network)
				netbar ();
	
			if (global.battery.ok && show_fields[i] == Battery)
				batterybar ();
	
			if (show_fields[i] == Process) print_proc ();
		}

		sleep (1);
	}

	raise (SIGTERM);

	return 0;
}

void printbar (const char *str, double perc, int reverse)
{
	int bar = (int) (perc * 0.25);
	int i;

	if (str == NULL || perc < 0.0 || perc > 100.0)
		return;

	if (strlen (str) > 6)
	{
		mvprintw (global.line++, 0, "%s:\n", str);
		mvprintw (global.line++, 0, "\t%3d%% [", (int) perc);
	}
	else
		mvprintw (global.line++, 0, "%s:\t%3d%% [", str, (int) perc);

	if (reverse)
	{
		if (perc > 50.0)
			coloron (GREEN);
		else if (perc > 10.0 && perc <= 50.0)
			coloron (YELLOW);
		else
			coloron (RED);
	}
	else
	{
		if (perc < 50.0)
			coloron (GREEN);
		else if (perc >= 50.0 && perc < 90.0)
			coloron (YELLOW);
		else
			coloron (RED);
	}

	for (i = 0; i < bar; ++i)
		printw ("=");
	for (; i < 25; ++i)
		printw (" ");

	coloroff (GREEN);
	coloroff (YELLOW);
}
