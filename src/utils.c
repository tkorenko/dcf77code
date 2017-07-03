#include <time.h>

void
normalizeStructTM(struct tm * pStm)
{
	time_t t;

	pStm->tm_isdst = -1;
	pStm->tm_zone = NULL;

	t = mktime(pStm);
	(void)localtime_r(&t, pStm);
}
