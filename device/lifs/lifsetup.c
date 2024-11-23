/* lifsetup.c - lifsetup */

#include <xinu.h>

/*
	unified munipulate for (direct / indirect) index block and data block
*/
void Unified_munipulate(dbid32 *dnumptr, char *data_blk_ptr, dbid32 *dnumptr_real, bool8 *dirty_bit_ptr1, bool8 *dirty_bit_ptr2){
	if (*dnumptr_real == LF_DNULL){
		*dnumptr_real = lifindballoc((struct lifdbfree *)data_blk_ptr);
		*dirty_bit_ptr1 = TRUE;
		*dirty_bit_ptr2 = FALSE; // since just allocate and initialize a brand new block
	}
	else if (*dnumptr_real != *dnumptr){
		read(Lif_data.lif_dskdev, data_blk_ptr, *dnumptr_real);
		// all the read and write on "Lif_data.lif_dskdev" is "ramread/ramwrite".
		*dirty_bit_ptr2 = FALSE;
	}
	*dnumptr = *dnumptr_real;
}

/*------------------------------------------------------------------------
 * lifsetup  -  Set a file's index block and data block for the current
 *		 file position (assumes file mutex held)
 *------------------------------------------------------------------------
 */

status	lifsetup (
	  struct liflcblk  *lifptr	/* Pointer to slave file device	*/
	)
{
	ibid32	ibnum;			/* I-block number during search	*/
	struct	ldentry	*ldptr;		/* Ptr to file entry in dir.	*/
	struct	lifiblk	*ibptr;		/* Ptr to in-memory index block	*/
	int32	index_;
	dbid32	*numptr;		/* where to store the newly allocated block number */
	uint32  delta_;
	bool8   *dirty_bit_ptr_for_numptr_origin;

	/* Obtain exclusive access to the directory */

	wait(Lif_data.lif_mutex);

	/* Get pointers to in-memory directory, file's entry in the	*/
	/*	directory, and the in-memory index block		*/

	ldptr = lifptr->lifdirptr; // this is for directory information
	ibptr = &lifptr->lifiblock; // this is i-block information

	/* If existing index block or data block changed, write to disk	*/

	if (lifptr->lifibdirty || lifptr->lifdbdirty || lifptr->lifindbdirty || lifptr->lif2indbdirty || lifptr->lif3indbdirty) {
		lifflush(lifptr);
	}
	ibnum = lifptr->lifinum;		/* Get ID of curr. index block	*/

	/* If there is no index block in memory (e.g., because the file	*/
	/*	was just opened), either load the index block of	*/
	/*	the file or allocate a new index block		*/

	if (ibnum == LF_INULL) {

		/* Check directory entry to see if index block exists	*/

		ibnum = ldptr->ld_ilist;
		if (ibnum == LF_INULL) { /* Empty file - get new i-block*/
			ibnum = lifiballoc();
			lifibclear(ibptr, 0);
			ldptr->ld_ilist = ibnum; // ldptr is always in memory, so no need to write back
			// dirty because the i-block number is updated
			lifptr->lifibdirty = TRUE;
		} else {		/* Nonempty - read first and only i-block*/
	 		lifibget(Lif_data.lif_dskdev, ibnum, ibptr);
		}
		lifptr->lifinum = ibnum;
	}

	/* At this point, an index block is in memory (pointed to by ibptr), 
	 * but the file's position might point to data reachable 
	 * only through indirect blocks */
	if (lifptr->lifpos >= LIF_AREA_DIRECT) {
		if (lifptr->lifpos < LIF_AREA_DIRECT + LIF_AREA_INDIR){
			// 1INDIR
			delta_ = lifptr->lifpos - LIF_AREA_DIRECT;

			Unified_munipulate(
				&(lifptr->lifindnum),
				lifptr->lifindblock,
				&(ibptr->ind),
				&(lifptr->lifibdirty),
				&(lifptr->lifindbdirty)
			);
			
			index_ = delta_ >> 9;
			numptr = &lifptr->lifindblock[index_];
		}
		else if (lifptr->lifpos < LIF_AREA_DIRECT + LIF_AREA_INDIR + LIF_AREA_2INDIR){
			// 2INDIR
			delta_ = lifptr->lifpos - LIF_AREA_DIRECT - LIF_AREA_INDIR;

			Unified_munipulate(
				&(lifptr->lif2indnum),
				lifptr->lif2indblock,
				&(ibptr->ind2),
				&(lifptr->lifibdirty),
				&(lifptr->lif2indbdirty)
			);

			index_ = delta_ >> (9 + 7);
			numptr = &lifptr->lif2indblock[index_];

			Unified_munipulate(
				&(lifptr->lifindnum),
				lifptr->lifindblock,
				numptr,
				&(lifptr->lif2indbdirty),
				&(lifptr->lifindbdirty)
			);

			index_ = (delta_ >> 9) & LIF_INDMASK;
			numptr = &lifptr->lifindblock[index_];
		}
		else if (lifptr->lifpos < LIF_AREA_DIRECT + LIF_AREA_INDIR + LIF_AREA_2INDIR + LIF_AREA_3INDIR){
			// 3INDIR
			delta_ = lifptr->lifpos - LIF_AREA_DIRECT - LIF_AREA_INDIR - LIF_AREA_2INDIR;

			Unified_munipulate(
				&(lifptr->lif3indnum),
				lifptr->lif3indblock,
				&(ibptr->ind3),
				&(lifptr->lifibdirty),
				&(lifptr->lif3indbdirty)
			);

			index_ = delta_ >> (9 + 7 + 7);
			numptr = &lifptr->lif3indblock[index_];

			Unified_munipulate(
				&(lifptr->lif2indnum),
				lifptr->lif2indblock,
				numptr,
				&(lifptr->lif3indbdirty),
				&(lifptr->lif2indbdirty)
			);

			index_ = (delta_ >> (9 + 7)) & LIF_INDMASK;
			numptr = &lifptr->lif2indblock[index_];

			Unified_munipulate(
				&(lifptr->lifindnum),
				lifptr->lifindblock,
				numptr,
				&(lifptr->lif2indbdirty),
				&(lifptr->lifindbdirty)
			);

			index_ = (delta_ >> 9) & LIF_INDMASK;
			numptr = &lifptr->lifindblock[index_];
		}
		else{
			kprintf("not support such a large file\n");
			exit();
		}
		// but no matter in which level indirect region, we will finally get "numptr" from "lifptr->lifindblock"
		dirty_bit_ptr_for_numptr_origin = &(lifptr->lifindbdirty);
	} else {
		index_ = (lifptr->lifpos & LF_IMASK) >> 9;
		numptr = &lifptr->lifiblock.ib_dba[index_];
		dirty_bit_ptr_for_numptr_origin = &(lifptr->lifibdirty);
	}

	/* dnum is set to the correct data block that covers the current file position */

	/* At this point, dnum is either LF_DNULL (meaning the position is beyond the file content)
	 *  or covers the current file position (i.e., position lifptr->lifpos).  
	 *  The	next step consists of loading the correct data block.	*/

	Unified_munipulate(
		&(lifptr->lifdnum),
		lifptr->lifdblock,
		numptr,
		dirty_bit_ptr_for_numptr_origin,
		&(lifptr->lifdbdirty)
	);

	/* Use current file offset to set the pointer to the next byte	*/
	/*   within the data block					*/

	lifptr->lifbyte = &lifptr->lifdblock[lifptr->lifpos & LIF_DMASK];
	signal(Lif_data.lif_mutex);
	return OK;
}
