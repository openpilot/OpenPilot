/*  Glue functions for the minIni library, based on the FatFs and Tiny-FatFs
 *  libraries, see http://elm-chan.org/fsw/ff/00index_e.html
 *
 *  Copyright (c) ITB CompuPhase, 2008-2009
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *  use this file except in compliance with the License. You may obtain a copy
 *  of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 *  License for the specific language governing permissions and limitations
 *  under the License.
 */

#define INI_BUFFERSIZE  256       /* maximum line length, maximum path length */

/* You must set _USE_STRFUNC to 1 or 2 in the include file ff.h (or tff.h)
 * to enable the "string functions" fgets() and fputs().
 */
#include "ff.h"                   /* include tff.h for Tiny-FatFs */
#define INI_FILETYPE    FIL

#define ini_openread(filename,file)   (f_open((file),(filename),FA_READ+FA_OPEN_EXISTING) == 0)
#define ini_openwrite(filename,file)  (f_open((file),(filename),FA_WRITE+FA_CREATE_ALWAYS) == 0)
#define ini_close(file)               f_close(file)
#define ini_read(buffer,size,file)    fgets((buffer),(size),(file))
#define ini_write(buffer,file)        fputs((buffer),(file))
#define ini_rename(source,dest)       f_rename((source),(dest))
#define ini_remove(filename)          f_unlink(filename)
#define ini_rewind(file)              f_lseek((file),0)
