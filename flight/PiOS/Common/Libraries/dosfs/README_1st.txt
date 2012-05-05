DOSFS has been developed by Lewin Edwards, and is provided as freeware.

See README.txt for details.

This package has been downloaded from:
   http://www.larwe.com/zws/products/dosfs/

Version 1.03 from 9/30/06 is used.


dfs_sdcard has been added as access layer between DFS functions and PIOS_SDCARD functions

The original usage examples can be found under unused/main.c

Addendum:

TK 2008-12-18: 
DFS_Seek was running endless, applied a patch which has been posted at
http://reza.net/wordpress/?p=110

TK 2008-12-18: 
patched the patch: endcluster wasn't calculated correctly

TK 2008-12-18: 
added 'DFS_CachingEnabledSet(uint8_t enable)' function to enable a simple 
caching mechanism. This feature has to be explicitely enabled, as it isn't 
reentrant and requires to use the same buffer pointer whenever reading a file!

TK 2008-18-12
added missing pendant to DFS_CanonicalToDir;
char *DFS_DirToCanonical(char *dest, char *src)
expects a 13 byte buffer in *dest

TK 2009-02-12
added dummy "DFS_Close" function
It has no effect if writing to SD Card, it's only used by the DosFS wrapper 
in emulation

TK 2009-07-04
fixed bug in DFS_GetNext() in conjunction with the DFS_GetFreeDirEnt() function
New files where not added correctly to subdirectories

