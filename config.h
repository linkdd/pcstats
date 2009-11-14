#ifndef __CONFIG_H
#define __CONFIG_H

#ifdef __MAIN__

enum Fields { EndOfFields, Uname, Loadavg, Date, Cpu, Ram, Swap, Disk, Wireless, Network, Battery, Process };
enum Fields show_fields[] =
{
	Uname,
	Loadavg,
	Date,
	Cpu,
	Ram,
	Swap,
	Disk,
	Network,
	EndOfFields
};

#endif /* __MAIN__ */

#ifdef __NET__

/* Add your network interfaces here,
 * like eth0, wlan0, etc...
 */
struct NetworkInterface ethernet[] =
{
	{ .name = "eth0" },
	{ .name = NULL }
};

/* Add your wireless interfaces here,
 * like wlan0, wlan1, etc...
 */
const char *wireless[] =
{
	"wlan0",
	NULL
};

/* 
 * ethernet : Measure the interface's flow
 * wireless : Measure the signal intensity
 */

#endif /* __NET__ */

#ifdef __TOP__

/* Sort process list by *** */
enum { CPU = 1, MEM, TIME };
#define TYPE	CPU

/* Number of processes printed, 0 for all */
#define NPROCS	5

#endif /* __TOP__ */

#endif /* __CONFIG_H */

