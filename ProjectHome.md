My experimentation tree. The goal of this system is to be a "living" inferno tree accessible to people new to the community. With that in mind I want to explain the system in a way that people outside the systems software research community can understand. I'd also like it to encourage open development and experimentation like Caerwyn's [Inferno Programmer's Notebook](http://www.caerwyn.com/ipn/). Eventually I want it to be a common source for Inferno folks to keep an unstable tree with common signers and ndb/locals. That way people can see, upload and look at what everyone is working on. I'm also going to add a set of scripts to allow the quick bringup of a variety of different types of inferno installs(i.e. starting with fs, auth and cpu and moving towards something more buzzwordy like a "cloud" system).

I'd also like to change the Inferno api a bit. IMO it's gotten a bit too Javaesque(the Registries module is a good example). I'd like to revert most of the fiddly big parts of the system back to working with little languages rather than complicated adt based apis.

Eventually I expect the worthwhile aspects of this to merge back into the main Inferno tree. I intend to have an extremely liberal commit and contributer access policy. If you'd like to be a contributor mail me for access.

relevant files:

[installing from source](http://inferno-npe.googlecode.com/hg/INSTALL)

[the main installation document](http://inferno-npe.googlecode.com/hg/doc/install.pdf)