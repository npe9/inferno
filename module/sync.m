Sync: module 
{
	PATH:	con "/dis/lib/sync.dis";
	
	Rendez: adt[T] {
		l: chan of int;
		val: T;
		ctr: int;
		x: chan of T;

		rendezvous: fn(nil: self ref Rendez[T], val: T): T;
		new: fn(): ref Rendez[T];
	};

	init: fn();
};
