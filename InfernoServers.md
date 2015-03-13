# Introduction #

As a distributed system inferno provides the user with a lot of different capabilities, an inferno instance can be a file server, a service registry or a computation node. You can even run multiple emulated instances of Inferno on a single host, each providing a different service. This tutorial gives you


# Details #

The following command starts a simple cpu server in the background. You can use nohup or screen to keep it running in the background after your session finishes.
```
emu -d /dis/emuinit.dis sh -c "{svc/net; svc/rstyx}"
```