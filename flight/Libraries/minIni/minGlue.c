#include "dosfs.h"
#include "pios.h"
#include "minGlue.h"

/* TODO: Use global filesystem mounting */
/* TODO: Implement error handling properly */
static uint8_t Sector[SECTOR_SIZE];
static VOLINFO vi;
static uint32_t pstart, psize;
static uint8_t  pactive, ptype;
static uint32_t successcount;

int dosfs_ini_openread(const char *filename, PFILEINFO file)
{
	pstart = DFS_GetPtnStart(0, Sector, 0, &pactive, &ptype, &psize);
	if (pstart == 0xffffffff) {
		/* Cannot find first partition */
		return 0;
	}

	if(DFS_GetVolInfo(0, Sector, pstart, &vi) != DFS_OK) {
		/* No volume information */
		return 0;
	}

	if(DFS_OpenFile(&vi, (uint8_t *)filename, DFS_READ, Sector, file)) {
		/* Error opening file */
		return 0;
	}

	/* No errors */
	return 1;
}

int dosfs_ini_openwrite(const char *filename, PFILEINFO file)
{
	pstart = DFS_GetPtnStart(0, Sector, 0, &pactive, &ptype, &psize);
	if (pstart == 0xffffffff) {
		/* Cannot find first partition */
		return 0;
	}

	if(DFS_GetVolInfo(0, Sector, pstart, &vi) != DFS_OK) {
		/* No volume information */
		return 0;
	}

	if(DFS_OpenFile(&vi, (uint8_t *)filename, DFS_WRITE, Sector, file)) {
		/* Error opening file */
		return 0;
	}

	/* No errors */
	return 1;
}

int dosfs_ini_close(PFILEINFO file)
{
	/* This doesn't actually do anything */
	DFS_Close(file);

	/* No errors */
	return 1;
}

int dosfs_ini_read(char *buffer, int size, PFILEINFO file)
{
	DFS_ReadFile(file, Sector, (uint8_t *)buffer, &successcount, size);

	if(successcount == size) {
		/* No errors */
		return 1;
	} else {
		/* Reached EOF */
		return 0;
	}
}

int dosfs_ini_write(char *buffer, PFILEINFO file)
{
	/* TODO: Check this works */
	DFS_WriteFile(file, Sector, (uint8_t *)buffer, &successcount, sizeof(buffer));

	/* No errors */
	return 1;
}

int dosfs_ini_rename(const char *source, const char *dest)
{
	/* TODO: Implement, or just don't, since we aren't planning on using renaming */

	/* No errors */
	return 1;
}

int dosfs_ini_remove(const char *filename)
{
	VOLINFO vi;
	uint32_t pstart, psize;
	uint8_t  pactive, ptype;

	pstart = DFS_GetPtnStart(0, Sector, 0, &pactive, &ptype, &psize);
	if (pstart == 0xffffffff) {
		/* Cannot find first partition */
		return 0;
	}

	if(DFS_GetVolInfo(0, Sector, pstart, &vi) != DFS_OK) {
		/* No volume information */
		return 0;
	}

	/* Remove the file */
	DFS_UnlinkFile(&vi, (uint8_t *)filename, Sector);

	/* No errors */
	return 1;
}

int dosfs_ini_rewind(PFILEINFO file)
{
	DFS_Seek(file, 0, Sector);
	/* No errors */
	return 1;
}

