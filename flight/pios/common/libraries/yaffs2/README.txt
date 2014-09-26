yaffs2 library in flight/pios/common/libraries/yaffs2

Implementation of core Yaffs Direct:
	yaffs_allocator.c 	Allocates Yaffs object and tnode structures.
	yaffs_checkpointrw.c 	Streamer for writing checkpoint data
	yaffs_ecc.c 		ECC code
	yaffs_guts.c 		The major Yaffs algorithms.
	yaffs_nand.c		Flash interfacing abstraction.
	yaffs_packedtags1.c 	Tags packing code
	yaffs_packedtags2.c
	yaffs_qsort.c		Qsort used during Yaffs2 scanning
	yaffs_tagscompat.c 	Tags compatibility code to support Yaffs1 mode.
	yaffs_tagsvalidity.c 	Tags validity checking.
	yaffsfs.c		The Yaffs direct interface 
	yaffs_hweight.c		Linux hweight implementation equivalent (Is this in OP TODO)
	yaffs_list.c		Linked list implementation
	yaffs_tarsmarshall.c
Interface between Yaffs and OP PiOS or Posix:
	ydirectenv.h 		Environment wrappers for Yaffs direct to suit the OP firmware environment
	yaffs_osglue.h		Interface for Yaffs to use to access OS method
Interface to Yaffs Direct:
	yaffsfs.h		and interface structures and functions defined here

POSIX Implementation in flight/pios/common/libraries/posix
PiOS Implementation in flight/pios/common/libraries/PiOS

CHANGE LOG:
1. Initial import
2. ydirectenv.h		Defined Y_LOFF_T to be int32_t.  Needs to be signed to return -1 for lseek invalid
3. library.mk		Compiler defines to set yaffs2 diect mode and port options
4. simposix		Added simposix implementation of lower layers
5. arm compilation	Addressed compile errors in arm toolchain. Generally marked changes with CONFIG_YAFFS_OP
