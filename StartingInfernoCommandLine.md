# Introduction #

Default inferno environment is quite different from what others like perl or python provides.

This article tries to show how to set up an inferno environment which behaves like perl or python do, in the sense of executing programs.

The objetive is being able to call inferno programs like:

```
c:\Documents and Settings\Administrator\scripts>inferno program.dis inputfile outputfile
```

or

```
[user@localhost] ~/scritps $ inferno inputfile outputfile
```

# Details #

The current approach is based on the previous work done by [acme-sac](http://acme-sac.googlecode.com) project, and basically consist of a set of script which initialize the inferno environment and execute the commands given on the input.

One script is on the host side, that is, it is done in the host shell (.bat, .sh , etc.), and the second one is done in the inferno environment.

The host script calls inferno (emu.exe) with the initialization script as its first parameter (/ios/init.sh) and its own arguments as the following parameters ($**)**

# Host side scripts #

These scripts will run the inferno environment, calling the initialization script and the  program given as a parameter:

## Windows script ##

This script assumes inferno is installed in c:\inferno and that %CD% variable contains the path from which the script is called (this work on all kind of Windows version i have at hand (Xp, Vista and 7).

A default font is specified from those included in the default svn tree.

The script calls a initialization script within the inferno environment (see Inferno side script below)

I
```
@echo off

set PATH=c:\inferno\Nt\386\bin;%PATH%
set EMU=-rc:/inferno -g1024x768 -f/fonts/misc/unicode.6x13.font

set LPWD=%CD%
emu.exe /ios/init.sh %*
```

Its main limitation is that the quoting of the Windows shell is very different from what inferno shell or programs expect, so it could be difficult to get it right at first.

### Unix script ###

The Unix script is just a translation of the Windows script and the same limitations apply. The script assumes inferno is installed in /usr/lib/inferno

```
#!/bin/sh

export PATH=$PATH:/usr/lib/inferno/Linux/386/bin
export EMU='-r/usr/lib/inferno -g1024x768 -f/fonts/misc/unicode.6x13.font'

export  LPWD=`pwd`
emu $EMU /ios/init.sh $*
```

# Inferno side scripts #

The host side scripts expects a initialization script called init.sh under $INFERNO\_ROOT/ios/ directory.

This file could be adapted to support more than one OS, at the moment I did different scripts for each platform.


## Windows version ##

The windows version expects the script /ios/drivelist.vbs in order to enumerate the windows units and make them available on the inferno environment:

```
Set objFSO = CreateObject("Scripting.FileSystemObject")
Set colDrives = objFSO.Drives

For Each objDrive in colDrives
	Wscript.Echo objDrive.DriveLetter
Next
```


It also expects the files /ios/plumbing containing a set of plumbing rules.

```
#!/dis/sh.dis
load std
user="{cat /dev/user}
bind -a '#C' /
mount -ac {mntgen} /n
mount -ac {mntgen} /usr
if {~ $emuhost Nt}{
	drives=`{os cscript /nologo ios\drivelist.vbs}
	for i in ''^$drives {
		trfs '#U' ^ $i ^ ':/' /n/$i
	}
	currentpath=`{os cmd /c 'echo %LPWD%' |
		tr -d ''| sed -n '
			s/ / /g
			s/\\/\//g
			s/^([A-Z]):/\/n\/\1/p'}
	userhome=`{os cmd /c 'echo %USERPROFILE%' |
		tr -d '' | sed -n '
			s/ / /g
			s/\\/\//g
			s/^([A-Z]):/\/n\/\1/p'}

	bind -a $userhome/ /usr/$user


	cd $currentpath
}
svc/net
bind '#s' /chan
plumber /ios/plumbing
$*
# hack for acme, which returns after exec (acme-sac)
if {~ $1 acme } {
sleep 1
waitpid=`{sed '' /prog/*/status | sort -n | grep Acme | sed 1q | sed 's/^ +([0-9]+) +.*/\1/'}
read < /prog/^$waitpid^/wait
sleep 2
}

echo halt > /dev/sysctl

```

## MacOSX version ##

This version does not need the drivelist script or anything simmilar because on Unix if you can access /, you have access to the whole namespace.

```
 #!/dis/sh.dis
load std
user="{cat /dev/user}
bind -a '#C' /
mount -ac {mntgen} /n
mount -ac {mntgen} /usr
if {~ $emuhost MacOSX}{
    trfs '#U*' /n/local >[2] /dev/null
    currentpath=`{echo 'echo $LPWD' | os sh |  sed -n '
                    s/ /␣/g
                    s/^(.*)/\/n\/local\/\1/p'}
    userhome=`{echo 'echo $HOME' | os sh |  sed -n '
                    s/ /␣/g
                    s/^(.*)/\/n\/local\/\1/p'}
    bind -a $userhome/ /usr/$user
    cd $currentpath
}
svc/net
bind '#s' /chan
plumber /ios/plumbing
$*
# hack for acme, which returns after exec (acme-sac)
if {~ $1 acme } {
    sleep 1
    waitpid=`{sed '' /prog/*/status | sort -n | grep Acme | sed 1q | sed 's/^ +([0-9]+) +.*/\1/'}
    read < /prog/^$waitpid^/wait
    sleep 2
}
echo halt > /dev/sysctl
```

## Linux version ##

This version is directly derived from the MaxOSX version, both are the same right now.
```

 #!/dis/sh.dis
load std
user="{cat /dev/user}
bind -a '#C' /
mount -ac {mntgen} /n
mount -ac {mntgen} /usr
if {~ $emuhost Linux}{
    trfs '#U*' /n/local >[2] /dev/null
    currentpath=`{echo 'echo $LPWD' | os sh |  sed -n '
                    s/ /␣/g
                    s/^(.*)/\/n\/local\/\1/p'}
    userhome=`{echo 'echo $HOME' | os sh |  sed -n '
                    s/ /␣/g
                    s/^(.*)/\/n\/local\/\1/p'}
    bind -a $userhome/ /usr/$user
    cd $currentpath
}
svc/net
bind '#s' /chan
plumber /ios/plumbing
$*
# hack for acme, which returns after exec (acme-sac)
if {~ $1 acme } {
    sleep 1
    waitpid=`{sed '' /prog/*/status | sort -n | grep Acme | sed 1q | sed 's/^ +([0-9]+) +.*/\1/'}
    read < /prog/^$waitpid^/wait
    sleep 2
}
echo halt > /dev/sysctl

```


# Examples #

Given that the host side script is called ios.bat we can exec programs like:

**execute ls:
```
Z:\General>dir
 El volumen de la unidad Z es Activity
 El número de serie del volumen es: 4C44-883A

 Directorio de Z:\General

14/10/2008  16:31    <DIR>          .
14/10/2008  16:31    <DIR>          ..
05/02/2009  18:40    <DIR>          Productos
02/03/2009  20:17    <DIR>          SOP
               0 archivos              0 bytes
               4 dirs  63.394.906.112 bytes libres

Z:\General>ios ls
Productos
SOP
Z:\General>
```**

