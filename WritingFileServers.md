# Introduction #

Writing file servers for systems is an important part of programming for Inferno. It's also one of the least explained. This page attempts to give you a basis on how to write simple Styx synthetic file servers in Inferno.

# Details #

One of the central advantages of Inferno is that everything is a file. By making things files you can share them over networks, write to them from a variety of different architectures and generally provide clear, language independent interfaces to resources that are normally quite different and a pain in the butt to deal with.

However it's not entirely straightforward how to make a filesystem in Inferno. There are three ways to do it programmatically?(using sys-file2chan(2), styx-servers(2) or by responding to styx messages directly.

The man pages provide a pretty good feel for the system if you are comfortable with the concepts involved but for a new user they are pretty difficult.
It would be much nicer to have a little language to define simple file servers. Also modules are a pain in the butt, a new user shouldn't have to know all of the prerequisite modules when they are first using something. One of the advantages of the original unix system calls was they provided a quick and easy interface to the system with a minimum of knowledge about the underpinnings. A user shouldn't have to know styx and all of the aspects of a module to make all of this work.

What a user really wants is to specify a file tree and then associate it with the various things you're dealing with programmatically(this could be a way of creating new file systems in the shell as well, you create a list of hierarchy => shell closure mappings which are then handled for you.

I'm still not quite there. you're ending up with something like file2chan. which doesn't really handle arbitrary generated hierarchies either.

## Qids and Fids ##
The key to understanding Styx fileserver is understanding the concept of Qids and Fids. Qids and Fids are the way that Inferno keeps track of files. Qids are maintained by the server, the correspond to inodes in traditional file systems. But they can be allocated or distributed in any way the user wants. Each separate namespace has its own Qid space as well.

While Qids represent something "real" and unique on the server, a Fid is a way of representing **pointers** to a given Qid.

When you're thinking about 9p understanding the transactions as a matter of manipulating the Qid and Fid space.

## Using a traditional filesystem as an example ##

let's say we want to represent a traditional filesystem in Inferno. One way to implement a filesystem is to have the inodes progress consecutively. additions increment the Qids. You're still stuck though because Dirtab doesn't really handle this right. You need a way of manipulating the Dirtab to have the correct structure.

This is what the Plan 9 root filesystem driver does.

## Qids ##

A qid is a way of representing the abstraction represented by the file on the server. It's a way of keeping track of which files are open on the system and where they are.

### Ways of partitioning Qids ###


### Multilevel Filesystems using Qids ###

You can create multilevel filesystems like tcp/ or telco/ using clever partitioning of your qid space.
## Fids ##

### Fid tables ###