// THIS IS AN INTERMEDIATE VERSION NOT YET READY FOR USE

// Object Cache
//	The object cache is a type-specific allocator; each cache only
//	allocates buffers of the same size and runs a constructor on them
//	to place them in an initial state and a destructor to ready them for
//	release.
//
//	The cache is organized into a series of magazines, arrays of pointers;
//	There are a series of 'links', which contain a loaded magazine and 
//	the previous magazine; there is a central collection of magazines,
//	called the Depot.
// 
//	The design is strongly based on the Magazine & Depot layers of 
//	Solaris's libumem.
//
// References:
//	Magazines and Vmem: Extending the Slab Allocator to Many CPUs and
//	Arbitrary Resources. Bonwick, Adams, 2001.

// TODO:
//	- Magazine size is currently fixed; we should resize mags when
//	  contention on the depot lock exceeds a threshold, as in the Solaris
//	  kernel design
//	- Depots do not currently release either empty or full magazines to
//	  the backing allocator; this needs to be fixed
//	- Currently, every kproc uses the same set of magazines, links[0]; we
//	  ought either hash on thread ID or give each kproc its own
//	  magazine_link, as in the Solaris kernel
//	- Extend AVL-based check logic and add it to the core malloc/free; also
//	  protect it with locks. Also, (in core free), if an object was allocated
//	  from the objcache layer, we'd like to know the PC of the free-r who is 
//	  not conforming to protocol
//	...
//	- Experiment with supporting general-purpose *malloc and image allocations
//	  with this allocator.
//

// $Id: objcache.c,v 1.1 2010/02/06 16:33:42 me Exp me $

#include	"dat.h"
#include	"fns.h"
#include	"interp.h"
#include	"error.h"
#include	"avl.h"
#include	<assert.h>

#define		INIT_MAGS	4
#define		INIT_MAG_SIZE	32

struct magazine {
	int rounds;			// Number of remaining buffers
	struct magazine *next;		// Linkage used on depot list only
	void **mag_round;		// Resizable array of buffers
};

struct magazine_link {
	struct magazine_link *next;
	Lock l;
	int size;
	struct magazine *loaded;
	struct magazine *prev;
};

struct objcache {
	char *name;
	ulong size;
	ulong align;
	int (*ctor)(void *, void *);
	void (*dtor)(void *, void *);
	void *priv;

	int allocs;
	int frees;
	int bytes_out;

	struct magazine_link links[INIT_MAGS];

	Lock depot_lock;
	struct magazine *depot_full;
	struct magazine *depot_empty;
	int depot_lock_contention;
};

// xxx: needs locks
static Avltree* checktree;

struct malloc_addr {
	Avl avl;
	void *addr;
	int type;
};

// malloc_addr.type
enum {
	VOID,
	OBJCACHE_ALLOCATED_ALLOC,
	OBJCACHE_ALLOCATED_FREE,
	RAW_MALLOC_ALLOC,
	RAW_MALLOC_FREE,
	UNKNOWN
};

static int checkaddr(Avl *va, Avl *vb) {
	struct malloc_addr *a = (struct malloc_addr *) va, *b = (struct malloc_addr *) vb;

	return (char*) a->addr - (char *) b->addr;
}

void objcache_init(void) {
	checktree = mkavltree(checkaddr);

}

objcache*
objcache_create(char *name, ulong size, ulong align,
		int (*ctor)(void *obj, void *priv),
		void (*dtor)(void *obj, void *priv),
		void *priv)
{
	objcache *cache = malloc(sizeof(objcache));
	int i;

	cache->name = name;
	cache->size = size;
	cache->align = align;
	cache->ctor = ctor;
	cache->dtor = dtor;
	cache->priv = priv;

	cache->allocs = cache->frees = cache->bytes_out = 0;

	// xxx: Convert to variable number of magazine links, hash kproc ID to assign to
	// magazine link

	for (i = 0; i < INIT_MAGS; i++) {
		cache->links[i].next = nil;
		cache->links[i].loaded = nil;
		cache->links[i].prev = nil;
		cache->links[i].size = INIT_MAG_SIZE;
	}

	cache->depot_full = nil;
	cache->depot_empty = nil;
	cache->depot_lock_contention = 0;

	return cache;
}

void*
objcache_alloc(objcache *cache, int flag)
{
	void *obj;
	int first_time = 1;
	struct malloc_addr *m_a_check = malloc(sizeof (struct malloc_addr));
	Avl *old;

	lock(&cache->links[0].l); do {

		// xxx: needs thought about locks; these are per-cache stats
		if (first_time == 1) {
			cache->allocs++;
			cache->bytes_out += cache->size;
			first_time = 0;
		}

		// if the loaded magazine has rounds, allocate and return
		if (cache->links[0].loaded && cache->links[0].loaded->rounds > 0) {
			obj = cache->links[0].loaded->mag_round[--cache->links[0].loaded->rounds];
			break;
		}

		// if the previous magazine is full, swap it with loaded and try again	
		if (cache->links[0].prev && cache->links[0].prev->rounds == cache->links[0].size) {
			struct magazine *tmp = cache->links[0].prev;
			cache->links[0].prev = cache->links[0].loaded;
			cache->links[0].loaded = tmp;
			continue;
		}

		// if the depot has any full magazines, return prev, mov loaded to prev, load a full mag, retry 

		int nContended = canlock(&cache->depot_lock);
		int retry_alloc = 0;
		if (nContended == 0) {
			lock (&cache->depot_lock); 
			cache->depot_lock_contention++;
		} do {
			if (cache->depot_full != nil) {
				struct magazine *tmp = cache->depot_full;
				if (tmp)	
					cache->depot_full = tmp->next;

				// return prev
				struct magazine *tmp2 = cache->links[0].prev;
				if (tmp2) {
					tmp2->next = cache->depot_empty;
					cache->depot_empty = tmp2;
				}
		
				// move loaded to prev
				cache->links[0].prev = cache->links[0].loaded;

				// load the new mag
				if (tmp)
					cache->links[0].loaded = tmp;

				// Retry the allocation, without depot lock held
				retry_alloc = 1;
			}
		} while(0); unlock(&cache->depot_lock);
		if (retry_alloc == 1)
			continue;

	} while(0); unlock(&cache->links[0].l);	

	// allocate an object from malloc, call the ctor
	// xxx: extract size and ctor under cache locks
	// xxx: objcache should check if old entry exists, should check ranges, not points
	if (obj == nil) {
		obj = malloc(cache->size);
		if (cache->ctor)
			cache->ctor(obj, cache->priv);
		m_a_check->addr = obj;
		m_a_check->type = RAW_MALLOC_ALLOC;
		insertavl(checktree, &m_a_check->avl, &old);
	} else {
		m_a_check->addr = obj;
		m_a_check->type = OBJCACHE_ALLOCATED_ALLOC;
		insertavl(checktree, &m_a_check->avl, &old);
	}

	return obj;
}

void
objcache_free(objcache *cache, void *p)
{
	int first_time = 1;
	int do_free = 0;
	struct malloc_addr a;
	a.addr = p;

	Avl *tgtx = lookupavl(checktree, &a.avl);
	struct malloc_addr *tgt = (struct malloc_addr *) tgtx;
	if (!(tgt->type == OBJCACHE_ALLOCATED_ALLOC || tgt->type == RAW_MALLOC_ALLOC))
		printf("Warning! Addr mismatch on %x caller %x\n", p, __builtin_return_address(0));
	
	lock(&cache->links[0].l); do {
		// xxx: need to think of correct locking/atomics for these.
		if (first_time == 1) {
			cache->bytes_out -= cache->size;
			cache->frees++;
			first_time = 0;
		}

		assert(cache->bytes_out >= 0);

		// if the loaded magazine isn't full insert and done
		if (cache->links[0].loaded && cache->links[0].loaded->rounds < cache->links[0].size) {
			cache->links[0].loaded->mag_round[cache->links[0].loaded->rounds++] = p;
			break;
		}

		// if the prev magazine is empty, exchange and try again
		if (cache->links[0].prev && cache->links[0].prev->rounds == 0) {
			struct magazine *tmp = cache->links[0].prev;
			cache->links[0].prev = cache->links[0].loaded;
			cache->links[0].loaded = tmp;
			continue;
		}

		// if the depot has any empty magazines, move previous to depot, 
		// move loaded to previous, load empty magazine, continue

		int nContended = canlock(&cache->depot_lock);
		int retry_free = 0;
		if (nContended == 0) {
			lock(&cache->depot_lock);
			cache->depot_lock_contention++;
		} do {
			if (cache->depot_empty != nil) {
				// move previous to depot_full list
				int was_prev = 0;
				if (cache->links[0].prev) {
					cache->links[0].prev->next = cache->depot_full;
					was_prev = 1;
				}
			// INVARIANT: if cache->links[0].prev was nil, we want to ensure that we are not
			// throwing away the entire full set of our depot.
				if (was_prev == 0) 
					assert(cache->depot_full == nil);
				cache->depot_full = cache->links[0].prev;
		
				// move loaded to previous
				cache->links[0].prev = cache->links[0].loaded;

				// load empty magazine
				cache->links[0].loaded = cache->depot_empty;
				cache->depot_empty = cache->links[0].loaded->next;
				// we would like to be sure that the magazine we just loaded was actually empty.
				assert(cache->links[0].loaded->rounds == 0);

				// Retry free
				retry_free = 1;
			}
		} while(0); unlock(&cache->depot_lock);
		if (retry_free)
			continue;

		// malloc an empty magazine, put in the depot, continue;
		// xxx: threshold for 'too many magazines'? is this a good 
		// time to release depot extras?
		struct magazine *newmag = malloc(sizeof(struct magazine));
		newmag->rounds = 0;
		newmag->mag_round = malloc(sizeof(void *) * cache->links[0].size);
		newmag->next = nil;
		cache->depot_empty = newmag;
		continue;

	} while(0); unlock(&cache->links[0].l);
		
	// return the buffer to the underlying allocator
	// xxx: want ctor extracted under cache lock
	if (do_free == 1) {
		if (cache->dtor)
			cache->dtor(p, cache->priv);
		free(p);
	}

}

void
objcache_destroy(objcache *cache)
{
	// xxx: TODO.
	// Do we *really* want this? Probably; at least checking the Mainmem
	// pool on exit is a noble goal.
}
