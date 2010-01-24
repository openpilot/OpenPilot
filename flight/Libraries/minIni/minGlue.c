#include "dosfs.h"
#include "pios.h"
#include "minGlue.h"

/* Global Variables */
extern uint8_t Sector[SECTOR_SIZE];
extern VOLINFO VolInfo;

/* Local Variables */
static uint32_t SuccessCount;

int dosfs_ini_openread(const char *filename, PFILEINFO file)
{
	if(DFS_OpenFile(&VolInfo, (uint8_t *)filename, DFS_READ, Sector, file)) {
		/* Error opening file */
		return 0;
	}

	/* No errors */
	return 1;
}

int dosfs_ini_openwrite(const char *filename, PFILEINFO file)
{
	/* TODO: Check this works */
	if(DFS_OpenFile(&VolInfo, (uint8_t *)filename, DFS_WRITE, Sector, file)) {
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
	DFS_ReadFile(file, Sector, (uint8_t *)buffer, &SuccessCount, size);

	if(SuccessCount == size) {
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
	DFS_WriteFile(file, Sector, (uint8_t *)buffer, &SuccessCount, sizeof(buffer));

	/* No errors */
	return 1;
}

int dosfs_ini_rename(const char *source, const char *dest)
{
	/* TODO: Check this works */
	FILEINFO SourceFile, DestFile;

	/* Disable caching to avoid file inconsistencies while using different sector buffers! */
	DFS_CachingEnabledSet(0);

	if(DFS_OpenFile(&VolInfo, (uint8_t *)source, DFS_READ, Sector, &SourceFile)) {
		/* Source file doesn't exist */
		return 1;
	} else {
		/* Delete destination file if it already exists - ignore errors */
		DFS_UnlinkFile(&VolInfo, (uint8_t *)dest, Sector);

		if(DFS_OpenFile(&VolInfo, (uint8_t *)dest, DFS_WRITE, Sector, &DestFile)) {
			/* Failed to create destination file */
			return 1;
		}
	}

	/* Copy operation */
	uint8_t WriteBuffer[SECTOR_SIZE];
	uint32_t SuccessCountRead;
	uint32_t SuccessCountWrite;
	do {
		if(DFS_ReadFile(&SourceFile, Sector, WriteBuffer, &SuccessCountRead, SECTOR_SIZE)) {
			/* DFS_ReadFile failed */
			return 1;
		} else if(DFS_WriteFile(&DestFile, Sector, WriteBuffer, &SuccessCountWrite, SuccessCountRead)) {
			/* DFS_WriteFile failed */
			return 1;
		}
	} while(SuccessCountRead > 0);

	/* No errors */
	return 1;
}

int dosfs_ini_remove(const char *filename)
{
	/* TODO: Check this works */
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
	/* TODO: Check this works */
	DFS_Seek(file, 0, Sector);
	/* No errors */
	return 1;
}

