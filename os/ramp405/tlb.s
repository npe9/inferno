#include	"mem.h"
#define	MB	(1024*1024)

/*
 * TLB prototype entries, loaded once-for-all at startup,
 * remaining unchanged thereafter.
 * Limit the table size to ensure it fits in small TLBs.
 */
#define	TLBE(hi, lo)	WORD	$(hi);  WORD	$(lo)

TEXT	tlbtab(SB), $-4

	/* tlbhi tlblo */

	/* DRAM, 32MB */
	TLBE(KZERO|PHYSDRAM|TLB16MB|TLBVALID, PHYSDRAM|TLBZONE(0)|TLBWR|TLBEX)
	TLBE(KZERO|(PHYSDRAM+16*MB)|TLB16MB|TLBVALID, (PHYSDRAM+16*MB)|TLBZONE(0)|TLBWR|TLBEX)

	/* memory-mapped IO, 4K */
	TLBE(PHYSMMIO|TLB4K|TLBVALID, PHYSMMIO|TLBZONE(0)|TLBWR|TLBI|TLBG)

TEXT	tlbtabe(SB), $-4
	RETURN
