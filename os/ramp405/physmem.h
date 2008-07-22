/*
 * Memory-mapped IO
 */

#define	PHYSPCIBRIDGE	0x80000000
#define	PHYSMMIO		    0x40000000
#define	MMIO(i)	(PHYSMMIO+(i)*0x100000)
#define	PHYSGPT	MMIO(0)
#define PHYSUART	MMIO(4)
#define PHYSUARTx(x) (PHYSUART+(x)*0x20000)
#define	PHYSUART0	PHYSUARTx(0)
#define	PHYSUART1	PHYSUARTx(1)
#define	PHYSIIC	MMIO(5)
#define	PHYSOPB	MMIO(0)
#define	PHYSGPIO	MMIO(0)
#define	PHYSEMAC0	MMIO(12)

#define	PHYSPCIIO0	0xE8000000	/* for 64M */
#define	PHYSPCIMEM	0x80000000
#define	PHYSPCIADDR	0xEEC00000	/* for 8 bytes */
#define	PHYSPCIDATA	0xEEC00004
#define	PHYSPCIACK	0xEED00000	/* interrupt acknowledge */
#define	PHYSPCIBCFG	0xEF400000	/* bridge configuration registers */
