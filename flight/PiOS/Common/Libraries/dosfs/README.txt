README.TXT                                         (C) Copyright 2006
DOSFS Level 1 Version 1.02      Lewin A.R.W. Edwards (sysadm@zws.com)
=====================================================================

Abstract
========
DOSFS is a FAT-compatible filesystem intended for fairly low-end
embedded applications. It is not the leanest possible implementation
(the leanest FAT implementations operate in << 512 bytes of RAM, with
heavy restrictions). This code strikes a good balance between size
and functionality, with an emphasis on RAM footprint.

Intended target systems would be in the ballpark of 1K RAM, 4K ROM
or more.

Features:
* Supports FAT12, FAT16 and FAT32 volumes
* Supports storage devices up to 2048Gbytes in size (LBA32)
* Supports devices with or without MBRs (hard disks vs. floppy disks
  or ZIP drives formatted as "big floppies")
* Supports multiple partitions on disks with MBRs
* Supports subdirectories
* Can be operated with a single global 512-byte sector buffer
* Fully reentrant code (assuming the underlying physical device driver
  is reentrant and global sector buffers are not used). There are no
  global variables in the filesystem
* Does not perform any memory allocation
* Partial support for random-access files

Applications:
* Firmware upgrades
* Failsafe IPL
* Media playback
* Data logging
* Configuration storage

There is no technical support for this free product; however, if you
have questions or suggestions, you are encouraged to email Lewin
Edwards at sysadm@zws.com. If you need custom additions to the code,
or if you have other projects for which you need engineering
assistance, please feel free to email or call (646) 549-3715.

License
=======
The license for DOSFS is very simple but verbose to state.

1. DOSFS is (C) Copyright 2006 by Lewin A.R.W. Edwards ("Author").
   All rights not explicitly granted herein are reserved. The DOSFS
   code is the permanent property of the Author and no transfer of
   ownership is implied by this license.

2. DOSFS is an educational project, provided as-is. No guarantee of
   performance or suitability for any application is stated or
   implied. You use this product entirely at your own risk. Use of
   this product in any manner automatically waives any right to seek
   compensation or damages of any sort from the Author. Since the
   products you might make are entirely out of the Author's control,
   use of this product also constitutes an agreement by you to take
   full responsibility for and indemnify the Author against any
   action for any loss or damage (including economic loss of any
   type, and specifically including patent litigation) that arises
   from a product made by you that incorporates any portion of
   the DOSFS code.

3. If you live under the jurisdiction of any legislation that would
   prohibit or limit any condition in this license, you cannot be
   licensed to use this product.

4. If you do not fall into the excluded category in point 3, you are
   hereby licensed to use the DOSFS code in any application that you
   see fit. You are not required to pay any fee or notify the Author
   that you are using DOSFS. Any modifications made by you to the
   DOSFS code are your property and you may distribute the modified
   version in any manner that you wish. You are not required to
   disclose sourcecode to such modifications, either to the Author or
   to any third party. Any such disclosure made to the Author will
   irrevocably become the property of the Author in the absence of a
   formal agreement to the contrary, established prior to such
   disclosure being made.

To summarize the intent of the above: DOSFS is free. You can do what
you want with it. Anything that happens as a result is entirely your
responsibility. You can't take ownership of my code and stop me from
doing whatever I want with it. If you do something nifty with DOSFS
and send me the sourcecode, I may include your changes in the next
distribution and it will be released to the world as free software.
If someone sues you because your DOSFS-containing product causes
any sort of legal, financial or other problem, it's your lawsuit,
not mine, and you'll exclude me from the proceedings.

User-Supplied Functions
=======================
You must provide functions to read sectors into memory and write
them back to the target media. The demo suite includes an emulation
module that reads/writes a disk image file (#define HOSTVER pulls
in hostemu.h which wraps the prototypes for these functions).
There are various tools for UNIX, DOS, Windows et al, to create
images from storage media; my preferred utility is dd.

The functions you must supply in your embedded app are:

DFS_ReadSector(unit,buffer,sector,count)
DFS_WriteSector(unit,buffer,sector,count)

These two functions read and write, respectively, "count" sectors of
size SECTOR_SIZE (512 bytes; see below) from/to physical sector
#"sector" of device "unit", to/from the scratch buffer "buffer". They
should return 0 for success or nonzero for failure. In the current
implementation of DOSFS, count will always be 1.

The "unit" argument is designed to permit implementation of multiple
storage devices, for example multiple media slots on a single device,
or to differentiate between master and slave devices on an ATAPI bus.
                 
This code is designed for 512-byte sectors. Although the sector size
is a #define, you should not tinker with it because the vast majority
of FAT filesystems use 512-byte sectors, and the DOSFS code doesn't
support runtime determination of sector size. This will not affect the
vast majority of users.

Example Code
============
Refer to the tests in main.c to see how to call DOSFS functions.
(These tests are all commented out). Note that the only two files
you need to add to your project are dosfs.c and dosfs.h.


Mounting Volumes
================
--If the device has a partition table (practically all removable flash
  media are formatted this way), call DFS_GetPtnStart to get the
  starting sector# of the desired partition. You can optionally also
  retrieve the active state, partition type byte and partition size
  in this step. The reason this step is broken out separately is so
  you can support devices that are formatted like a floppy disk, i.e.
  the volume starts directly at physical sector 0 of the media.

--Call DFS_GetVolInfo to read filesystem info into a VOLINFO structure.
  DFS_GetVolInfo needs to know the unit number and partition starting
  sector (as returned by DFS_GetPtnStart, or 0 if this is a "floppy-
  format" volume without an MBR).

From this point on, the VOLINFO structure is all you'll need - you can
forget the unit and partition start sector numbers.

Enumerating Directory Contents
==============================
--Call DFS_Opendir and supply a path, populated VOLINFO and a
  DIRINFO structure to receive the results. Note - you must PREPOPULATE
  the DIRINFO.scratch field with a pointer to a sector scratch buffer.
  This buffer must remain unmolested while you have the directory open
  for searching.
--Call DFS_GetNext to receive the DIRENT contents for the next directory
  item. This function returns DFS_OK for no error, and DFS_EOF if there
  are no more entries in the directory being searched.
  Before using the DIRENT, check the first character of the name. If it
  is NULL, then this is an unusable entry - call DFS_GetNext again to
  keep searching. LFN directory entries are automatically tagged this way
  so your application will not be pestered by them.

  Note: A designed side-effect of this code is that when you locate the
  file of interest, the DIRINFO.currentcluster, DIRINFO.currentsector
  and DIRINFO.currententry-1 fields will identify the directory entry of
  interest.

Reading a File
==============
--Call DFS_OpenFile with mode = DFS_READ and supply a path and the relevant
  VOLINFO structure. DFS_OpenFile will populate a FILEINFO that can be used
  to refer to the file.
--Optionally call DFS_Seek to set the file pointer. If you attempt to set
  the file pointer past the end of file, the file will NOT be extended. Check
  the FILEINFO.pointer value after DFS_Seek to verify that the pointer is
  where you expect it to be.
--Observe that functionality similar to the "whence" parameter of fseek() can
  be obtained by using simple arithmetic on the FILEINFO.pointer and
  FILEINFO.filelen members.
--Call DFS_ReadFile with the FILEINFO you obtained from OpenFile, and a
  pointer to a buffer plus the desired number of bytes to read, and a
  pointer to a sector-sized scratch buffer. The reason a scratch sector is
  required is because the underlying sector read function doesn't know
  about partial reads.
--Note that a file opened for reading cannot be written. If you need r/w
  access, open with mode = DFS_WRITE (see below).

Writing a file
==============
--Call DFS_OpenFile with mode = DFS_WRITE and supply a path and the relevant
  VOLINFO structure. DFS_OpenFile will populate a FILEINFO that can be used to
  refer to the file.
--Optionally call DFS_Seek to set the file pointer. Refer to the notes on
  this topic in the section on reading files, above.
--Call DFS_WriteFile with the FILEINFO you obtained from OpenFile, and a
  pointer to the source buffer, and a pointer to a sector-sized scratch
  buffer.
--Note that a file open for writing can also be read.
--Files are created automatically if they do not exist. Subdirectories are
  NOT automatically created.
--If you open an existing file for writing, the file pointer will start at
  the beginning of the data; if you want to append, seek to the end before
  writing new data.
--If you perform random-access writes to a file, the length will NOT change
  unless you exceed the file's original length. There is currently no
  function to truncate a file at the current pointer position.
--On-disk consistency is guaranteed when DFS_WriteFile exits, unless your
  physical layer has a writeback cache in it.

Deleting a file
===============
--Call DFS_UnlinkFile
--WARNING: This call will delete a subdirectory (correctly) but will NOT
  first recurse the directory to delete the contents - so you will end up
  with lost clusters.

Notes
=====
Some platforms may require explicit pragmas or attributes to the structures
and unions. For example, arm-gcc will require __attribute__ ((__packed__))
otherwise it will try to be "smart" and place the uint8_t members on 4-byte
boundaries. There is no truly elegant compiler-independent method to get
around this sort of problem.

The code assumes either a von Neumann architecture, or a compiler that
is smart enough to understand where your pointers are aimed and emit
the right kind of memory read and write instructions. The implications
of this statement depend on your target processor and the compiler you
are using. Be very careful not to straddle bank boundaries on bank-
switched memory systems.

Physical 32-bit sector numbers are used throughout. Therefore, the
CHS geometry (if any) of the storage media is not known to DOSFS. Your
sector r/w functions may need to query the CHS geometry and perform
mapping.

File timestamps set by DOSFS are always 1:01:00am on Jan 1, 2006. If
your system has a concept of real time, you can enhance this.

FILEINFO structures contain a pointer to the corresponding VOLINFO
used to open the file, mainly in order to avoid mixups but also to
obviate the need for an extra parameter to every file read/write. DOSFS
assumes that the VOLINFO won't move around. If you need to move or
destroy VOLINFOs pertaining to open files, you'll have to fix up the
pointer in the FILEINFO structure yourself.

The subdirectory delimiter is a forward slash ( '/' ) by default. The
reason for this is to avoid the common programming error of forgetting
that backslash is an escape character in C strings; i.e. "\MYDIR\FILE"
is NOT what you want; "\\MYDIR\\FILE" is what you wanted to type. If you
are porting DOS code into an embedded environment, feel free to change
this #define.

DOSFS does not have a concept of "current directory". A current directory
is owned by a process, and a process is an operating system concept.
DOSFS is a filesystem library, not an operating system. Therefore, any
path you provide to a DOSFS call is assumed to be relative to the root of
the volume.

There is no call to close a file or directory that is open for reading or
writing. You can simply destroy or reuse the data structures allocated for
that operation; there is no internal state in DOSFS so no cleanup is
necessary. Similarly, there is no call to close a file that is open for
writing. (Observe that dosfs.c has no global variables. All state information
is stored in data structures provided by the caller).

MAX_PATH is defined as 64. MS-type DOS filesystems support 128 characters
or more in paths. You can increase this define, but it may GREATLY
increase memory requirements.

VFAT long filenames are not supported. There is a certain amount of
patent controversy about them, but more importantly they don't really
belong in the scope of a "minimalist embedded filesystem".

Improving Performance
=====================
Read performance is fairly good, but can be improved by implementing read
caching on the FAT (see below) and, depending on your hardware platform,
possibly by implementing multi-sector reads.

Write performance may benefit ENORMOUSLY from platform-specific
optimization, especially if you are working with a flash media type that
has a large erase block size. While it is not possible to offer detailed
platform-independent advice, my general advice is to implement writeback
caching on the FAT area. One method for doing this would be to have a
cache system that lives in the DFS_ReadSector/WriteSector functions (on
top of the physical sector r/w functions) and is initially switched off.
Once you have called DFS_GetVolInfo, you then extract the VOLINFO.fat1
and VOLINFO.rootdir parameters and pass them to your caching layer.
Sectors >= fat1 and < rootdir should be cached. The cache strategy is
determined by the physical storage medium underlying the filesystem.

CACHING HINT:
Observe that there will be numerous read-modify-write operations in the
region from VOLINFO.fat1 through VOLINFO.fat1+VOLINFO.secperfat-1, but
in the region from VOLINFO.fat1+VOLINFO.secperfat through VOLINFO.rootdir
there will ONLY be write operations.

Platform Compatibility
======================
DOSFS was derived from code originally written for ARM7TDMI but
designed to be portable. It has been tested on AVR (using avrgcc),
MSP430 (using Rowley's CrossWorks) and PPC603e (using gcc); the host
test suite has also been validated on x86 using gcc under both Cygwin
and 32-bit Fedora Core 4 Linux.

TODO list
=========
* Add function to create subdirectory
* Make DFS_UnlinkFile recognize non-empty subdirectories
* Support "fast write" files where the FAT is not updated, for
  logging applications where latency is important.

Test cases for V1.02
====================
Version 1.02 has NOT been through full regression testing. However the
bugs fixed in this version are important, and people have been asking
about them.

Test cases for V1.01
====================
See below.

Test cases for V1.00
====================
These are the test cases that were used to validate the correct
functionality of the DOSFS suite. Each test was performed on FAT12,
FAT16 and FAT32 volumes. P=Pass, F=Fail.

Case                                                      F12 F16 F32
---------------------------------------------------------------------
Get volume information                                    P   P   P
Open root directory                                       P   P   P
List contents of root directory (fully populated)         P   P   P
Open subdirectory                                         P   P   P
List contents of subdirectory (<= 1 cluster)              P   P   P
List contents of large subdirectory (> 1 cluster)         P   P   P
Open 5-level nested subdirectory                          P   P   P
Open existing file for reading                            P   P   P
Open nonexistent file for reading                         P   P   P
Seek past EOF, file open for reading                      P   P   P
Seek to cluster boundary                                  P   P   P
Seek past cluster boundary                                P   P   P
Seek backwards to nonzero offset, pointer > cluster size  P   P   P
Block-read entire file >1 cluster in size, odd size       P   P   P
Seek to odd location in file                              P   P   P
Perform <1 sector reads from random file locations        P   P   P
Open nonexistent file for writing in root dir             P   P   P
Open nonexistent file for writing in subdir               P   P   P
Repeat prev. 2 tests on volume with 0 free clusters       P   P   P
Seek past EOF, file open for writing                      P   P   P
Open existing file for writing in root dir                P   P   P
Write random-length records to file, 20 clusters total    P   P   P
MS-DOS 6.0 SCANDISK cross-check                           P   P   P

Revision History
================
Jan-06-2005 larwe Initial release (1.0)
Jan-29-2006 larwe Bugfix release (1.01)
 - Fixed error in FAT12 FAT read on boundary of sector
 - Improved compilability under avrgcc
Sep-16-2006 larwe Bugfix release (1.02)
 - DFS_Seek would not correctly rewind to start of file
 - DFS_Seek would not correctly seek to a position not on a cluster
   boundary
 - DFS_OpenFile fencepost error caused memory access at [start of
   string-1] with a local variable
 - DFS_OpenFile could not open a file in the root directory
 
 