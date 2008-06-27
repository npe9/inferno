/*
  * mmap shared memory plug-in for devshm
  */

#include	"dat.h"
#include	"fns.h"
#include	"error.h"
#include 	"ip.h"
#include   	"shm.h"

#include	<sys/types.h>
#include	<sys/mman.h>

#define mb() __asm__ __volatile__ ("": : :"memory")

static int 
msmconnect(Conv *c)
{
	char *path;
	int offset, mfd, count;
	struct chan_pipe *chan;
	int flags = O_RDWR;
	int mode = 0660;
	if(c->bufsize == 0)
			c->bufsize = (8 * 1024);
	if(c->state == Announcing) {
		path = c->laddr;
		offset = c->lport;
		flags |= O_CREAT;
	} else {
		path = c->raddr;
		offset = c->rport;
	}

	mfd = open(path, flags, mode);
	if(mfd < 0) {
		perror("msmconnect: open: ");		
		error("Could not open or create shared memory segment");
		print("Could not open or create shared memory segment");
		return -1;
	}
	
	if(c->state == Announcing) {
		for(count = 0; count < (sizeof(struct chan_pipe)+(c->bufsize*2)); count++)
			write(mfd, "\0", 1);
	}

	c->raw = (void *) mmap(0, sizeof(struct chan_pipe)+(c->bufsize*2), 
						PROT_READ|PROT_WRITE, MAP_SHARED, mfd, offset);
	if(c->raw < 0) {
		perror("msmconnect: mmap: ");		
		error("Could not open or create shared memory segment");
		print("Could not open or create shared memory segment");
		return -1;
	}
	c->priv = (void *) mfd;
	c->chan = (void *) (c->raw);
	chan = (struct chan_pipe *) c->chan;
	if(c->state == Announcing) {	
		chan->magic = CHAN_MAGIC;
		chan->out.magic = CHAN_BUF_MAGIC;
		chan->out.buflen = c->bufsize;
		chan->out.write = 0;
		chan->out.read = 0;
		chan->out.overflow = 0;
		chan->in.magic = CHAN_BUF_MAGIC;
		chan->in.buflen = c->bufsize;
		chan->in.write = 0;
		chan->in.read = 0;
		chan->in.overflow = 0;
		chan->state = Announced;
		c->mode = SM_SERVER;
	} else {
		c->bufsize = chan->out.buflen;
		c->mode = SM_CLIENT;	
		if(chan->magic != CHAN_MAGIC) {
			error("you suck\n");
			return -1;
		}
		while(chan->state != Announced) 
			sleep(1);

		chan->state = Connecting;
		while(chan->state != Connected) 
			nanosleep(HANDSHAKE_POLL);
	}	
	return 0;
}

static int
msmclose(struct Conv *c)
{
	if(c->state == Connected) {
		((struct chan_pipe *)c->chan)->state = Hungup;
		munmap(c->raw, sizeof(struct chan_pipe)+(c->bufsize*2));
		close((int) c->priv); 
	}
	c->state = Hungup;

	return 0;
}

struct Shmops msmop = {
	msmconnect,
	msmconnect,
	shmlisten,
	shmread,
	shmwrite,
	msmclose,
	shmdebug
};

void
msmlink(void)
{
	devshminit();
	devshmnewproto("msm", S_MSM, 8, &msmop);
}
