/*
 *  RAMP specific code for the ns16552 
 */
enum
{
	UartFREQ = 0, /* TODO */
};

#define uartwrreg(u,r,v)	*(uchar*)((u)->port+r) = v
#define uartrdreg(u,r)		*(uchar*)((u)->port+r)

#define outb(a, v) *(uchar*)(a) = (v)
#define inb(a) *(uchar*)(a)

extern ulong UART_BASE;

void	ns16552setup(ulong, ulong, char*);
void 	ns16552special(int port, int baud, Queue **in, Queue **out, 
			int (*putc)(Queue*, int));


static void
uartpower(int, int)
{
}

/*
 *  handle an interrupt to a single uart
 */
static void
ns16552intrx(Ureg *ur, void *arg)
{
	USED(ur);

	ns16552intr((ulong)arg);
}

/*
 *  install the uarts (called by reset)
 */
void
ns16552install(void)
{
	static int already;
	void uartclock(void);

	if(already)
		return;
	already = 1;

	/* first two ports are always there and always the normal frequency */
	ns16552setup(UART_BASE, UartFREQ, "eia0");
	ns16552special(0, 38400, &kbdq, &printq, kbdputc);
	addclock0link(uartclock, 22);
}

/*
 * If the UART's receiver can be connected to a DMA channel,
 * this function does what is necessary to create the
 * connection and returns the DMA channel number.
 * If the UART's receiver cannot be connected to a DMA channel,
 * a -1 is returned.
 */
char
ns16552dmarcv(int dev)
{
 
	USED(dev);
        return -1;
}

long
dmasetup(int,void*,long,int)
{
	return 0;
}

void
dmaend(int)
{
}

int
dmacount(int)
{
	return 0;
}
