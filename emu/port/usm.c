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

static int 
usmconnect(Conv *c)
{
	key_t shmkey;
	long shmid;
	int offset;
	struct chan_pipe *chan;
	int flags = 0660;

	if(c->state == Announcing) {
		shmkey = atoi(c->laddr);
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
		perror("usmconnect: shmget: ");		
		error("Could not connect or create shared memory segment");
		print("Could not connect or create shared memory segment");
		return -1;
	}
	c->raw = (void *) shmat(shmid, 0, 0);	
	if(c->raw < 0) {
		perror("usmconnect: shmat: ");
		error("Could not attach to shared memory segment");
		print("Could not attach to shared memory segment");
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

struct Shmops usmop = {
	usmconnect,
	usmconnect,
	shmlisten,
	shmread,
	shmwrite,
	usmclose,
	shmdebug
};

void
usmlink(void)
{
	shminit();
	shmnewproto("usm", S_USM, 8, &usmop);
}
