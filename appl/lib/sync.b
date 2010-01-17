implement Sync;

include "sys.m";
	sys: Sys;
include "sync.m";

Rendez[T].new(): ref Rendez 
{
	R := ref Rendez[T];
	R.l = chan[1] of int;
	R.x = chan of T;
	R.ctr = 0;	
	return R;
}

Rendez[T].rendezvous(R: self ref Rendez[T], val: T): T 
{
	# Lock the Rendez structure; if there was no one waiting,
	# store our value in the structure and block on the channel.
	# If there was a waiter, we extract the value and wake up the
	# waiter.

	wait := -1;
	x: T;

	R.l <-= 0; # Lock
	case R.ctr {
	0 =>
		# There is no one waiting; we store, bump the counter, and
		# receive on the Rendez's 'x' channel outside of the locked section
		R.val = val;
		R.ctr = 1;
		wait = 1;
	1 =>
		# Pickup the value, signal the other proc, and reset the Rendez
		x = R.val;
		R.x <-= val;
		R.ctr = 0;
		wait = 0;
	}
	<- R.l; # Unlock

	if (wait == 1)
		x = <- R.x;

	return x;
}

init() 
{
}
