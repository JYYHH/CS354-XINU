/* lifsinit.c - lifsinit */

#include <xinu.h>

struct	lifdata	Lif_data;
	// this is the memory cache version, for the first block in device (the directory block)

/*------------------------------------------------------------------------
 * lifsinit  -  Initialize the local file system master device
 * called only when rebooting the file system
 *------------------------------------------------------------------------
 */
devcall	lifsinit (
	  struct dentry *devptr		/* Entry in device switch table */
	)
{
	/* Assign ID of disk device that will be used */

	Lif_data.lif_dskdev = LF_DISK_DEV;

	/* Create a mutual exclusion semaphore */

	Lif_data.lif_mutex = semcreate(1);

	/* Zero directory area (for debugging) */

	memset((char *)&Lif_data.lif_dir, NULLCH, sizeof(struct lifdir));

	/* Initialize directory to "not present" in memory */

	Lif_data.lif_dirpresent = Lif_data.lif_dirdirty = FALSE;

	return OK;
}
