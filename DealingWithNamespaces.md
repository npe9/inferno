# Introduction #

Inferno filesystems can be confusing to new users. Unlike Unix changes to Inferno namespaces do not alter the state of the whole system, they only change the state for that process. Users operating from the Unix or Windows models where changes to the mount table propagate across every aspect of the system can be unpleasantly surprised when they realize that any changes they make are local.


# Details #

In Inferno every process group has a namespace. A namespace can be thought of as similar to a per process group mount table. In Unix the mount table is global for the system. This provides certain advantages and disadvantages. In Unix changes made by the administer(typically only the administrator can mount arbitrary filesystems into the filesystem namespace and users can mount a few items specifically allowed for them) are global. In Inferno they are not. When a process wants to change its namespace changes are local. This provides a lot of advantages enumerated here:

### Sandboxing ###
You can use namespaces to sandbox processes.

### Importing Remote Resources ###
In Inferno since everything is a file it's possible to alter the namespace to use remote resources.

For example this is what the CPU command does

```
cpu->code("here");
```
### Changing the Representation of Resources ###
You can also use this to change the entire represetation of a resource. For example, Windows, Linux and Mac OSX filenames often contain spaces. A convention in Inferno is to tokenize  filenames on whitespace. When you are trying to use an external filesystem in Inferno it can be impossible.

You do have an alternative though. Trfs(1) does the job for you. It looks and the file names and converts them to utf byte 2342534 which now makes it possible to tokenize on whitespace again.

```
# example using trfs on a windows machine
```

### Testing files for equality ###
```
isconsole(fd: ref Sys->FD): int
{
	(ok1, d1) := sys->fstat(fd);
	(ok2, d2) := sys->stat("/dev/cons");
	if (ok1 < 0 || ok2 < 0)
		return 0;
	return d1.dtype == d2.dtype && d1.qid.path == d2.qid.path;
}
First you test if the servers are the same, then you make sure that paths are the same as well.

```

## Mounting ##

In Inferno and plan 9 mounting is the act of taking a file descriptor and placing it in the mount table, then reading and writing to it.

to mount wmimport mount {wmimport} /n/wm