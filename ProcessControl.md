#summary Information about Inferno processes and how to manipulate them.
# Introduction #

Inferno handles its processes a lot like Unix, but there are some subtle differences. This page describes how to use inferno processes and what they are good for.


# Details #

Inferno divides its processes hierarchically the same as Unix. This means that every process(other than the first process, typically emuinit.dis) has a parent process and potentially child processes. This leads to a lot of sharing and passing of data structures between processes.

In Inferno processes are actually threads spawned by other threads running in the dis interpreter. This is different from traditional Unix where every process is actually an in memory image running on the physical processor.

In limbo the syntax to spawn a process is:
```
include "sh.m";

fn()
{
	cmd := load Command "/dis/cmd.dis"
	cmd->init(drawcontext, "cmd"::"list"::"of"::"arguments"::nil); # or spawn cmd->init(drawcontext, "cmd"::"list"::"of"::"arguments"::nil);
}
```

It's actually no different than importing a module function into your code, so processes are actually the same thing as library functions. Coming from Unix this can be disconcerting.

Every process comes with a set of data structures as well: file descriptors, name spaces, environments.

You have file descriptor groups. A file descriptor is a way of denoting open files in the system.

Name spaces are the base filesystem your system sees.

Environments are your environment variables. This is another place where Inferno(and Plan 9 before it) were fundamentally different than Unix. Environment variables in Inferno are actually a set of files rooted at /env.

## Manipulating Process Specific Data Structures ##
You can manipulate the process specific data structures with a function call `sys->pctl`(2). `pctl` allows you to perform a variety of tasks. For instance if you need to redirect the standard input and output file descriptors you can use the pctl flag `Sys->FORKFD` to create a separate copy of your existing file descriptors for your group. You can also specify certain file descriptors to move.

The following example from :: shows how to use it.

```


```

What it's doing is taking
## Changing the Process Group ##
If you'd like the change the hierarchy of a process group in Inferno you can do it using the

## Dumping the state of the variables ##
When you're doing development in Limbo it's often useful to know the state of the system you're in. i.e. your current namespace, your file descriptors etc...

The following limbo function dumps this data for you:
```
dumpgrps()
{
# environment
# fds
# ns
}
```