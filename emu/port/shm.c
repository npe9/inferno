/*
  * UNIX shared memory plug-in for devshm
  */
#define		_XOPEN_SOURCE

#include	"dat.h"
#include	"fns.h"
#include	"error.h"
#include 	"ip.h"
#include   	"shm.h"

#include	<sys/types.h>
#include	<sys/shm.h>
#include	<sys/ipc.h>

/* 
  * Shared memory circular buffer code
  */

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
	char *buf;			/* pointer to buffer area */
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

static void
yield(int x)
{
	struct timespec yield_time;

	if(x==0) {
		osyield();
	} else {
		yield_time.tv_sec = 0;
		yield_time.tv_nsec = x; 
		nanosleep(&yield_time); 
	}
}

/* TODO: Fix - figure out what we really need to do here */
#define mb() __asm__ __volatile__ ("": : :"memory")

/*
 * (expr) may be as much as (limit) "below" zero (in an unsigned sense).
 * We add (limit) before taking the modulus so that we're not dealing with
 * "negative" numbers.
 */
#define CIRCULAR(expr, limit) (((expr) + (limit)) % (limit))


static inline int 
check_write_buffer(const Channel *h, __u32 bufsize)
{
	/* Buffer is "full" if the write index is one behind the read index. */
	return (h->write != CIRCULAR((h->read - 1), bufsize));
}

static inline int 
check_read_buffer(const Channel *h, __u32 bufsize)
{
	/* Buffer is empty if the read and write indices are the same. */
	return (h->read != h->write);
}

/* We can't fill last byte: would look like empty buffer. */
static char *
get_write_chunk(const Channel *h, char *buf, __u32 bufsize, __u32 *len)
{
        /* We can't fill last byte: would look like empty buffer. */
	__u32 write_avail = CIRCULAR(((h->read - 1) - h->write), bufsize);
	*len = ((h->write + write_avail) <= bufsize) ?
		write_avail : (bufsize - h->write);
	return buf + h->write;
}

static const char *
get_read_chunk(const Channel *h, const char *buf, __u32 bufsize, __u32 *len)
{
	__u32 read_avail = CIRCULAR((h->write - h->read), bufsize);
       	*len = ((h->read + read_avail) <= bufsize) ?
		read_avail : (bufsize - h->read);
	return buf + h->read;
}

static void 
update_write_chunk(Channel *h, __u32 bufsize, __u32 len)
{
	/* fprint(2, "> %x\n",len); DEBUG */
	h->write = CIRCULAR((h->write + len), bufsize);
	mb();
}

static void 
update_read_chunk(Channel *h, __u32 bufsize, __u32 len)
{
	/* fprint(2, "< %x\n",len); DEBUG */
	h->read = CIRCULAR((h->read + len), bufsize);
	mb();
}

#undef CIRCULAR

static int 
usmwrite(struct Conv *conv, void *src, unsigned long len)
{
	int ret = 0;
	struct chan_pipe *p = (struct chan_pipe *)conv->chan;
	Channel *c = &p->in;
	__u32 bufsize = c->buflen;

	if(conv->mode == SM_CLIENT)
		c = &p->out;

	while (!check_write_buffer(c, bufsize)) {
		yield(conv->datapoll);
	}

        while (len > 0) {
		__u32 thislen;
		char *dst = get_write_chunk(c, c->buf, bufsize, &thislen);

		if (thislen == 0) {
			yield(conv->datapoll);
			continue;
		}
		
		if (thislen > len)
			thislen = len;
		memcpy(dst, src, thislen);
		mb();
		update_write_chunk(c, bufsize, thislen);
		src += thislen;
		len -= thislen;
		ret += thislen;
	}

	/* Must have written data before updating head. */
	return ret;
}

static int 
usmread(struct Conv *conv, void *dst,  unsigned long len)
{
	int ret = 0;
	struct chan_pipe *p = (struct chan_pipe *)conv->chan;
	Channel *c = &p->out;
	__u32 bufsize = c->buflen;

	if(conv->mode == SM_CLIENT)
		c = &p->in;

	while (!check_read_buffer(c, bufsize)) {
		if(p->state == Hungup)
			return 0;
		yield(conv->datapoll);
	}

	while (len > 0) {
		__u32 thislen;
		const char *src;

		src = get_read_chunk(c, c->buf, bufsize, &thislen);

		if (thislen == 0) {
			if(p->state == Hungup)
				return 0;
			yield(conv->datapoll);
			continue;
		}
		if (thislen > len) 
			thislen = len;
		memcpy(dst, src, thislen);
		mb();
		update_read_chunk(c, bufsize, thislen);
		dst += thislen;
		len -= thislen;
		ret += thislen;
		break; /* obc */
	}

	/* Must have read data before updating head. */
	return ret;
}

static int 
usmconnect(Conv *c)
{
	key_t shmkey;
	long shmid;
	int offset;
	struct chan_pipe *chan;
	int flags = 0660;

	if(c->state == Announcing) {
		shmid = atoi(c->laddr);
		offset = c->lport;
		flags |= IPC_CREAT;
		if(c->bufsize == 0)
			c->bufsize = (8 * 1024);
	} else {
		shmkey = atoi(c->raddr);
		offset = c->rport;
	}

	shmid = shmget(shmkey, offset+sizeof(struct chan_pipe)+
				(c->bufsize*2), flags);

	if(shmid < 0) {
		error("Could not connect or create shared memory segment");
		return -1;
	}

	c->raw = (void *) shmat(shmid, 0, 0);	
	if(c->raw < 0) {
		error("Could not attach to shared memory segment");
		return -1;
	}

	c->chan = (void *) (c->raw+offset);
	chan = (struct chan_pipe *) c->chan;
	if(c->state == Announcing) {	
		chan->magic = CHAN_MAGIC;
		chan->out.magic = CHAN_BUF_MAGIC;
		chan->out.buflen = c->bufsize;
		chan->out.write = 0;
		chan->out.read = 0;
		chan->out.overflow = 0;
		chan->out.buf = chan->buffers;
		chan->in.magic = CHAN_BUF_MAGIC;
		chan->in.buflen = c->bufsize;
		chan->in.buf = chan->buffers+c->bufsize;
		chan->in.write = 0;
		chan->in.read = 0;
		chan->in.overflow = 0;
		chan->state = Announced;
		c->mode = SM_SERVER;
	} else {
		if(chan->magic != CHAN_MAGIC) {
			error("you suck\n");
			return -1;
		}
		while(chan->state != Announced)
			sleep(1);
		chan->state = Connecting;
		while(chan->state != Connected)
			nanosleep(HANDSHAKE_POLL);
		c->mode = SM_CLIENT;
	}	
	
	return 0;
}

static int
usmclose(struct Conv *c)
{
	if(c->state == Connected) {
		((struct chan_pipe *)c->chan)->state = Hungup;
		if (shmctl(atoi(c->laddr), IPC_RMID, 0) < 0) {
			error("shmctl error on cleanup\n");	
			return -1;
		}	
	}
	c->state = Hungup;

	return 0;
}

static int
usmdebug(struct Conv *c, void *buf, unsigned long len)
{
	int ret;
	struct chan_pipe *chan = (struct chan_pipe *) c->chan;
	ret = sprint(buf, "Magic: %x\nOut\n Out.Magic: %x\n Out.buflen: %x\n Out.write: %x\n Out.read: %x\n Out.over: %x\nIn\n In.Magic: %x\n In.buflen: %x\n In.write: %x\n In.read: %x\n In.over: %x\n",
		chan->magic, chan->out.magic, chan->out.buflen, chan->out.write, chan->out.read, 
		chan->out.overflow, chan->in.magic, chan->in.buflen, chan->in.write, chan->in.read,
		chan->in.overflow);
	return ret;
}

static int
usmlisten(struct Conv *c)
{
	struct chan_pipe *chan = (struct chan_pipe *)c->chan;
	while(chan->state != Connecting)
		sleep(1);

	chan->state = Connected;
	mb();
	return 0;
}

struct Shmops usmop = {
	usmconnect,
	usmconnect,
	usmlisten,
	usmread,
	usmwrite,
	usmclose,
	usmdebug
};

void
shmlink(void)
{
	shminit();
	shmnewproto("usm", S_USM, 8, &usmop);
}
