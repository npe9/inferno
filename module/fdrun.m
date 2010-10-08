FDrun: module {
	PATH: con "/dis/fdrun.dis";
	init: fn();
	run: fn(ctxt: ref Draw->Context, args: list of string, spec: string, sfds: array of ref Sys->FD, result: chan of string): int;
};
