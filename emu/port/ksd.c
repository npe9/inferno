/* kproc-specific-data
 * provides a mechanism to create keys, associated with different data in each
 * running kproc.
 */

#include "dat.h"
#include "fns.h"
#include "interp.h"
#include "error.h"

static Ref high_ksd;

int
ksd_key_create(void) 
{
	int i;
	i = incref(&high_ksd) - 1;

	if (i >= NKEYS) {
		decref(&high_ksd);
		return -1;
	}

	return i;
}

void *
ksd_getspecific(int key)
{
	if (key < high_ksd.ref && key >= 0)
		return up->ksd[key];
	return nil;
}

void *
ksd_setspecific(int key, void *val)
{
	if (!(key < high_ksd.ref && key >= 0))
		return nil;

	void *tmp = up->ksd[key];
	up->ksd[key] = val;
	return tmp;
}

