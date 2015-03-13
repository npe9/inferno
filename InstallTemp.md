# Introduction #

request an account
```
mkdir /usr/$username
mkdir /usr/$username/$keyring
getauthinfo default
```

to get your system out of a firewall:

```
#!/bin/bash
ROOT="$HOME/inferno"

exec emu -s -r "$ROOT" "$@" /dis/sh.dis -c "{
        load std
        or {ftest -e /net/cs} {ndb/cs}
        bind -c '#U*' /n/local
	bind -a '#C' /
	bind /net /n/local/home/npevans/9/net.alt
	dial -A tcp!go.cs.bell-labs.com!3225 {export /n/local &}
	while {} {date;sleep 60;}
}"

#!/bin/rc
rm /srv/npesurv
kill listen1 | rc
aux/listen1 -tv tcp!*!1978 /bin/rc -c 'echo -n 0 > /srv/npesurv'
```

# Details #

Add your content here.  Format your content with:
  * Text in **bold** or _italic_
  * Headings, paragraphs, and lists
  * Automatic links to other wiki pages