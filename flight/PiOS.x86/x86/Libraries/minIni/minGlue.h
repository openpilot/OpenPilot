/*  Glue functions for the minIni library */

#define INI_LINETERM		"\r\n"
#define NDEBUG
#define INI_BUFFERSIZE		80
// #define INI_BUFFERSIZE	256       /* maximum line length, maximum path length */

#include <stdio.h>
#include <unistd.h>
#define INI_FILETYPE FILE*

#define ini_openread(filename,file)	((*file=fopen((char*)filename,"r"))!=NULL)
#define ini_openwrite(filename,file)	((*file=fopen((char*)filename,"2"))!=NULL)
#define ini_close(file)			fclose(*file)
#define ini_read(buffer,size,file)	fread(buffer,1,size,*file)
#define ini_write(buffer,file)		fprintf(*file,"%s",buffer)
#define ini_rename(source,dest)		rename(source,dest)
#define ini_remove(filename)		unlink(filename)
#define ini_rewind(file)		rewind(*file)

//#include "dosfs.h"
//#define INI_FILETYPE    FILEINFO


//#define ini_openread(filename,file)   dosfs_ini_openread(filename,file)
//#define ini_openwrite(filename,file)  dosfs_ini_openwrite(filename,file)
//#define ini_close(file)               dosfs_ini_close(file)
//#define ini_read(buffer,size,file)    dosfs_ini_read(buffer,size,file)
//#define ini_write(buffer,file)        dosfs_ini_write(buffer,file)
//#define ini_rename(source,dest)       dosfs_ini_rename(source,dest)
//#define ini_remove(filename)          dosfs_ini_remove(filename)
//#define ini_rewind(file)              dosfs_ini_rewind(file)

//extern int dosfs_ini_openread(const char *filename, PFILEINFO file);
//extern int dosfs_ini_openwrite(const char *filename, PFILEINFO file);
//extern int dosfs_ini_close(PFILEINFO file);
//extern int dosfs_ini_read(char *buffer, int size, PFILEINFO file);
//extern int dosfs_ini_write(const char *buffer, PFILEINFO file);
//extern int dosfs_ini_rename(const char *source, const char *dest);
//extern int dosfs_ini_remove(const char *filename);
//extern int dosfs_ini_rewind(PFILEINFO file);

