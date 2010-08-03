#include "dat.h"
#include "fns.h"

int _xadd(ulong *, int);

int
incref(Ref *r)
{
    return _xadd(&r->ref, 1) + 1;
}

int
decref(Ref *r)
{
    int x;

    x = _xadd(&r->ref, -1) - 1;
    if (x < 0)
        panic("decref, pc=0x%lux", getcallerpc(&r));

    return x;
}
