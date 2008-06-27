/*
  * Shared memory interface based on devip
  * 
  */


#include	"dat.h"
#include	"fns.h"
#include	"error.h"
#include	"ip.h"
#include   "shm.h"

enum
{
	Qtopdir		= 1,	/* top level directory */
	Qtopbase,

	Qprotodir = Qtopbase,		/* directory for a protocol */
	Qprotobase,
	Qclone=	Qprotobase,
	Qstats,

	Qconvdir,		/* directory for a conversation */
	Qconvbase,
	Qctl=	Qconvbase,
	Qdata,
	Qlisten,
	Qlocal,
	Qremote,
	Qstatus,
	Qdebug,

	Logtype=	5,
	Masktype=	(1<<Logtype)-1,
	Logconv=	12,
	Maskconv=	(1<<Logconv)-1,
	Shiftconv=	Logtype,
	Logproto=	8,
	Maskproto=	(1<<Logproto)-1,
	Shiftproto=	Logtype + Logconv,

	Statelen = 256,

	Nfs=	1,

	Maxproto	= 4,
	MAXCONV		= 4096,
};

#define TYPE(x) 	( ((ulong)(x).path) & Masktype )
#define CONV(x) 	( (((ulong)(x).path) >> Shiftconv) & Maskconv )
#define PROTO(x) 	( (((ulong)(x).path) >> Shiftproto) & Maskproto )
#define QID(p, c, y) 	( ((p)<<(Shiftproto)) | ((c)<<Shiftconv) | (y) )

/*
 * one per IP protocol stack
 */
struct Fs
{
	RWlock	l;
	int	dev;

	int	np;
	Proto*	p[Maxproto+1];	/* list of supported protocols */
	Proto*	t2p[256];	/* vector of all protocols */
};

static 	int	ipfs_init = 0;
static	Fs	*ipfs[Nfs];	/* attached fs's */
static	char	network[] = "network";
static	char* ipstates[] = {
	"Closed",	/* Idle */
	"Announcing",
	"Announced",
	"Connecting",
	"Established",	/* Connected */
	"Closed",	/* Hungup */
};

static	Conv*	protoclone(Proto*, char*, int);
static	Conv*	newconv(Proto*, Conv **);
static	void	setladdr(Conv*);

static int
ip3gen(Chan *c, int i, Dir *dp)
{
	Qid q;
	Conv *cv;
	char *p;

	cv = ipfs[c->dev]->p[PROTO(c->qid)]->conv[CONV(c->qid)];
	if(cv->owner == nil)
		kstrdup(&cv->owner, eve);
	mkqid(&q, QID(PROTO(c->qid), CONV(c->qid), i), 0, QTFILE);

	switch(i) {
	default:
		return -1;
	case Qctl:
		devdir(c, q, "ctl", 0, cv->owner, cv->perm, dp);
		return 1;
	case Qdata:
		devdir(c, q, "data", 0, cv->owner, cv->perm, dp);
		return 1;
	case Qlisten:
		devdir(c, q, "listen", 0, cv->owner, cv->perm, dp);
		return 1;
	case Qlocal:
		p = "local";
		break;
	case Qremote:
		p = "remote";
		break;
	case Qstatus:
		p = "status";
		break;
	case Qdebug:
		p = "debug";
		break;
	}
	devdir(c, q, p, 0, cv->owner, 0444, dp);
	return 1;
}

static int
ip2gen(Chan *c, int i, Dir *dp)
{
	Qid q;

	switch(i) {
	case Qclone:
		mkqid(&q, QID(PROTO(c->qid), 0, Qclone), 0, QTFILE);
		devdir(c, q, "clone", 0, network, 0666, dp);
		return 1;
	case Qstats:
		mkqid(&q, QID(PROTO(c->qid), 0, Qstats), 0, QTFILE);
		devdir(c, q, "stats", 0, network, 0444, dp);
		return 1;
	}	
	return -1;
}

static int
ip1gen(Chan *c, int i, Dir *dp)
{
	Qid q;
	char *p;
	int prot;
	int len = 0;
	Fs *f;
	extern ulong	kerndate;

	f = ipfs[c->dev];

	prot = 0664;
	mkqid(&q, QID(0, 0, i), 0, QTFILE);
	switch(i) {
	default:
		return -1;
	}
	devdir(c, q, p, len, network, prot, dp);

	return 1;
}

static int
ipgen(Chan *c, char *name, Dirtab *tab, int x, int s, Dir *dp)
{
	Qid q;
	Conv *cv;
	Fs *f;

	USED(name);
	USED(tab);
	USED(x);
	f = ipfs[c->dev];

	switch(TYPE(c->qid)) {
	case Qtopdir:
		if(s == DEVDOTDOT){
			mkqid(&q, QID(0, 0, Qtopdir), 0, QTDIR);
			sprint(up->genbuf, "#X%lud", c->dev);
			devdir(c, q, up->genbuf, 0, network, 0555, dp);
			return 1;
		}
		if(s < f->np) {
			mkqid(&q, QID(s, 0, Qprotodir), 0, QTDIR);
			devdir(c, q, f->p[s]->name, 0, network, 0555, dp);
			return 1;
		}
		s -= f->np;
		return ip1gen(c, s+Qtopbase, dp);
	case Qprotodir:
		if(s == DEVDOTDOT){
			mkqid(&q, QID(0, 0, Qtopdir), 0, QTDIR);
			sprint(up->genbuf, "#X%lud", c->dev);
			devdir(c, q, up->genbuf, 0, network, 0555, dp);
			return 1;
		}
		if(s < f->p[PROTO(c->qid)]->ac) {
			cv = f->p[PROTO(c->qid)]->conv[s];
			sprint(up->genbuf, "%d", s);
			mkqid(&q, QID(PROTO(c->qid), s, Qconvdir), 0, QTDIR);
			devdir(c, q, up->genbuf, 0, cv->owner, 0555, dp);
			return 1;
		}
		s -= f->p[PROTO(c->qid)]->ac;
		return ip2gen(c, s+Qprotobase, dp);
	case Qclone:
	case Qstats:
		return ip2gen(c, TYPE(c->qid), dp);
	case Qconvdir:
		if(s == DEVDOTDOT){
			s = PROTO(c->qid);
			mkqid(&q, QID(s, 0, Qprotodir), 0, QTDIR);
			devdir(c, q, f->p[s]->name, 0, network, 0555, dp);
			return 1;
		}
		return ip3gen(c, s+Qconvbase, dp);
	case Qctl:
	case Qdata:
	case Qlisten:
	case Qlocal:
	case Qremote:
	case Qstatus:
	case Qdebug:
		return ip3gen(c, TYPE(c->qid), dp);
	}
	return -1;
}

void
devshmnewproto(char *name, int type, int maxconv, struct Shmops *op)
{
	Proto *p;

	p = smalloc(sizeof(*p));
	p->name = name;
	p->stype = type;
	p->ipproto = type+1;	/* temporary */
	p->nc = maxconv;
	p->op.connect = op->connect;
	p->op.announce = op->announce;
	p->op.read = op->read;
	p->op.write = op->write;
	p->op.close = op->close;
	p->op.debug = op->debug;
	p->op.listen = op->listen;

	if(Shmproto(ipfs[0], p))
		panic("can't devshmnewproto %s", name);
}

void
devshminit(void)
{
	if(!ipfs_init) {
		ipfs_init++;
		ipfs[0] = malloc(sizeof(Fs));
		if(ipfs[0] == nil)
			panic("no memory for IP stack");
	}
}

Chan *
devshmattach(char *spec)
{
	Chan *c;

	if(atoi(spec) != 0)
		error("bad specification");

	c = devattach('X', spec);
	mkqid(&c->qid, QID(0, 0, Qtopdir), 0, QTDIR);
	c->dev = 0;

	return c;
}

static Walkqid*
devshmwalk(Chan* c, Chan *nc, char **name, int nname)
{
	return devwalk(c, nc, name, nname, nil, 0, ipgen);
}

static int
devshmstat(Chan *c, uchar *db, int n)
{
	return devstat(c, db, n, 0, 0, ipgen);
}

static int m2p[] = {
	4,
	2,
	6,
};

static Chan *
devshmopen(Chan *c, int omode)
{
	Conv *cv, *nc;
	Proto *p;
	ulong raddr;
	ushort rport;
	int perm, sfd;
	Fs *f;

	perm = m2p[omode&3];

	f = ipfs[c->dev];

	switch(TYPE(c->qid)) {
	default:
		break;
	case Qtopdir:
	case Qprotodir:
	case Qconvdir:
	case Qstatus:
	case Qremote:
	case Qlocal:
	case Qstats:
	case Qdebug:
		if(omode != OREAD)
			error(Eperm);
		break;
	case Qclone:
		p = f->p[PROTO(c->qid)];
		cv = protoclone(p, up->env->user, -1);
		if(cv == 0)
			error(Enodev);
		mkqid(&c->qid, QID(p->x, cv->x, Qctl), 0, QTFILE);
		break;
	case Qdata:
	case Qctl:
		p = f->p[PROTO(c->qid)];
		qlock(&p->l);
		cv = p->conv[CONV(c->qid)];
		qlock(&cv->l);
		if(waserror()){
			qunlock(&cv->l);
			qunlock(&p->l);
			nexterror();
		}
		if((perm & (cv->perm>>6)) != perm) {
			if(strcmp(up->env->user, cv->owner) != 0)
				error(Eperm);
			if((perm & cv->perm) != perm)
				error(Eperm);
		}
		cv->inuse++;
		if(cv->inuse == 1) {
			kstrdup(&cv->owner, up->env->user);
			cv->perm = 0660;
		}
		poperror();
		qunlock(&cv->l);
		qunlock(&p->l);
		break;
	case Qlisten:
		p = f->p[PROTO(c->qid)];
		cv = p->conv[CONV(c->qid)];
		if((perm & (cv->perm>>6)) != perm){
			if(strcmp(up->env->user, cv->owner) != 0)
				error(Eperm);
			if((perm & cv->perm) != perm)
				error(Eperm);
		}

		if(cv->state != Announced)
			error("not announced");

		qlock(&cv->listenq);
		if(waserror()){
			qunlock(&cv->listenq);
			nexterror();
		}

		if(p->op.listen(cv) < 0) {
			p->op.close(cv);
			error(Enodev);
		}

		nc = protoclone(p, up->env->user, CONV(c->qid));
		if(nc == 0) {
			p->op.close(cv);
			error(Enodev);
		}

		setladdr(nc);
		nc->state = Connected;
		mkqid(&c->qid, QID(PROTO(c->qid), nc->x, Qctl), 0, QTFILE);

		poperror();
		qunlock(&cv->listenq);		
		break;
	}
	c->mode = openmode(omode);
	c->flag |= COPEN;
	c->offset = 0;
	return c;
}

static void
closeconv(Conv *cv)
{
	int fd;

	qlock(&cv->l);

	if(--cv->inuse > 0) {
		qunlock(&cv->l);
		return;
	}

	if(waserror()){
		qunlock(&cv->l);
		return;
	}
	kstrdup(&cv->owner, network);
	cv->perm = 0660;

	cv->p->op.close(cv); 

	cv->state = Idle;

	poperror();
	qunlock(&cv->l);
}

static void
devshmclose(Chan *c)
{
	Fs *f;

	f = ipfs[c->dev];
	switch(TYPE(c->qid)) {
	case Qdata:
	case Qctl:
		if(c->flag & COPEN)
			closeconv(f->p[PROTO(c->qid)]->conv[CONV(c->qid)]);
		break;
	}
}

static long
devshmread(Chan *ch, void *a, long n, vlong off)
{
	int r;
	Conv *c;
	Proto *x;
	char *p, *s;
	Fs *f;
	ulong offset = off;

	f = ipfs[ch->dev];

	p = a;
	switch(TYPE(ch->qid)) {
	default:
		error(Eperm);
	case Qprotodir:
	case Qtopdir:
	case Qconvdir:
		return devdirread(ch, a, n, 0, 0, ipgen);
	case Qctl:
		sprint(up->genbuf, "%lud", CONV(ch->qid));
		return readstr(offset, p, n, up->genbuf);
	case Qremote:
		x = f->p[PROTO(ch->qid)];
		c = x->conv[CONV(ch->qid)];
		sprint(up->genbuf, "%s!%d\n", c->raddr, c->rport);
		return readstr(offset, p, n, up->genbuf);
	case Qlocal:
		x = f->p[PROTO(ch->qid)];
		c = x->conv[CONV(ch->qid)];
		sprint(up->genbuf, "%s!%d\n", c->laddr, c->lport);
		return readstr(offset, p, n, up->genbuf);
	case Qstatus:
		x = f->p[PROTO(ch->qid)];
		c = x->conv[CONV(ch->qid)];
		s = smalloc(Statelen);
		if(waserror()){
			free(s);
			nexterror();
		}
		snprint(s, Statelen, "%s\n", ipstates[c->state]);
		n = readstr(offset, p, n, s);
		poperror();
		free(s);
		return n;
	case Qdata:
		x = f->p[PROTO(ch->qid)];
		c = x->conv[CONV(ch->qid)];
		r = x->op.read(c, a, n);
		if(r < 0)
			oserror();
		return r;
	case Qdebug:
		if(off > 0)
			return 0;
		x = f->p[PROTO(ch->qid)];
		c = x->conv[CONV(ch->qid)];

		r = x->op.debug(c, a, n);
		if(r < 0)
			oserror();
		return r;
	case Qstats:
		error("stats not implemented");
		return n;
	}
}

static void
setladdr(Conv *c)
{
	ulong laddr;
	
	sprint(c->laddr, "local");
}

static unsigned long
portno(char *p)
{
	long n;
	char *e;

	n = strtoul(p, &e, 0);
	if(p == e)
		error("non-numeric port number");
	return n;
}

/*
 *  set a local address and port from a string of the form
 *	[address!]port[!r]
 */
static void
setladdrport(Conv *c, char *str, int announcing)
{
	char *p;
	unsigned long lport;

	/*
	 *  ignore restricted part if it exists.  it's
	 *  meaningless on local ports.
	 */
	p = strchr(str, '!');
	if(p != nil){
		*p++ = 0;
		if(strcmp(p, "r") == 0)
			p = nil;
	}

	c->lport = 0;
	if(p == nil){
		if(announcing)
			sprint(c->laddr, "local");	
		p = str;
	} else {
		if(strcmp(str, "*") == 0)
			sprint(c->laddr, "noaddr");
		else
			strncpy(c->laddr, str, Shmaddrlen);
	}

	if(announcing && strcmp(p, "*") == 0){
		c->lport = 0;
	
		return;
	}

	lport = portno(p);
	if(lport <= 0)
		c->lport = 0;
	else
		c->lport = lport;


}

static char*
setraddrport(Conv *c, char *str)
{
	char *p;

	p = strchr(str, '!');
	if(p == nil)
		return "malformed address";
	*p++ = 0;
	strncpy(c->raddr, str, Shmaddrlen);
	c->rport = portno(p);
	p = strchr(p, '!');
	if(p){
		if(strstr(p, "!r") != nil)
			return "no restricted shm ports";
	}
	return nil;
}

static void
connectctlmsg(Proto *x, Conv *c, Cmdbuf *cb)
{
	char *p;

	if(c->state != Idle)
		error(Econinuse);
	c->state = Connecting;
	c->cerr[0] = '\0';
	switch(cb->nf) {
	default:
		error("bad args to connect");
	case 2:
		p = setraddrport(c, cb->f[1]);
		if(p != nil)
			error(p);
		break;
	case 3:
		p = setraddrport(c, cb->f[1]);
		if(p != nil)
			error(p);
		c->lport = portno(cb->f[2]);
		
		break;
	}
	qunlock(&c->l);
	if(waserror()){
		qlock(&c->l);
		c->state = Connected;	/* sic */
		nexterror();
	}
	if( x->op.connect(c) < 0) {
		error("Could not connect");
	} else {

		qlock(&c->l);

		poperror();
		setladdr(c);

		c->state = Connected;
	}
}

static void
announcectlmsg(Proto *x, Conv *c, Cmdbuf *cb)
{
	if(c->state != Idle)
		error(Econinuse);
	c->state = Announcing;
	c->cerr[0] = '\0';
	ipmove(c->raddr, IPnoaddr);
	c->rport = 0;
	switch(cb->nf){
	default:
		error("bad args to announce");
	case 2:
		setladdrport(c, cb->f[1], 1);
		break;
	}
	
	if (x->op.announce(c) < 0) {
		error("Could not announce");
	} else {
		c->state = Announced;
	}
}

static void
bindctlmsg(Proto *x, Conv *c, Cmdbuf *cb)
{
	USED(x);
	switch(cb->nf){
	default:
		error("bad args to bind");
	case 2:
		setladdrport(c, cb->f[1], 0);
		break;
	}
}

static long
devshmwrite(Chan *ch, void *a, long n, vlong off)
{
	Conv *c;
	Proto *x;
	char *p;
	Cmdbuf *cb;
	Fs *f;

	f = ipfs[ch->dev];

	switch(TYPE(ch->qid)) {
	default:
		error(Eperm);
	case Qdata:
		x = f->p[PROTO(ch->qid)];
		c = x->conv[CONV(ch->qid)];

		qlock(&c->wlock);
		if(waserror()){
			qunlock(&c->wlock);
			nexterror();
		}
		n = x->op.write(c, a, n);
		poperror();
		qunlock(&c->wlock);
		if(n < 0)
			oserror();
		break;
	case Qctl:
		x = f->p[PROTO(ch->qid)];
		c = x->conv[CONV(ch->qid)];
		cb = parsecmd(a, n);
		qlock(&c->l);
		if(waserror()){
			qunlock(&c->l);
			free(cb);
			nexterror();
		}
		if(cb->nf < 1)
			error("short control request");
		if(strcmp(cb->f[0], "connect") == 0)
			connectctlmsg(x, c, cb);
		else if(strcmp(cb->f[0], "announce") == 0)
			announcectlmsg(x, c, cb);
		else if(strcmp(cb->f[0], "bind") == 0)
			bindctlmsg(x, c, cb);
		else if(strcmp(cb->f[0], "ttl") == 0){
			/* ignored */
		} else if(strcmp(cb->f[0], "tos") == 0){
			/* ignored */
		} else if(strcmp(cb->f[0], "ignoreadvice") == 0){
			/* ignored */
		} else if(strcmp(cb->f[0], "datapoll") == 0) {
			qunlock(&c->l);
			if(waserror()){
				qlock(&c->l);
				nexterror();
			}
			qlock(&c->l);
			poperror();
			c->datapoll = cb->nf > 1? atoi(cb->f[1]): 1;
		} else if(strcmp(cb->f[0], "bufsize") == 0) {
			qunlock(&c->l);
			if(waserror()){
				qlock(&c->l);
				nexterror();
			}
			qlock(&c->l);
			poperror();
			if(c->state != Idle) {
				error(Eperm);
			} else {
				c->bufsize = cb->nf > 1? atoi(cb->f[1]): 1;
			}	
		} else if(strcmp(cb->f[0], "hangup") == 0){
			qunlock(&c->l);
			if(waserror()){
				qlock(&c->l);
				nexterror();
			}
			/* TO DO: check fd status if socket close/hangup interrupted */
			qlock(&c->l);
			poperror();
			x->op.close(c);
			c->state = Hungup;
		} else
			error(Enoctl);
		poperror();
		qunlock(&c->l);
		free(cb);
		break;
	}
	return n;
}

static int
devshmwstat(Chan *c, uchar *dp, int n)
{
	Dir *d;
	Conv *cv;
	Proto *p;
	Fs *f;

	f = ipfs[c->dev];
	switch(TYPE(c->qid)) {
	default:
		error(Eperm);
		break;
	case Qctl:
	case Qdata:
		break;
	}

	d = smalloc(sizeof(*d)+n);
	if(waserror()){
		free(d);
		nexterror();
	}
	n = convM2D(dp, n, d, (char*)&d[1]);
	if(n == 0)
		error(Eshortstat);
	p = f->p[PROTO(c->qid)];
	cv = p->conv[CONV(c->qid)];
	if(!iseve() && strcmp(up->env->user, cv->owner) != 0)
		error(Eperm);
	if(!emptystr(d->uid))
		kstrdup(&cv->owner, d->uid);
	if(d->mode != ~0UL)
		cv->perm = d->mode & 0777;
	poperror();
	free(d);
	return n;
}

static Conv*
protoclone(Proto *p, char *user, int cv)
{
	Conv *c, **pp, **ep, **np;
	int maxconv;

	c = 0;
	qlock(&p->l);
	if(waserror()) {
		qunlock(&p->l);
		nexterror();
	}
	ep = &p->conv[p->nc];
	for(pp = p->conv; pp < ep; pp++) {
		c = *pp;
		if(c == 0) {
			c = newconv(p, pp);
			break;
		}
		if(canqlock(&c->l)){
			if(c->inuse == 0)
				break;
			qunlock(&c->l);
		}
	}
	if(pp >= ep) {
		if(p->nc >= MAXCONV) {
			qunlock(&p->l);
			poperror();
			return 0;
		}
		maxconv = 2 * p->nc;
		if(maxconv > MAXCONV)
			maxconv = MAXCONV;
		np = realloc(p->conv, sizeof(Conv*) * maxconv);
		if(np == nil)
			error(Enomem);
		p->conv = np;
		pp = &p->conv[p->nc];
		memset(pp, 0, sizeof(Conv*)*(maxconv - p->nc));
		p->nc = maxconv;
		c = newconv(p, pp);
	}

	c->inuse = 1;
	kstrdup(&c->owner, user);
	c->perm = 0660;
	c->state = Idle;
	strcpy(c->laddr, "noaddr");
	strcpy(c->raddr, "noaddr");
	c->lport = 0;
	c->rport = 0;

	if(cv >= 0) {
		Conv *old_conv = p->conv[cv];
		c->bufsize = old_conv->bufsize;
		c->mode	= old_conv->mode;
		c->raw = old_conv->raw;
		c->chan = old_conv->chan;
		c->parent = cv;
		c->datapoll = DATA_POLL;
	} else {
		c->parent = -1;
	}

	qunlock(&c->l);
	qunlock(&p->l);
	poperror();
	return c;
}

static Conv*
newconv(Proto *p, Conv **pp)
{
	Conv *c;

	*pp = c = malloc(sizeof(Conv));
	if(c == 0)
		error(Enomem);
	qlock(&c->l);
	c->inuse = 1;
	c->p = p;
	c->x = pp - p->conv;
	p->ac++;
	return c;
}

Dev shmdevtab = {
	'X',
	"shm",

	devshminit,
	devshmattach,
	devshmwalk,
	devshmstat,
	devshmopen,
	devcreate,
	devshmclose,
	devshmread,
	devbread,
	devshmwrite,
	devbwrite,
	devremove,
	devshmwstat
};

int
Shmproto(Fs *f, Proto *p)
{
	if(f->np >= Maxproto)
		return -1;

	p->f = f;

	if(p->ipproto > 0){
		if(f->t2p[p->ipproto] != nil)
			return -1;
		f->t2p[p->ipproto] = p;
	}

	p->qid.type = QTDIR;
	p->qid.path = QID(f->np, 0, Qprotodir);
	p->conv = malloc(sizeof(Conv*)*(p->nc+1));
	if(p->conv == nil)
		panic("Shmproto");

	p->x = f->np;
	f->p[f->np++] = p;

	return 0;
}

