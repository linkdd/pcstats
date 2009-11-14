#include "local.h"

void rambar (void)
{
	unsigned long usedram = global.sys.totalram - global.sys.freeram;
	double perc = (((double) usedram / (double) global.sys.totalram) * 100.0) / 4;
	printbar ("RAM", perc, 0);
}

void swapbar (void)
{
	unsigned long usedswap = global.sys.totalswap - global.sys.freeswap;
	double perc = ((double) usedswap / (double) global.sys.totalswap) * 100.0;
	printbar ("SWAP", perc, 0);
}

