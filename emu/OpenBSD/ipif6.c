/*
 * ipv6 mixed with ipv4 on openbsd:
 *
 * openbsd's AF_INET6 sockets will not do ipv4-mapped addresses (see inet6(4)).
 * we create an ipv4 socket by default.  if we are asked to
 * connect/bind to ipv6, we create a new ipv6 socket, and dup it
 * over the original ipv4 socket.
 *
 * caveats:
 * - we can listen on an explicit ipv6 address.
 *   listening on the "any address" is always ipv4 only.
 * - socket options set before dialing/announcing will be lost when
 *   dialing ipv6 (though eg keepalive can probably only set on connected
 *   sockets).
 */

#ifdef sun
#define	uint uxuint
#define	ulong uxulong
#define	ushort uxushort
#endif
#include <sys/types.h>
#include	<sys/time.h>
#include	<sys/socket.h>
#include	<net/if.h>
#include	<net/if_arp.h>
#include	<netinet/in.h>
#include	<netinet/tcp.h>
#include	<arpa/inet.h>
#include	<netdb.h>
#include	<sys/ioctl.h>
#undef ulong
#undef ushort
#undef uint

#include        "dat.h"
#include        "fns.h"
#include        "ip.h"
#include        "error.h"

char Enotv4[] = "address not IPv4";

static void
ipw6(uchar *a, ulong w)
{
	memmove(a, v4prefix, IPv4off);
	memmove(a+IPv4off, &w, IPv4addrlen);
}

static int
so_socket0(int type, int family)
{
	int fd, one;

	fd = socket(family, type, 0);
	if(fd < 0)
		oserror();
	one = 1;
	if(type == SOCK_DGRAM)
		setsockopt(fd, SOL_SOCKET, SO_BROADCAST, (char*)&one, sizeof(one));
	else
		setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char*)&one, sizeof(one));
	return fd;
}

int
so_socket(int type)
{
	switch(type){
	case S_TCP:
		type = SOCK_STREAM;
		break;
	case S_UDP:
		type = SOCK_DGRAM;
		break;
	default:
		error("not tcp or udp");
	}
	return so_socket0(type, AF_INET);
}

static int
sockisfamily(int fd, int family)
{
	struct sockaddr_storage ss;
	socklen_t len;

	len = sizeof ss;
	return getsockname(fd, (void*)&ss, &len) >= 0 && ss.ss_family == family;
}

/*
 * did we already bind an address or port to this socket?
 * if not, we can dup another one over it.
 */
static int
isbound(int fd)
{
	struct sockaddr_storage ss;
	struct sockaddr_in *sin;
	struct sockaddr_in6 *sin6;
	socklen_t len;

	len = sizeof ss;
	if(getsockname(fd, (void*)&ss, &len) < 0)
		return 1;
	if(ss.ss_family == AF_INET) {
		sin = (struct sockaddr_in*)&ss;
		return sin->sin_addr.s_addr != 0 || sin->sin_port != 0;
	} else {
		sin6 = (struct sockaddr_in6*)&ss;
		return memcmp(&sin6->sin6_addr, IPnoaddr, IPaddrlen) != 0 || sin6->sin6_port != 0;
	}
}

static int
socksetfamily(int ofd, int v4)
{
	int family, type, nfd, r;
	socklen_t len;

	family = v4 ? AF_INET : AF_INET6;

	if(ofd < 0 || sockisfamily(ofd, family))
		return ofd;
	if(isbound(ofd))
		return -1;

	len = sizeof type;
	if(getsockopt(ofd, SOL_SOCKET, SO_TYPE, (void*)&type, &len) < 0)
		return -1;
	if(type != SOCK_STREAM && type != SOCK_DGRAM)
		return -1;

	nfd = so_socket0(type, family);
	if(nfd < 0)
		return -1;
	r = dup2(nfd, ofd);
	close(nfd);
	return r;
}

static int
sendto4(int sock, void *va, int len, char *h, int hdrlen)
{
	struct sockaddr sa;
	struct sockaddr_in *sin;

	memset(&sa, 0, sizeof(sa));
	sin = (struct sockaddr_in*)&sa;
	sin->sin_family = AF_INET;
	switch(hdrlen){
	case OUdphdrlenv4:
		memmove(&sin->sin_addr, h,  4);
		memmove(&sin->sin_port, h+8, 2);
		break;
	case OUdphdrlen:
		v6tov4((uchar*)&sin->sin_addr, h);
		memmove(&sin->sin_port, h+2*IPaddrlen, 2);	/* rport */
		break;
	default:
		v6tov4((uchar*)&sin->sin_addr, h);
		memmove(&sin->sin_port, h+3*IPaddrlen, 2);
		break;
	}
	return sendto(sock, va, len, 0, &sa, sizeof(sa));
}

static int
sendto6(int sock, void *va, int len, char *h, int hdrlen)
{
	struct sockaddr_storage sa;
	struct sockaddr_in6 *sin6;

	memset(&sa, 0, sizeof(sa));
	sin6 = (struct sockaddr_in6*)&sa;
	sin6->sin6_family = AF_INET6;
	switch(hdrlen){
	case OUdphdrlenv4:
		error(Enotv4);
		break;
	case OUdphdrlen:
		memmove(&sin6->sin6_addr, h, IPaddrlen);
		memmove(&sin6->sin6_port, h+2*IPaddrlen, 2);	/* rport */
		break;
	default:
		memmove(&sin6->sin6_addr, h, IPaddrlen);
		memmove(&sin6->sin6_port, h+3*IPaddrlen, 2);
		break;
	}
	return sendto(sock, va, len, 0, (void*)&sa, sizeof *sin6);
}

int
so_send(int sock, void *va, int len, void *hdr, int hdrlen)
{
	int r;
	char *h = hdr;
	int v4;

	osenter();
	if(hdr == 0)
		r = write(sock, va, len);
	else {
		/*
		 * we check for v6Unspecified so bad sends return ipv4-like error messages,
		 * and so we do not needlessly convert to an ipv6 socket.
		 */
		v4 = hdrlen == OUdphdrlenv4 || isv4(h) || ipcmp(h, v6Unspecified) == 0;
		if(socksetfamily(sock, v4) < 0)
			return -1;

		if(v4)
			r = sendto4(sock, va, len, h, hdrlen);
		else
			r = sendto6(sock, va, len, h, hdrlen);
	}
	osleave();
	return r;
}

static void
headersv4(int sock, void *hdr, int hdrlen, struct sockaddr_storage *sa)
{
	char h[Udphdrlen];
	struct sockaddr_in *sin;
	socklen_t len;

	sin = (struct sockaddr_in*)sa;

	memset(h, 0, sizeof h);
	switch(hdrlen){
	case OUdphdrlenv4:
		memmove(h, &sin->sin_addr, IPv4addrlen);
		memmove(h+2*IPv4addrlen, &sin->sin_port, 2);
		break;
	case OUdphdrlen:
		v4tov6(h, (uchar*)&sin->sin_addr);
		memmove(h+2*IPaddrlen, &sin->sin_port, 2);
		break;
	default:
		v4tov6(h, (uchar*)&sin->sin_addr);
		memmove(h+3*IPaddrlen, &sin->sin_port, 2);
		break;
	}

	/* alas there's no way to get the local addr/port correctly.  Pretend. */
	memset(sa, 0, sizeof *sa);
	len = sizeof sa;
	getsockname(sock, (void*)sa, &len);
	switch(hdrlen){
	case OUdphdrlenv4:
		memmove(h+IPv4addrlen, &sin->sin_addr, IPv4addrlen);
		memmove(h+2*IPv4addrlen+2, &sin->sin_port, 2);
		break;
	case OUdphdrlen:
		v4tov6(h+IPaddrlen, (uchar*)&sin->sin_addr);
		memmove(h+2*IPaddrlen+2, &sin->sin_port, 2);
		break;
	default:
		v4tov6(h+IPaddrlen, (uchar*)&sin->sin_addr);
		v4tov6(h+2*IPaddrlen, (uchar*)&sin->sin_addr);	/* ifcaddr */
		memmove(h+3*IPaddrlen+2, &sin->sin_port, 2);
		break;
	}
	memmove(hdr, h, hdrlen);
}

static void
headersv6(int sock, void *hdr, int hdrlen, struct sockaddr_storage *sa)
{
	char h[Udphdrlen];
	struct sockaddr_in6 *sin6;
	socklen_t len;

	sin6 = (struct sockaddr_in6*)sa;

	memset(h, 0, sizeof h);
	switch(hdrlen){
	case OUdphdrlenv4:
		error("IPv6 packet on headers4 socket");
		break;
	case OUdphdrlen:
		memmove(h, &sin6->sin6_addr, IPaddrlen);
		memmove(h+2*IPaddrlen, &sin6->sin6_port, 2);
		break;
	default:
		memmove(h, &sin6->sin6_addr, IPaddrlen);
		memmove(h+3*IPaddrlen, &sin6->sin6_port, 2);
		break;
	}

	/* alas there's no way to get the local addr/port correctly.  Pretend. */
	memset(sa, 0, sizeof *sa);
	len = sizeof sa;
	getsockname(sock, (void*)sa, &len);
	switch(hdrlen){
	case OUdphdrlenv4:
		memset(h+IPv4addrlen, 0, IPv4addrlen);
		memmove(h+2*IPv4addrlen+2, &sin6->sin6_port, 2);
		break;
	case OUdphdrlen:
		memmove(h+IPaddrlen, &sin6->sin6_addr, IPaddrlen);
		memmove(h+2*IPaddrlen+2, &sin6->sin6_port, 2);
		break;
	default:
		memmove(h+IPaddrlen, &sin6->sin6_addr, IPaddrlen);
		memmove(h+2*IPaddrlen, &sin6->sin6_addr, IPaddrlen);
		memmove(h+3*IPaddrlen+2, &sin6->sin6_port, 2);
		break;
	}
	memmove(hdr, h, hdrlen);
}

int
so_recv(int sock, void *va, int len, void *hdr, int hdrlen)
{
	int r, l;
	struct sockaddr_storage sa;

	osenter();
	if(hdr == 0)
		r = read(sock, va, len);
	else {
		l = sizeof(sa);
		r = recvfrom(sock, va, len, 0, (void*)&sa, &l);
		if(r >= 0) {
			if(sa.ss_family == AF_INET)
				headersv4(sock, hdr, hdrlen, (void*)&sa);
			else
				headersv6(sock, hdr, hdrlen, (void*)&sa);
		}
	}
	osleave();
	return r;
}

void
so_close(int sock)
{
	close(sock);
}

void
so_connect(int fd, unsigned char *raddr, unsigned short rport)
{
	int r;
	struct sockaddr_storage sa;
	struct sockaddr_in *sin;
	struct sockaddr_in6 *sin6;
	socklen_t len;
	int v4;

	v4 = isv4(raddr);
	if(socksetfamily(fd, v4) < 0)
		error(Enotv4);

	if(v4) {
		memset(&sa, 0, sizeof(sa));
		sin = (struct sockaddr_in*)&sa;
		len = sizeof *sin;
		sin->sin_family = AF_INET;
		hnputs(&sin->sin_port, rport);
		memmove(&sin->sin_addr.s_addr, raddr+IPv4off, IPv4addrlen);
	} else {
		memset(&sa, 0, sizeof(sa));
		sin6 = (struct sockaddr_in6*)&sa;
		len = sizeof *sin6;
		sin6->sin6_family = AF_INET6;
		hnputs(&sin6->sin6_port, rport);
		memmove(&sin6->sin6_addr, raddr, IPaddrlen);
	}

	osenter();
	r = connect(fd, (void*)&sa, len);
	osleave();
	if(r < 0)
		oserror();
}

void
so_getsockname(int fd, unsigned char *laddr, unsigned short *lport)
{
	int len;
	struct sockaddr_storage sa;
	struct sockaddr_in *sin;
	struct sockaddr_in6 *sin6;

	len = sizeof(sa);
	if(getsockname(fd, (void*)&sa, &len) < 0)
		oserror();

	if(sa.ss_family == AF_INET) {
		sin = (struct sockaddr_in*)&sa;
		ipw6(laddr, sin->sin_addr.s_addr);
		*lport = nhgets(&sin->sin_port);
	} else if(sa.ss_family == AF_INET6) {
		sin6 = (struct sockaddr_in6*)&sa;
		memmove(laddr, &sin6->sin6_addr, IPaddrlen);
		*lport = nhgets(&sin6->sin6_port);
	} else
		error("bogus socket");
}

void
so_listen(int fd)
{
	int r;

	osenter();
	r = listen(fd, 5);
	osleave();
	if(r < 0)
		oserror();
}

int
so_accept(int fd, unsigned char *raddr, unsigned short *rport)
{
	int nfd, len;
	struct sockaddr_storage sa;
	struct sockaddr_in *sin;
	struct sockaddr_in6 *sin6;

	len = sizeof(sa);
	osenter();
	nfd = accept(fd, (void*)&sa, &len);
	osleave();
	if(nfd < 0)
		oserror();

	if(sockisfamily(nfd, AF_INET)) {
		sin = (struct sockaddr_in*)&sa;
		ipw6(raddr, sin->sin_addr.s_addr);
		*rport = nhgets(&sin->sin_port);
	} else {
		sin6 = (struct sockaddr_in6*)&sa;
		memmove(raddr, &sin6->sin6_addr, IPaddrlen);
		*rport = nhgets(&sin6->sin6_port);
	}
	return nfd;
}

static int
bind0(int fd, unsigned char *addr, unsigned short port, int v4)
{
	struct sockaddr_storage sa;
	struct sockaddr_in *sin;
	struct sockaddr_in6 *sin6;

	if(v4) {
		memset(&sa, 0, sizeof(sa));
		sin = (struct sockaddr_in*)&sa;
		sin->sin_family = AF_INET;
		memmove(&sin->sin_addr.s_addr, addr+IPv4off, IPv4addrlen);
		hnputs(&sin->sin_port, port);
		return bind(fd, (void*)&sa, sizeof *sin);
	} else {
		memset(&sa, 0, sizeof(sa));
		sin6 = (struct sockaddr_in6*)&sa;
		sin6->sin6_family = AF_INET6;
		memmove(&sin6->sin6_addr, addr, IPaddrlen);
		hnputs(&sin6->sin6_port, port);
		return bind(fd, (void*)&sa, sizeof *sin6);
	}
}

void
so_bind(int fd, int su, unsigned char *addr, unsigned short port)
{
	int i, one;
	int v4;

	v4 = isv4(addr) || ipcmp(addr, v6Unspecified) == 0;
	if(socksetfamily(fd, v4) < 0)
		error(Enotv4);

	one = 1;
	if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&one, sizeof(one)) < 0) {
		oserrstr(up->genbuf, sizeof(up->genbuf));
		print("setsockopt: %s", up->genbuf);
	}

	if(su) {
		for(i = 600; i < 1024; i++)
			if(bind0(fd, addr, i, v4) >= 0)
				return;
		oserror();
	}

	if(bind0(fd, addr, port, v4) < 0)
		oserror();
}

void
so_setsockopt(int fd, int opt, int value)
{
	int r;
	struct linger l;

	if(opt == SO_LINGER){
		l.l_onoff = 1;
		l.l_linger = (short) value;
		osenter();
		r = setsockopt(fd, SOL_SOCKET, opt, (char *)&l, sizeof(l));
		osleave();
		if(r < 0)
			oserror();
	}else
		error(Ebadctl);
}

static int
resolve(char *name, char **hostv, int n, int isnumeric)
{
	int i;
	struct addrinfo *res0, *r;
	char buf[5*8];
	uchar addr[IPaddrlen];
	struct addrinfo hints;

	memset(&hints, 0, sizeof hints);
	hints.ai_flags = isnumeric ? AI_NUMERICHOST : 0;
	hints.ai_family = PF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if(getaddrinfo(name, nil, &hints, &res0) < 0)
		return 0;
	i = 0;
	for(r = res0; r != nil && i < n; r = r->ai_next) {
		if(r->ai_family == AF_INET)
			v4tov6(addr, (uchar*)&((struct sockaddr_in*)r->ai_addr)->sin_addr);
		else if(r->ai_family == AF_INET6)
			memmove(addr, &((struct sockaddr_in6*)r->ai_addr)->sin6_addr, IPaddrlen);
		else
			continue;

		snprint(buf, sizeof buf, "%I", addr);
		hostv[i++] = strdup(buf);
	}

	freeaddrinfo(res0);
	return i;
}

int
so_gethostbyname(char *host, char **hostv, int n)
{
	return resolve(host, hostv, n, 0);
}

int
so_gethostbyaddr(char *addr, char **hostv, int n)
{
	return resolve(addr, hostv, n, 1);
}

int
so_getservbyname(char *service, char *net, char *port)
{
	ushort p;
	struct servent *s;

	s = getservbyname(service, net);
	if(s == 0)
		return -1;
	p = s->s_port;
	sprint(port, "%d", nhgets(&p));	
	return 0;
}
int
so_hangup(int fd, int linger)
{
	int r;
	static struct linger l = {1, 1000};

	osenter();
	if(linger)
		setsockopt(fd, SOL_SOCKET, SO_LINGER, (char*)&l, sizeof(l));
	r = shutdown(fd, 2);
	if(r >= 0)
		r = close(fd);
	osleave();
	return r;
}

void
arpadd(char *ipaddr, char *eaddr, int n)
{
#ifdef SIOCGARP
	struct arpreq a;
	struct sockaddr_in pa;
	int s;
	uchar addr[IPaddrlen];

	s = socket(AF_INET, SOCK_DGRAM, 0);
	memset(&a, 0, sizeof(a));
	memset(&pa, 0, sizeof(pa));
	pa.sin_family = AF_INET;
	pa.sin_port = 0;
	parseip(addr, ipaddr);
	if(!isv4(addr)){
		close(s);
		error(Ebadarg);
	}
	memmove(&pa.sin_addr, ipaddr+IPv4off, IPv4addrlen);
	memmove(&a.arp_pa, &pa, sizeof(pa));
	while(ioctl(s, SIOCGARP, &a) != -1) {
		ioctl(s, SIOCDARP, &a);
		memset(&a.arp_ha, 0, sizeof(a.arp_ha));
	}
	a.arp_ha.sa_family = AF_UNSPEC;
	parsemac((uchar*)a.arp_ha.sa_data, eaddr, 6);
	a.arp_flags = ATF_PERM;
	if(ioctl(s, SIOCSARP, &a) == -1) {
		oserrstr(up->env->errstr, ERRMAX);
		close(s);
		error(up->env->errstr);
	}
	close(s);
#else
	error("arp not implemented");
#endif
}

int
so_mustbind(int restricted, int port)
{
	return restricted || port != 0;
}

void
so_keepalive(int fd, int ms)
{
	int on;

	on = 1;
	setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (char*)&on, sizeof(on));
#ifdef TCP_KEEPIDLE
	if(ms <= 120000)
		ms = 120000;
	ms /= 1000;
	setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, (char*)&ms, sizeof(ms));
#endif
}
