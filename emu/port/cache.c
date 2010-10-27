#include "dat.h"
#include "fns.h"

/*
 * 9P file cache
 */
void
cinit(void)
{
}

void
copen(Chan *c)
{
	USED(c);
}

int
cread(Chan *c, uchar *b, int n, vlong off)
{
	USED(c);
	USED(b);
	USED(n);
	USED(off);
	return 0;
}

void
cwrite(Chan *c, uchar *buf, int n, vlong off)
{
	USED(c);
	USED(buf);
	USED(n);
	USED(off);
}

void
cupdate(Chan *c, uchar *buf,  int n, vlong off)
{
	USED(c);
	USED(buf);
	USED(n);
	USED(off);
}

