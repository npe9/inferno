#include "dat.h"
#include "fns.h"

int
incref(Ref *r)
{
    int x;

    lock(&r->lk);
    x = ++r->ref;
    unlock(&r->lk);
    return x;
}

int
decref(Ref *r)
{
    int x;

    lock(&r->lk);
    x = --r->ref;
    unlock(&r->lk);
    if(x < 0)
        panic("decref, pc=0x%lux", getcallerpc(&r));

    return x;
}

