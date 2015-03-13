# Introduction #

One one to use inferno is like any other scripting language. For these purpose, some binary packages have been created with the [inferno-os](http://inferno-os.googlecode.com) code.

# Installing #

Install the packages as any other software, the locations choosed by the installers must be respected :). May be the windows installer try to install Inferno under c:\Program files\... , i would choose C:\Inferno, the inferno launcher script could fail with directory names with spaces.

In the these distributions, the inferno launcher script is called ios, and the initialization script is called init.sh. see inferno command line [page](StartingInfernoCommnadLine.md) for the details of these scripts.

## Windows ##

The suggested directories to install the software are:

  * inferno root: c:\Inferno\
  * emu binaries: c:\Inferno\Nt\386\bin

The windows package comes in MSI format, and should be installable on your windows machine like any other program. Just doubleclick the file and press "Next" a couple of times.

The package is called inferno-NNN.msi, where NNN stands for the svn release number.

## Mac OSX ##

The suggested directories to install the software are:

  * inferno root: /Library/Inferno
  * emu binaries: /Library/Inferno/MacOSX/386/bin

The Mac OSX package comes in .pkg format, and should be installable on your mac machine like any other program. Just doubleclick the file and press "Next" a couple of times.

## Linux ##

The suggested directories to install the software are:

  * inferno root: /usr/lib/inferno
  * emu binaries: /usr/lib/inferno/Linux/386/bin

The Linux .tgz package contains a directory called 'inferno' which contains all the files needed. You can install this manually by extracting it in the path you want, etc.

There could be other packages such as .deb for Ubuntu distribution, or .rpm for Fedora. Those packages should be usable in other Linux distributions with those package management systems.

# Set up #

Add the binaries location to your PATH variable (win , mac, linux) to being able to use the launcher script right away.

For example:

```
Microsoft Windows XP [Versión 5.1.2600]
(C) Copyright 1985-2001 Microsoft Corp.

C:\>ios pwd
/n/C
C:\>
```

Visit this [page](StartingInfernoCommnadLine.md) to see how that ios script is made.