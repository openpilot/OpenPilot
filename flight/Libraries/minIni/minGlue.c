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
	if(PIOS_SDCARD_ReadLine(file, (uint8_t *)buffer, size) < 0)
	{
		/* Error reading line */
		return 0;
	}

	/* No errors */
	return 1;
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
	if(PIOS_SDCARD_FileCopy((char *)source, (char *)dest)) {
		/* Error renaming file */
		return 0;
	}

	/* No errors */
	return 1;
}

int dosfs_ini_remove(const char *filename)
{
	/* Remove the file */
	if(PIOS_SDCARD_FileDelete((char *)filename)) {
		/* Error deleting file */
		return 0;
	}

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

