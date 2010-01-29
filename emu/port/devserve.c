/*
 * this should be in os/port too.
 * needs a review for error handling.
 * don't use too much, it may go away.
 */

#include	"dat.h"
#include	"fns.h"
#include	"error.h"

typedef struct Serve Serve;
struct Serve
{
	int	id;
	char	*name;
	int	perm;
	Chan	*c;
};
Serve **serves;
int nserves;
int serveidgen;


Serve *
servefind(int id)
{
	int i;
	for(i = 0; i < nserves; i++)
		if(serves[i]->id == id)
			return serves[i];
	return nil;
}

int
serveunlink(int id)
{
	int i;
	Serve *ss;

	for(i = 0; i < nserves; i++)
		if(serves[i]->id == id) {
			ss = serves[i];
			serves[i] = serves[nserves-1];
			serves[nserves-1] = nil;
			nserves--;
			if(ss->c != nil)
				cclose(ss->c);
			free(ss->name);
			free(ss);
			return 1;
		}
	return 0;
}

void
servelink(Serve *ss)
{
	serves = realloc(serves, (nserves+1)*sizeof serves[0]);
	serves[nserves++] = ss;
}

Serve*
servenew(char *name, int perm)
{
	Serve *s;

	s = malloc(sizeof s[0]);
	s->id = ++serveidgen;
	s->name = strdup(name);
	s->perm = perm;
	s->c = nil;
	return s;
}

static int
servegen(Chan *c, char *name, Dirtab *tab, int ntab, int s, Dir *dp)
{
	Serve *ss;
	Qid q;

	USED(name);
	USED(tab);
	USED(ntab);

	if(s == DEVDOTDOT) {
		devdir(c, c->qid, "#с", 0, eve, 0555, dp);
		return 1;
	}
	if(s >= nserves)
		return -1;
	ss = serves[s];
	q.path = ss->id;
	q.vers = 0;
	q.type = QTFILE;
	devdir(c, q, ss->name, 0, "serve", ss->perm, dp);
	return 1;
}

static Chan*
serveattach(char *spec)
{
	return devattach(0x0441, spec); /* L'с' */
}

static Walkqid*
servewalk(Chan *c, Chan *nc, char **name, int nname)
{
	return devwalk(c, nc, name, nname, nil, 0, servegen);
}

static int
servestat(Chan *c, uchar *db, int n)
{
	return devstat(c, db, n, 0, 0, servegen);
}

static Chan*
serveopen(Chan *c, int omode)
{
	Serve *ss;
	Chan *nc;

	openmode(omode);	/* check it */
	if(c->qid.type & QTDIR){
		if(omode != OREAD)
			error(Eisdir);
		c->mode = omode;
		c->flag |= COPEN;
		c->offset = 0;
		return c;
	}

	ss = servefind(c->qid.path);
	if(ss == nil || ss->c == nil)
		error(Ebadfd);

	nc = ss->c;
	cclose(c);
	incref(&nc->r);
	return nc;
}

static void
servecreate(Chan *c, char *name, int omode, ulong perm)
{
	Serve *ss;
	int i;

	validwstatname(name);
	for(i = 0; i < nserves; i++)
		if(strcmp(serves[i]->name, name) == 0)
			error(Eexist);
	ss = servenew(name, 0666);
	servelink(ss);

	if(omode != OWRITE)
		error(Ebadarg);

	c->offset = 0;
	c->flag |= COPEN;
	c->mode = openmode(omode);
	c->qid.type = QTFILE;
	c->qid.vers = 0;
	c->qid.path = ss->id;
}

static void
serveclose(Chan* c)
{
	if((c->flag & COPEN) == 0)
		return;
	if(c->flag & CRCLOSE)
		serveunlink(c->qid.path);
}


static long
serveread(Chan *c, void *va, long count, vlong offset)
{
	if(c->qid.type & QTDIR)
		return devdirread(c, va, count, 0, 0, servegen);

	error(Ebadarg);
	return -1;
}

static long
servewrite(Chan *c, void *va, long count, vlong offset)
{
	int fd;
	char buf[32];
	Serve *ss;

	if(c->qid.type & QTDIR)
		error(Eperm);

	ss = servefind(c->qid.path);
	if(ss == nil)
		error(Enonexist);
	if(ss->c != nil)
		error(Ebadarg);

	if(count+1 > sizeof buf)
		error(Ebadarg);
	memmove(buf, va, count);
	buf[count] = 0;
	fd = atoi(buf);

	ss->c = fdtochan(up->env->fgrp, fd, -1, 0, 1);
	return count;
}

static void
serveremove(Chan *c)
{
	if(c->qid.type & QTDIR)
		error(Eperm);

	if(!serveunlink(c->qid.path))
		error(Enonexist);
}


Dev servedevtab = {
	0x0441, /* L'с' */
	"serve",

	devinit,
	serveattach,
	servewalk,
	servestat,
	serveopen,
	servecreate,
	serveclose,
	serveread,
	devbread,
	servewrite,
	devbwrite,
	serveremove,
	devwstat,
};
