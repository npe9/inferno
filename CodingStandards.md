# Details #

Use [Pike](http://www.lysator.liu.se/c/pikestyle.html)'s and [Russ](http://swtch.com/~rsc/worknotes/)'s C coding style for Limbo. Limbo is more verbose but the more you go out of your way to make it clear and concise the easier it is to understand. You can look [here](http://www.vitanuova.com/inferno/papers/descent.html) for a good introduction to writing Limbo code with a nice style.

**Don't** create huge adt's that are a pain in the butt to initialize. When your adt gets more than a certain number of members(say 7-8) split it into related subadt's or, better yet, see if you can imagine a tk style many language for manipulating it. Done right string glue is much easier to manipulate than forcing your concepts into Limbo. It also makes your data structures more accessible from the shell.

**Don't** use all caps for your constants unless they're really important or standard(i.e. DEBUG). Capitalize the first letter and use judicious Capitals inside to make them clearer. Resist the urge to make them huge as well. CpuSess obscures the surrounding code less than CurrentCpuSession. If it's unclear, add a comment to the definition.

**Don't** use native inferno stack lists when a cyclic ref linked list is more intuitive.

**Do** rename large global data structures(especially indexed ones) when you are manipulating them in local code. It's much easier to read
```
	c := cpusession[sid];
	c.fid = m.fid;
	c.cpuid = cpuid;
	c.omode = mode;
	c.sync = chan of int;
	c.proxyid = proxyid;
```
than
```
	cpusession[sessid].fid = m.fid;
	cpusession[sessid].cpuid = cpuid;
	cpusession[sessid].omode = mode;
	cpusession[sessid].sync = chan of int;
	cpusession[sessid].proxyid = proxyid;

```

examples of good limbo coding style are in limbo.b and mash.

**Don't** import a lot of things from other module's namespaces. Module prefixes do make code more verbose but they also show the hierarchy of the modules involved and make them more explicit. Importing adt's(like Iobuf) is fine, but don't go the whole hog(inferno's acme is an example of this).

**Do** create mini-constructors to go with your adts. Right now a ton of limbo code is littered with massive declarations. If you have a common way of instantiating your adts make a function that will allow you to generate it with the minimum of information.