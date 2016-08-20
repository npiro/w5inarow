#include <windows.h>
#include <stdlib.h>

#include "gettime.h"

float GetTime(void)
{
	return((float)GetTickCount()/1000.0);
}
