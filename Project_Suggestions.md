As we think of them, we'll add to the following lists of helpful things to do.  If you are planning to attempt one, please join the [Inferno e-mail list](http://www.vitanuova.com/news/newsgroup.html) and discuss it there; if you are particularly earnest, you might even tag the project with your name here.

The mark (GW) stands for "Grunge Work".

First, some Q&A:

  * _Implementation languages?_
> Inferno kernel code and the supporting libraries are written in either fairly strict ANSI C (ie, no GNU extensions) or [Plan 9 C](http://plan9.bell-labs.com/sys/doc/comp.html), depending on target environment.

> Inferno applications are written in the [Limbo programming language](http://www.vitanuova.com/inferno/papers/limbo.html), possibly controlled by small scripts written in the [Inferno shell](http://www.vitanuova.com/inferno/papers/sh.html).

> Nearly all the projects below would be written in Limbo, so be prepared. Most would involve writing at least one [Styx](http://www.vitanuova.com/inferno/man/5/0intro.html) file server, which is well-supported by either [system calls](http://www.vitanuova.com/inferno/man/2/sys-file2chan.html) or [library modules](http://www.vitanuova.com/inferno/man/2/styxservers.html).

  * _Style?_
> It's much easier for us if it follows the style of most of the existing code in our tree.

> For background, see [Rob Pike's comments](http://www.lysator.liu.se/c/pikestyle.html) or [The Practice of Programming](http://cm.bell-labs.com/cm/cs/tpop/), and [Russ Cox's more recent ones](http://swtch.com/~rsc/worknotes/) for programming projects; they give the gist. We maintain a similar style for Limbo.

  * _Licence?_
> We expect the projects would use an MIT-style licence, except for minor modifications made to existing code covered by the LGPL or GPL.

  * _Distribution?_
> During development, there should be a reasonably current copy of your work in a project on Google Code. On completion, we'd hope to add the code to the Inferno distribution.

## Summer-sized projects ##

We came up with the following project ideas for Google's [Summer of Code 2007](http://code.google.com/soc/), now ended.  Feel free to do any of them in spring, winter, or autumn. Several of these projects are big enough to be split up. We might add a few more from time to time.  Watch this space.

There is a separate (but related) page for [Plan 9 projects](http://plan9.bell-labs.com/wiki/plan9/TODO/index.html) that you might also check.

These are our own suggestions: feel free to offer ideas of your own. [The Inferno Programmer's Notebook](http://www.caerwyn.com/ipn/) might give you an idea.

Now, the currently typed-in project suggestions, in no particular order:

  * _Personal Naming Scheme_
> A recent MIT paper (http://pdos.csail.mit.edu/papers/uia:osdi06-abs.html) describes a scheme for an _Unmanaged Internet Architecture_ based on grouping and secure exchange of personal names for resources, including working through NAT layers. This decentralised approach seems attractive to play with in Inferno.  The task is to implement agreed parts of the paper, including its routing method (which might be more generally useful), probably using file server interfaces to many of the services, and seeing what existing parts of Inferno can help, and which other parts can be changed or extended to simplify the implementation.

  * _JavaScript outside_
> There is an ECMAscript implementation in the Limbo library, and it is currently used only by the browser Charon. It implements (or is intended to implement) all of ECMAscript, but the language defined by the standard is intended for use in many environments and does not specify all that people take to be JavaScript. For instance, it does not provide DOM (which comes as a shock to most JavaScript that is loaded by Charon, so less and less works these days). The task is: ensure the language implements the current ECMAscript standard; add support for the JavaScript objects/libraries; and make the module available as a standalone general-purpose language under Inferno. It would be nice to re-attach it to the browser Charon so that it will do Google mail properly (and not bite that feeding hand!), but that is suspected to be fairly hard in the time available. There should, however, be ways to access the Inferno environment.

  * _Restrained colour_
> "Selecting harmonious colours for traditional window systems can be a difficult and frustrating endeavour." Indeed many window systems look unpleasant to me. A thesis by MacIntyre (http://www.cs.uwaterloo.ca/research/tr/1991/55/tech.pdf) has been on my<sup>1</sup> shelf for quite some time. It describes a scheme for colour constraints to be used to control dynamic colour choice by a user interface automatically. I am interested in seeing how well it works in practice in the Inferno environment. I supposed it might be implemented as an Inferno service that could be used directly by graphics applications or indirectly through Tk (eg, a given packing would access the solver). The hope is to apply it to existing applications, with it able to be used by default. Badros (http://citeseer.ist.psu.edu/badros00extending.html) describes a more general use of constraints in interactive systems that might be worth considering too. The project would start with a little survey to see what else that is relevant might have been done since then, but of course this is primarily an implementation project.  <sup>1</sup>_charles.forsyth@gmail.com_

  * _Ventilator_
> [Venti](http://plan9.bell-labs.com/sys/doc/venti.html) is a [Plan 9](http://plan9.bell-labs.com/plan9/) archival data service. It would be useful for Inferno to have a simple file system implementation that uses an existing Venti for its archive and initial state:
    * it would have a basic implementation (Plan 9's own [Fossil](http://plan9.bell-labs.com/magic/man2html/4/fossil) is fairly complicated, at least for a summer project, and in retrospect has some limitations)
    * it could store file system metadata and data locally (and optionally separately)
    * it would allow disconnected operation
    * it could be fairly slow (doesn't matter at this point)

  * _Distiller_
> Design a tree-based internal form for a program, almost certainly typed and perhaps related to the one used by the [Limbo](http://www.vitanuova.com/inferno/papers/limbo.html) compiler, and write a Limbo module that translates it to a [Dis module](http://www.vitanuova.com/inferno/papers/dis.html) at run-time (but also allowing the results to be filed).

  * _httpd_
> Modify one of the httpd implementations (probably Mechiel's) to allow a process (or a set of processes) to maintain state for a logical http session, so the server-side application is written as a (fairly) normal process. Write some useful applications.

  * _Open season_
> Project areas that seem interesting but for which we have not (yet) provided details:
    * Sound (eg, music-related)
    * Data display, perhaps influenced by ideas of [Edward Tufte](http://www.edwardtufte.com) and others.
    * Inferno as a plug-in for Firefox (see http://code.google.com/p/inferno-plugin/ for the existing IE plugin, and the Links there for further details)
    * A Styx client implementation in JavaScript, ideally running in all browsers, and used to implement some non-trivial and useful applications.
    * A plug-in for Firefox (or any other suitable browser) that makes things inside the browser available on a Styx connection. That _might_ even include access to its user interface (that is probably hard, if only because a suitable programming interface would need to be designed as well as implemented).
    * Zeroconf (see below).
    * Styx access to host system resources, such as services or devices (see below).
    * Write a distributed application using one or more Inferno file servers to simplify implementation.

## Infrastructure ##

  * _TLS/SSL3.0_
> `/appl/lib/crypt/ssl3.b` has an implementation of SSL3(TLS), `/appl/lib/crypt/sslsession.b` implements some form of SSL session cache, and `/*/port/devtls.c` has support for the record structure and control operations of SSL3 (but ssl3.b doesn't use it).  Check ssl3.b etc. against the specification, and change it to suit. You might also need to change x509.b. (GW)

  * _64 bit Inferno_
> The Inferno kernels could be modified in a similar way as Plan 9 to run on 64-bit architectures, but could only run 32-bit Dis; a 64-bit variant of Dis would be needed to make full use of 64-bit address spaces.

  * _Open GL support_

  * _Zeroconf_
> [Zeroconf](http://en.wikipedia.org/wiki/Zeroconf) support/mapping within Inferno, probably hooked into [ndb/cs](http://www.vitanuova.com/inferno/man/8/cs.html) and/or [ndb/registry](http://www.vitanuova.com/inferno/man/4/registry.html)

  * _Access to host's devices_
> Better support for importing more eccentric devices from the hosted system - ie. video (from camera and/or video capture), tablet/pen controls w/pressure sensitivity, 3D mice, etc.; and then write Inferno applications to use them.

  * _Window resizing for hosted inferno_
> for one or more of: x11, windows, mac os x.

## Applications ##

  * _Inferno on T3_
> Help work on applications for Caerwyn's Inferno port to the Palm Tungsten T3 (see http://caerwyn.com/ipn/)

  * _svnfs_
> Provide an Inferno file system interface to [Subversion](http://www.subversion.org). Read-only is fine for now.

  * _Google APIs_
> Write file servers or modules to give access to some of the more interesting [Google APIs](http://code.google.com/apis/), where "more interesting" means you also write a useful and/or amusing application that uses the API(s) just implemented.

  * _Charon_
    * Add CSS support to the web browser [Charon](http://www.vitanuova.com/inferno/man/1/charon.html).
    * Convert the layout/output code into a (more) general library module for other programs.
    * Finish the implementation of HTTP/1.1, in particular fetching several things on a single connection.

  * _Acme_ ([A User Interface for Programmers](http://plan9.bell-labs.com/sys/doc/acme.pdf))
> Write some more Acme clients, for instance ...
    * a client for [Google gtags](http://google-code-updates.blogspot.com/2007/03/google-gtags-version-10.html)
    * clients to access other Google services (see Google APIs above)

  * _PDF_
> Write a PDF viewer(!).  An [pdf parser](http://www.ueber.net/code/r/pdfread) is available (not renderer!) and could be a useful start.

  * _mp3 decoder/player_
> Perhaps port an existing bsd/mit-licensed implementation to Limbo

## Ports ##

There are many potential targets for Inferno ports, either hosted, or native, or both.
Native ports might be tricky for summer of code projects, but hosted ones are plausible. Hypervisors are somewhere in between. Here are some that various people have suggested. If you are interested, drop a line to the email address above and your message will be forwarded to the person who originally expressed interest.  All of the native ports assume access to suitable hardware.


  * _Inferno on Nintendo DS_
> Help work on [Noah Evan's port of Inferno to the Nintendo DS](http://code.google.com/p/inferno-ds/), started as part of Google Summer-of-Code 2007.

  * _Gumstix_ (In progress.)

  * _PS3_
> PS3 port w/ppe file system interfaces and support for ps3 devices (controllers) - either hosted on Linux and/or native on the PS3 hypervisor

  * _OLPC_
> Hosted should just work with small effort since it's Linux/x86 (but there might be interesting file servers to write); Native would be more challenging but good to do.

  * _Sheevaplug_ (Marvell Kirkwood system-on-chip)
> Inferno already boots http://code.google.com/p/inferno-kirkwood/, needs more work.

  * _Hypervisors_
> Native support for hypervisors (eg, Xen, PAPR, KVM, ...).

> Native Inferno running in the same controlling environment as User Mode Linux. (The rationale is that some server companies offer virtual Linux environments based on UML that could boot User Mode Native Inferno instead.)

## Packaging ##

Packaging is often the ugly duckling of software (not just free software). That's because what you have to do usually _is_ ugly.

  * _.deb/.rpm/ebuild packaging for Linux distributions_ (GW)
> As with some other large packages, this would probably involve producing several packages, each corresponding to particular functionality (eg, a base set, hosted vs native, graphics and wm, supporting tools). For instance, those who just want to serve resources shouldn't need to install a big tree of unused material.
> The Vita Nuova distribution is actually built from some subpackages (inferno, src, os, utils) but those were not intended to form the basis for a shipped application. The grid scheduler client, for instance, uses a stripped-down Inferno for each client.
> Once we have something that we like, we can try to have it adopted by the distributions.