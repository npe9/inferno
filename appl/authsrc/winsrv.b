implement Newwin;
# serve /n/rwin/new, allowing new shell windows to be created
# by remote agents. assumes mntgen on /n.
# write kind of window to be created into /n/rwin/new (sh or rio); read name of directory
# containing cons/consctl, etc

include "sys.m";
	sys: Sys;
include "draw.m";
include "sh.m";
	sh: Sh;

Newwin: module {
	init: fn(nil: ref Draw->Context, nil: list of string);
};

init(ctxt: ref Draw->Context, nil: list of string)
{
	sys = load Sys Sys->PATH;
	sh = load Sh Sh->PATH;

	sys->pipe(p := array[2] of ref Sys->FD);
	spawn srv(ctxt, p[0], sync := chan of int);
	p[0] = nil;
	<-sync;
	if(sys->mount(p[1], nil, "/n/rwin", Sys->MREPL, nil) == -1)
		raise "fail:cannot mount";
}

srv(ctxt: ref Draw->Context, fd: ref Sys->FD, sync: chan of int)
{
	sys->pctl(Sys->FORKNS | Sys->FORKFD, nil);
	sync <-= 0;
	spawn export(fd, "/n/rwin");

	sys->bind("#s", "/n/rwin", Sys->MBEFORE);
	sh->run(nil, "mount" :: "-a" :: "{mntgen}" :: "/n/rwin" :: nil);
	fio := sys->file2chan("/n/rwin", "new");
	if(fio == nil){
		sys->print("cannot make /chan/newwin: %r");
		return;
	}
	spawn srv0(ctxt, fio);
}

export(fd: ref Sys->FD, d: string)
{
	sys->export(fd, d, Sys->EXPWAIT);
}

srv0(ctxt: ref Draw->Context, fio: ref Sys->FileIO)
{
	pending: list of (int, array of byte);
loop:
	for(;;)alt{
	(nil, data, fid, wc) := <-fio.write =>
		if(wc == nil)
			break;
		if(len data > 0 && data[len data - 1] == byte '\n')
			data = data[0:len data - 1];
		for(p := pending; p != nil; p = tl p)
			if((hd p).t0 == fid){
				wc <-= (-1, "request already pending");
				continue loop;
			}
		(d, e) := newwin(ctxt, string data);
		if(d == nil)
			wc <-= (-1, sys->sprint("cannot start shell: %s", e));
		else{
			pending = (fid, array of byte d) :: pending;
			wc <-= (len data, nil);
		}
	(offset, nb, fid, rc) := <-fio.read =>
		p: list of (int, array of byte);
		if(rc == nil){
			for(; pending != nil; pending = tl pending)
				if((hd pending).t0 != fid)
					p = hd pending :: p;
			pending = p;
			break;
		}
		d: array of byte;
		for(; pending != nil; pending = tl pending){
			if((hd pending).t0 == fid)
				d = (hd pending).t1;
			else
				p = hd pending :: p;
		}
		if(offset >= len d)
			rc <-= (nil, nil);
		else{
			rc <-= (d, nil);
			if(offset + nb < len d)
				pending = (fid, d) :: pending;
		}
	}
}

n := 0;

newwin(ctxt: ref Draw->Context, kind: string): (string, string)
{
	d := sys->sprint("%d", n++);
	case kind {
	"sh" =>
		e := sh->run(ctxt, "/dis/remotesh" :: "-s" :: "/n/rwin/"+d :: nil);
		if(e != nil)
			return (nil, e);
	"rio" =>
		e := sh->run(ctxt, "9win" :: "-s" :: "/n/rwin/"+d :: nil);
		if(e != nil)
			return (nil, e);
		sys->bind("/dev", "/n/rwin/"+d, Sys->MAFTER);
	* =>
		return (nil, "known kind of window");
	}
	return (d, nil);
}
