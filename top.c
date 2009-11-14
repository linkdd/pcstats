#include "local.h"

#define __TOP__
#include "config.h"

struct process *proc1;
time_t cur_time = 0;

struct process *find_proc (pid_t pid)
{
	struct process *p = proc1;

	while (p)
	{
		if (p->pid == pid)
			return p;
		p = p->next;
	}
	return NULL;
}

struct process *add_proc (pid_t pid)
{
	struct process *p = malloc (sizeof (struct process));
	memset (p, 0, sizeof (struct process));

	p->name = NULL;
	p->prev = NULL;
	p->next = proc1;

	if (p->next)
		p->next->prev = p;
	proc1 = p;

	p->pid = pid;
	p->timestamp = 0;
	
	p->o_utime = ULONG_MAX;
	p->o_ktime = ULONG_MAX;

	return p;
}

void del_proc (struct process *p)
{
	if (p->next)
		p->next->prev = p->prev;
	if (p->prev)
		p->prev->next = p->next;
	else
		proc1 = p->next;
	
	if (p->name)
		free (p->name);
	free (p);
}

void free_all_proc (void)
{
	struct process *tmp = NULL, *p = proc1;

	while (p)
	{
		tmp = p->next;
		if (p->name)
			free (p->name);
		free (p);
		p = tmp;
	}
	proc1 = NULL;
}

void cleanup_proc (void)
{
	struct process *p = proc1;

	while (p)
	{
		struct process *cur = p;
		p = p->next;

		if (cur->timestamp != cur_time)
			del_proc (cur);
	}
}

int proc_stat (struct process *p)
{
	char line[256], filename[256];
	unsigned long utime = 0;
	unsigned long ktime = 0;
	int nice, r;
	char *lp, *rp, *ret;
	FILE *fd;

	/* proc stat */
	snprintf (filename, sizeof filename, "/proc/%d/stat", p->pid);
	fd = fopen (filename, "r");
	if (fd == NULL) return 1;
	
	p->timestamp = cur_time;

	ret = fgets (line, sizeof line, fd);
	fclose (fd);
	if (ret == NULL) return 1;
	
	/* get procname */
	lp = strchr (line, '(');
	rp = strrchr (line, ')');

	if (!lp || !rp || rp < lp)
		return 1;

	r = (unsigned) (rp - lp - 1);
	if (p->name) free (p->name);
	p->name = malloc ((r + 1) * sizeof (char));
	strncpy (p->name, lp + 1, r);
	p->name[r] = 0;

	/* get stats */
	r = sscanf (rp + 1, "%*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %lu "
						"%lu %*s %*s %*s %d %*s %*s %*s %u %u",
						&p->utime, &p->ktime, &nice, &p->vsize, &p->rss);
	if (r < 5) return 1;

	/* MEM */
	p->rss *= getpagesize ();
	p->mem = (double) (((double) p->rss / global.sys.totalram) / 10.0);

	/* CPU Time */
	p->cpu_time = p->utime + p->ktime;

	if (p->o_utime == ULONG_MAX)
		p->o_utime = p->utime;
	if (p->o_ktime == ULONG_MAX)
		p->o_ktime = p->ktime;
	
	utime = p->utime - p->o_utime;
	ktime = p->ktime - p->o_ktime;

	p->o_utime = p->utime;
	p->o_ktime = p->ktime;

	p->utime = utime;
	p->ktime = ktime;

	return 0;
}

void proc_cpu (void)
{
	struct process *p = proc1;

	while (p)
	{
		p->cpu = ((double) (p->utime + p->ktime) / (double) global.totalcpu) * 100.0;
		p = p->next;
	}
}

int update_proc_table (void)
{
	DIR *dir;
	struct dirent *entry;

	dir = opendir ("/proc");
	if (dir == NULL) return 1;

	++cur_time;

	while ((entry = readdir (dir)))
	{
		pid_t pid;

		if (!entry)
		{
			closedir (dir);
			return 1;
		}

		if (1 == sscanf (entry->d_name, "%d", &pid))
		{
			struct process *p;

			p = find_proc (pid);
			if (p == NULL)
				p = add_proc (pid);
			proc_stat (p);
		}
	}

	closedir (dir);

	return 0;
}

void swap (struct process *p1, struct process *p2)
{
	p1->next = p2->next;
	p2->prev = p1->prev;
	p2->next = p1;
	p1->prev = p2;
}

void sort (int type)
{
	struct process *p = proc1;
	int sorted = 1;

	while (p)
	{
		if (p->next == NULL)
			break;

		if (type == 1) /* sort by CPU */
		{
			if (p->cpu < p->next->cpu)
			{
				swap (p, p->next);
				sorted = 0;
			}
		}
		else if (type == 2) /* sort by MEM */
		{
			if (p->mem < p->next->mem)
			{
				swap (p, p->next);
				sorted = 0;
			}
		}
		else if (type == 3) /* sort by TIME */
		{
			if (p->cpu_time < p->next->cpu_time)
			{
				swap (p, p->next);
				sorted = 0;
			}
		}

		p = p->next;
	}

	if (!sorted)
		sort (type);
}

void print_proc (void)
{
	struct process *p;
	int i;

	if (0 != update_proc_table ())
		return;
	proc_cpu ();

	sort (TYPE);

	coloron (RED);
	mvprintw (global.line++, 0, "PID\tTIME\tMEM\tCPU\tNAME\n");
	coloroff (RED);

	p = proc1;
	if (NPROCS == 0)
	{
		while (p)
		{
			mvprintw (global.line++, 0, "%d\t%lu\t%.1f\t%.1f\t%s\n",
					  p->pid, p->cpu_time, p->mem, p->cpu, p->name);
			p = p->next;
		}
	}
	else
	{
		for (i = 0; i < NPROCS; ++i)
		{
			mvprintw (global.line++, 0, "%d\t%lu\t%.1f\t%.1f\t%s\n",
					  p->pid, p->cpu_time, p->mem, p->cpu, p->name);
			p = p->next;
		}
	}

	cleanup_proc ();
}
