enum
{
	Idle=		0,
	Announcing=	1,
	Announced=	2,
	Connecting=	3,
	Connected=	4,
	Hungup=	5,

	Shmaddrlen = 255,
};

struct Shmops 
{
	int (*connect)(Conv*);
	int (*announce)(Conv*);
	int (*listen)(Conv*);
	int (*read)(Conv*, void*, unsigned long);
	int (*write)(Conv*, void*, unsigned long);
	int (*close)(Conv*);
	int (*debug)(Conv*, void *,unsigned long);
};

struct Proto
{
	QLock	l;
	int	x;
	int	ipproto;
	int	stype;
	char*	name;
	int	maxconv;
	Fs*	f;	/* file system this proto is part of */
	Conv**	conv;	/* array of conversations */
	int	pctlsize;	/* size of per protocol ctl block */
	int	nc;	/* number of conversations */
	int	ac;
	Qid	qid;	/* qid for protocol directory */
	/* port allocation isn't done here when hosted */
	
	struct Shmops op;

	void*	priv;
};

struct Conv
{
	QLock	l;

	int		x;	/* conversation index */
	struct Proto*	p;

	char	laddr[Shmaddrlen];	/* local IP address */
	char	raddr[Shmaddrlen];	/* remote IP address */
	
	ulong	lport;	/* local port number */
	ulong	rport;	/* remote port number */

	char*	owner;	/* protections */
	int		perm;
	int		inuse;	/* opens of listen/data/ctl */
	int		state;

	char	cerr[ERRMAX];

	QLock	listenq;

	void*	ptcl;	/* protocol specific stuff */

	QLock	wlock;	/* prevent data from being split by concurrent writes */

	int bufsize;
	int	mode;		/* 0 for server, 1 for client */
	void *raw;			/* shared memory handle */
	void *chan;		/* pointer to channel structure in raw */
	void *priv;			/* private data */
	int	parent;		/* pointer to parent conv */
	int datapoll;		/* poll interval for data */
};


enum 
{
	S_USM=	1,		/* Sys V shared memory */
	S_MSM=	2,		/* mmap */
	S_XEN=	3,		/* xen shared memory */
	S_PAPR=4,		/* FUTURE: power virtualization */
	S_KVM=	5,		/* FUTURE: KVM shared memory */
	S_PPE=	6,		/* FUTURE: Cell */
	S_PCI=	7,		/* FUTURE: PCI bus */

	SM_SERVER=	0,
	SM_CLIENT=	1,

	DATA_POLL=		100,
	HANDSHAKE_POLL=	100000000
};

void shmnewproto(char *, int, int, struct Shmops *);

/* generic shared memory bits */

typedef unsigned int __u32;
typedef unsigned char __u8;

typedef struct chan Channel;
struct chan
{
	__u32 magic;
	__u32 write; 
	__u32 read; 
	__u32 overflow;
	__u32 buflen;
};

enum {
	Chan_listen,
	Chan_connected,
	Chan_hungup
};

/* Two circular buffers: small one for input, large one for output. */
struct chan_pipe
{
	__u32 magic;
	int state;
	Channel out;
	Channel in;
	char buffers[0];
};

#define CHUNK_SIZE	(64<<20)
#define CHAN_MAGIC		0xB0BABEEF
#define CHAN_BUF_MAGIC	0xCAFEBABE

int  shmwrite(struct Conv *conv, void *src, unsigned long len);
int  shmread(struct Conv *conv, void *dst,  unsigned long len);
int shmdebug(struct Conv *c, void *buf, unsigned long len);
int shmlisten(struct Conv *c);
