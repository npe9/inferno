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

struct Conv
{
	QLock	l;

	int		x;	/* conversation index */
	Proto*	p;

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
	int	parent;		/* pointer to parent conv */
	int datapoll;		/* poll interval for data */
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

enum 
{
	S_USM=	1,
	S_XEN=	2,
	S_RHYPE=3,

	SM_SERVER=	0,
	SM_CLIENT=	1,

	DATA_POLL=		100,
	HANDSHAKE_POLL=	100000000
};

void shmnewproto(char *, int, int, struct Shmops *);
