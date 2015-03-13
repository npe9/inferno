# Introduction #

Inferno is a neat system. It provides a bunch of things that most operating systems and programming environments don't. It runs hosted or native, meaning that you can use it on bare hardware or you can run it on top of whatever operating system you are using.

# Details #

Inferno was originally made for settop boxes and embedded programming. It's still very good for that but since then it's morphed into a much broader system, it has advantages in a variety of fields including EmbeddedWork, GridComputing, NaturalLanguageProcessing. It gives you a standard interface.

If you need to create a distributed system with a lot of concurrency inferno can give you a pretty compelling platform. It also works very nicely for doing any sort of text processing task. The [Dis virtual machine](Dis.md) comes with a bunch of text processing built in. It also has pipelines that work the same way regardless of systems. This allows you to create a SoftwareTools style of system very easily. It's very easy to compose things in their component parts. Even inside programs things naturally break down into modules.

The system is also heavily text based, almost all text in the system(aside from the older gui tools) can be used created and used as both input and output from other programs.

The Inferno shell, which has  tcl style text closures[cite](cite.md) is very good for dealing with text based things from user space, including managing the gui.
