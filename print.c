#include "local.h"

void print_color (int y, int x, char *string)
{
	int i;

	for (i = 0; string[i] != 0; ++i)
	{
		switch (string[i])
		{
			case '$':
				switch (string[i + 1])
				{

					case '0':
						coloroff (GREEN);
						coloroff (YELLOW);
						coloroff (RED);
						coloroff (CYAN);
						break;

					case '1': coloron (GREEN); break;
					case '2': coloron (YELLOW); break;
					case '3': coloron (RED); break;
					case '4': coloron (CYAN); break;
					default: mvprintw (y, x++, "%c", string[i]); break;
				}
				++i;
				break;

			default:
				mvprintw (y, x++, "%c", string[i]);
				break;
		}
	}
}
