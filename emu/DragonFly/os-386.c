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

// osblock and osready
//
// osblock blocks an Inferno kproc until it is woken by a corresponding
// osready. We use the per-process os field (p->os) as a three-state counter
// to avoid lost osready() calls; block decrements this counter, while
// ready increments it. On the 0 -> -1 transition in block, we use
// DragonFly's umtx_sleep syscall to wait for the counter to be 0 again.
// On a 1 -> 0 transition in block, we 'consume' a ready event and return.
// In ready, on the 0 -> 1 transition, we return, while we use umtx_wakeup
// on a -1 -> 0 transition.
//
void
osblock(void)
{
        int val;

        val = _xadd(&up->os, -1);
        if (val == 1)
                return;

        while((int) up->os == -1)
                umtx_sleep(&up->os, -1, 0);
}

void
osready(Proc *p)
{
        int val;

        val = _xadd(&p->os, 1);
        if (val == -1)
                umtx_wakeup(&p->os, 1);
}

