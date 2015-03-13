# Introduction #

Inferno is very good for making prototype and distributed systems. Ways of doing novel things or making the system easier to use.

# Details #


## A Web server ##

The web is an increasingly important interface. Rails and Seaside as ways of writing web applications. Come up with a simple way using inferno to write scalable web applications.

## A Modular Awk ##

One of the main issues I have with Inferno is that many of its interfaces have moved away from little languages. The shell, the registry interface etc... all of these have cumbersome programmatic interfaces that would be better solved using a domain specific language like printf.

In this vein a version of awk is very important to inferno. While most of awk's capabilities can be gotten around using sh(1) it's still a lot more cumbersome than awk's spare and powerful syntax for dealing with streams. Also given Inferno's native support for utf8 a new modular awk would be an interesting place to explore the implications of dealing with streaming utf in a variety of languages.

## An Acid style debugger with an acme interface ##

On the subject of little languages when debugging inferno applications the current interfaces don't really do it for me. Inferno already provides a nice filesystem debugging interface, so it shouldn't be that hard to do(tcl?). Then attach an Acme client to it to make it more worthwhile. Caerwyn has a debugging client for acme that seems nice, but I don't have enough experience with it yet.

## Multicore ##

Right now Inferno is a highly concurrent language but it doesn't really take advantage of multicore systems. A way of scheduling threads across cores would be really nice.

## Acme Replacement ##
Acme is awesome, but with new devices like iphones and nintendo ds's running around the traditional mouse and windows method doesn't really work. is there a way of creating an editor for programmers which takes acme's advantages(all text is executable, the ability to work across files etc...) and take advantage of them in small, large and everything in between environments?

## Path Based Drawing ##

Inferno's blit based draw interface is nice, but I wonder if it might not be better to have a path based set of rendering drawing primitives. This would give Inferno the ability to deal with text, lines and rectangles etc... in a resolution independent way. Something very nice for an OS which runs on as many devices as inferno.

There would have to be some way of varying the quality of the rendering to work on slow devices(how does the iphone do this?).

## An NLP system based on EC2 ##

Right now NLP problems are extremely computationally intensive, it's difficult to find and implement tools to solve NLP problems as well as acquire the resources to solve NLP problems. With Amazon's EC2 users finally have the chance to get a large amount of computational power on a temporary basis and use that power to solve problems quickly. A system using  Inferno's portability and EC2's power would make solving NLP problems quick and easy.

## A consistent way of interacting ##

Inferno provides a great way of distributing things. However, on the other hand it doesn't provide a good way of publishing what is going on. The registry is good for low level stuff, but some standardized way of doing collaborative work, with version control in a plan 9 way, communication and publishing methods(wikis?) etc... that allow Inferno users to act as a more coherent community. This is not so much an a systems research problem as an operational one. What sort of tools does inferno need to take its ease of distribution and turn a bunch of ad hoc sites  into a whole that is greater than the sum of its parts?

## Something like google docs ##
What's the point of a distributed system if you can't use it for distributed tasks? Some interesting coherent way of allowing people to deal with constantly updated things in a coherent way. I really don't have a coherent idea of how it should be done. But I'd love something like subethaedit or google docs in Inferno.

## Use Inferno as a bootloader ##
I'd like to be able to use inferno as a boot loader. This idea keeps getting kicked around with plan 9 as well and I think Inferno has been used as a boot loader before.

## A Threadsafe Inferno Bufio library ##


## More list operations in Limbo ##
I'd really like to have a bunch of more functional commands in limbo. I notice that I'm constantly doing losts of bookkeeping. I'd like to be able to do list transformations(i.e. get the type of the return of a function, create a list of that function, apply the functions to each element of the list then return the new list). I can't see any way to do this in Limbo as it stands(maybe with Brucee's ref any?

## Code visualization tools ##
Limbo adt's can be very heavily nested. Some tool to generate grap data or something similar would make reading and understanding Limbo programs a lot easier.

## Persistent 9p sessions ##
When I close my terminal in acme sac my 9p sessions die. I want to be able to have persistent 9p sessions in inferno.

## Adt instantiation ##
a big pain in inferno is dealing with adt instantiation after you change the interface to your adt strcuture. this means that every program that uses your module will fail to compile if you change that interface. This is not good, if you want to add elements to the interface you need to recompile and recreate all of your tools based on your module.

## Programming Language Shootout ##

Limbo isn't represented on the[programming language shootout page](http://shootout.alioth.debian.org/). That would be a good type of evangelism to get our way of doing things out.

## An NLP Language ##
A good cache optimized associative dictionary.
Lots of floating point operations.
copy on write/ropes string handling to give users the ability to do a lot of different string handling.

## limbo module name completion ##
This is asking for trouble. I'm probably going to do it anyway.

## Statistics Library ##
I want to do a lot of statistics work in Inferno. A statistics module(implemented in C with a Limbo interface), would be a very good start.

## Dynamic Module Loading in Limbo ##
It is painful to load all the dependent modules manually:
```
dep1 := load Dep1 Dep1->PATH;
dep2 := load Dep2 Dep2->PATH;
m := load M M->PATH;
m->init(dep1, dep2);
```

Instead module dependencies can be resolved automatically (e.g. as it is done in Java with class loading).

## Private Visibility in ADTs ##
To be able to hide implementation details from ADT users.

## Read-Only Variables in ADTs ##
Something like _con_ for ADT members (const, final etc.)

## S3 Filesystem ##

Using s3 as a backend for an inferno filesystem(as simple as storing a kfs file in a bucket, would be a great way to give inferno a more permanent place on the web.