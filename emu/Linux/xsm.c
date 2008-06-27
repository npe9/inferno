#include "dat.h"
#include "fns.h"
#include "error.h"
#include "ip.h"
#include "shm.h"

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <xs.h>
#include <xenctrl.h>
#include <sys/mman.h>
#include <string.h>

int debuglevel = 0;

/*
 * Get a domid of the specified name
 * Referred by files in Xen tools/console/daemon/
 */

uint32_t 
xen_get_domid(char *dom_name)
{
	struct xs_handle *xs = NULL;
	int xc = -1;
	uint32_t domid = 0;
	uint32_t first_domid;
	xc_dominfo_t dominfo;

	/* Open XenBus/Xencontrol */
	xs = xs_daemon_open();
	if (xs == NULL) {
		perror("xs_daemon_open");
		goto out;
	}
	xc = xc_interface_open();
	if (xc == -1) {
		perror("xc_interface_open");
		goto out;
	}

	/* check all guest domains */
	for (first_domid = 1;
	  xc_domain_getinfo(xc, first_domid, 1, &dominfo) == 1;
	  first_domid = dominfo.domid + 1) {
		char *dompath;
		char path[100];
		char *data;
		unsigned int len;

		dompath = xs_get_domain_path(xs, dominfo.domid);

		/* check name */
		sprintf(path, "%s/%s", dompath, "name");
		len = strlen(path);
		data = xs_read(xs, XBT_NULL, path, &len);
		if (data == NULL)
			continue;
		if (strcmp(data, dom_name) != 0) {
			free(data);
			continue;
		}

		/* found the domain, exit */
		domid = dominfo.domid;
		free(data);
		goto out;
	}

	if (debuglevel)
		fprintf(stderr, "xen_get_domid: domain \"%s\" not found\n",
			dom_name);

  out:
	if (xs != NULL)
		xs_daemon_close(xs);
	if (xc != -1)
		xc_interface_close(xc);
    	return domid;
}

/*
 * Map the specified paddr ranges
 * Referred by files in XenPPC tools/daemon/
 */

void *
xen_map_pfn(uint32_t domid, unsigned long pfn, unsigned long num_pfn)
{
	int xc = -1;
	xen_pfn_t *page_array = NULL;
	int npages, i;
	char *mapped_addr = NULL;
	xen_pfn_t *p;

	if (debuglevel >= 1)
		printf("xen_map_pfn: domid=%d pfn=%ld num_pfn=%ld\n", domid, 
			pfn, num_pfn);

	/* Open XenBus/Xencontrol */
	xc = xc_interface_open();
	if (xc == -1) {
		perror("xc_interface_open");
		goto out;
	}

	fprintf(stderr, "mapping  pfn, %d num pfn %d\n", pfn, num_pfn);
	/* get page frame list for just the first 64 Meg chunk */
	if (((pfn << 12) + (num_pfn << 12)) > (64 << 20)) {
		fprintf(stderr, 
			"OOPS, address is too large pfn, %d num pfn %d\n",
			pfn, num_pfn);
		exit(-1);
	}

	num_pfn = ((64 << 20) >> 12);
	page_array = malloc(num_pfn * sizeof(xen_pfn_t));
	if (page_array == NULL) {
		fprintf(stderr, "xen_map_pfn: malloc failed\n");
		goto out;
	}

	/* We know that the RMA is machine contiguous so lets just get the
	 * first MFN and fill the rest in ourselves 
	 */
	i = xc_get_pfn_list(xc, domid, page_array, 1);
	if (i != 1) {
		fprintf(stderr,
			"xen_map_pfn: Could not get the page frame list\n");
		goto out;
	}

	p = page_array;
	for (i = 1; i < num_pfn; i++)
		p[i] = p[i - 1] + 1;

	mapped_addr = xc_map_foreign_batch(xc, domid, 
						PROT_READ | PROT_WRITE,
						&page_array[pfn], num_pfn);
	if (mapped_addr == NULL) {
		perror("xc_map_foreign_batch");
		goto out;
	}

	if (debuglevel >= 1)
		printf("xen_map_pfn: mapped at %p\n", mapped_addr);
	
  out:
	if (page_array != NULL)
		free(page_array);
	if (xc != -1)
		xc_interface_close(xc);
	return mapped_addr;
}


/*
 * Map the specified paddr ranges
 * Referred by files in XenPPC tools/daemon/
 */
void *xen_map_128Meg(uint32_t domid)
{
	int xc = -1;
	xen_pfn_t *page_array = NULL;
	int i;
	char *mapped_addr = NULL;
	int num_pfn = (128 << 20) >> 12;

	/* Open XenBus/Xencontrol */
	xc = xc_interface_open();
	if (xc == -1) {
		perror("xc_interface_open");
		goto out;
	}

	if (debuglevel)
		fprintf(stderr, "---- num_pfn %d, tot_pag %d\n",
			num_pfn, xc_get_tot_pages(xc, domid));

	if (num_pfn > xc_get_tot_pages(xc, domid)) 
		num_pfn = xc_get_tot_pages(xc, domid);
    
	if (debuglevel)
		fprintf(stderr, "xen_map_128Meg, mapping %d meg\n",
				(num_pfn << 12) >> 20);

	/* get page frame list for just the first 128 Meg chunk */
	page_array = malloc(num_pfn * sizeof(xen_pfn_t));
	if (page_array == NULL) {
		fprintf(stderr, "xen_map_pfn: malloc failed\n");
		goto out;
	}

	/* We know that the RMA is machine contiguous so lets just get the
	 * first MFN and fill the rest in ourselves 
	 */
	i = xc_get_pfn_list(xc, domid, page_array, num_pfn);
	if (i != num_pfn) {
		fprintf(stderr,
		  "xen_map_pfn: Could not get the page frame list\n");
		goto out;
	}

	mapped_addr = xc_map_foreign_batch(xc, domid, 
					PROT_READ | PROT_WRITE,
					&page_array[0], num_pfn);
	if (mapped_addr == NULL) {
		perror("xc_map_foreign_batch");
		goto out;
	}
	if (debuglevel >= 1)
		printf("xen_map_pfn: mapped at %p\n", mapped_addr);

  out:
	if (page_array != NULL)
		free(page_array);
	if (xc != -1)
		xc_interface_close(xc);
	return mapped_addr;
}

static int
xsmconnect(Conv *c)
{
	struct chan_pipe *chan;
	char *domain = c->laddr;
	int offset = c->lport;
	unsigned long domid;

	if(c->state != Announcing) {
		fprint(2, "xsmconnect: only supports server, not client\n");
		return -1;
	}

	/* TODO: Adjust to reduce startup latency */
	while ((domid = xen_get_domid(domain)) == 0) 
		sleep(1);

	/* FIXME: we can, for now, only map the first 128 meg */
	c->raw = (char *) xen_map_128Meg(domid);
	if(c->raw < 0) {
		perror("xsmconnect: shmat: ");
		error("Could not attach to shared memory segment");
		print("Could not attach to shared memory segment");
		return -1;
	}

	c->chan = (void *) (c->raw+offset);
	chan = (struct chan_pipe *) c->chan;
	if(c->state == Announcing) {	
		chan->magic = CHAN_MAGIC;
		chan->out.magic = CHAN_BUF_MAGIC;
		chan->out.write = 0;
		chan->out.read = 0;
		chan->out.overflow = 0;
		chan->in.magic = CHAN_BUF_MAGIC;
		chan->in.write = 0;
		chan->in.read = 0;
		chan->in.overflow = 0;
		chan->state = Announced;
		c->mode = SM_SERVER;
	} 

	return 0;
}

static int
xsmclose(Conv *c)
{
	c->state = Hungup;

	return 0;	
}

struct Shmops xsmop = {
	xsmconnect,
	xsmconnect,
	shmlisten,
	shmread,
	shmwrite,
	xsmclose,
	shmdebug
};

void
xsmlink(void)
{
	devshminit();
	devshmnewproto("xsm", S_XEN, 8, &xsmop);
}
