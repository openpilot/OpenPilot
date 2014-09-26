/**
 ******************************************************************************
 *
 * @file       pios_stdio.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @addtogroup PiOS
 * @{
 * @addtogroup PiOS
 * @{
 * @brief PiOS stdio posix file functions
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#ifndef PIOS_STDIO_H
#define PIOS_STDIO_H

#include "yaffsfs.h"

#define pios_open(path, oflag, mode)	yaffs_open(path, oflag, mode)
#define pios_close(fd)			yaffs_close(fd)
#define pios_fsync(fd)			yaffs_fsync(fd)
#define pios_flush(fd)			yaffs_flush(fd)
#define piod_read(fd, buf, nbyte)	yaffs_read(fd, buf, nbyte)
#define piod_write(fd, buf, nbyte)	yaffs_write(fd, buf, nbyte)
#define pios_fdatasync(fd)		yaffs_fdatasync(fd)
#define pios_access(path, amode) 	yaffs_access(path, amode)
#define pios_dup(fd) 			yaffs_dup(fd)
#define pios_pread(fd, buf, nbyte, offset) yaffs_pread(fd, buf, nbyte, offset)
#define pios_pwrite(fd, buf, nbyte, offset) yaffs_pwrite(fd, buf, nbyte, offset)
#define pios_lseek(fd, offset, whence)  yaffs_lseek(fd, offset, whence)
#define pios_truncate(path, new_size)	yaffs_truncate(path, new_size)
#define pios_ftruncate(fd, new_size) 	yaffs_ftruncate(fd, new_size)
#define pios_unlink(path)		yaffs_unlink(path)
#define pios_rename(oldPath, newPath) 	yaffs_rename(oldPath, newPath) 
#define pios_stat(path, buf) 		yaffs_stat(path, buf) 
#define pios_lstat(path, buf) 		yaffs_lstat(path, buf) 
#define pios_fstat(fd, buf) 		yaffs_fstat(fd, buf) 
#define pios_utime(path, buf) 		yaffs_utime(path, buf)
#define pios_futime(fd, buf) 		yaffs_futime(fd, buf)
#define pios_setxattr(path, name, data, size, flags) 	yaffs_setxattr(path, name, data, size, flags)
#define pios_lsetxattr(path, name, data, size, flags) yaffs_lsetxattr(path, name, data, size, flags)
#define pios_fsetxattr(fd, name, data, size, flags) 	yaffs_fsetxattr(path, name, data, size, flags)

#define pios_getxattr(path, name, data, size) yaffs_getxattr(path, name, data, size)
#define pios_lgetxattr(path, name, data, size) yaffs_lgetxattr(path, name, data, size)
#define pios_fgetxattr(fd, name, data, size) yaffs_fgetxattr(fd, name, data, size)
#define pios_removexattr(path, name) 	yaffs_removexattr(path, name)
#define pios_lremoveattr(path, name) 	yaffs_lremovexattr(path, name)
#define pios_fremovexattr(fd, name) 	yaffs_fremovexattr(fd, name)
#define pios_listxattr(path, list, size) yaffs_listxattr(path, list, size)
#define pios_llistxattr(path, list,size) yaffs_llistxattr(path, list, size)
#define pios_flistxattr(fd, list, size) yaffs_flistxattr(fd, list, size)
#define pios_chmod(path, mode) 		yaffs_chmod(path, mode)
#define pios_fchmod(fd, mode) 		yaffs_fchmod(fd, mode)
#define pios_mkdir(path, mode) 		yaffs_mkdir(path, mode) 
#define pios_rmdir(path) 		yaffs_rmdir(path) 
#define pios_opendir(dirname) 		yaffs_opendir(dirname) 
#define pios_readdir(dirp) 		yaffs_readdir(dirp)
#define pios_rewinddir(dirp) 		yaffs_rewinddir(dirp) 
#define pios_closedir(dirp) 		yaffs_closedir(dirp) 
#define pios_mount(path) 		yaffs_mount(path) 
#define pios_mount2(path, read_only) 	yaffs_mount2(path, read_only)
#define pios_mount3(path, read_only, skip_checkpt) yaffs_mount3(path, read_only, skip_checkpt)
#define pios_umount(path) 		yaffs_unmount(path) 
#define pios_umount2(path, force) 	yaffs_unmount2(path, force)
#define pios_remount(path, force, read_only) yaffs_remount(path, force,read_only)
#define pios_format(path, unmount_flag, force_unmount_flag, remount_flag)  yaffs_format(path, unmount_flag, force_unmount_flag, remount_flag)
#define pios_sync(path) 		yaffs_sync(path) 
#define pios_symlink(oldpath, newpath) 	yaffs_symlink(oldpath, newpath)
#define pios_readlink(path, buf, bufsize) yaffs_readlink(path, buf, bufsiz)
#define pios_link(oldpath, newpath) 	yaffs_link(oldpath, newpath)
#define pios_mknod(pathname, mode, dev) yaffs_mknod(pathname, mode, dev)
#define pios_freespace(path) 		yaffs_freespace(path)
#define pios_totalspace(path) 		yaffs_totalspace(path)


#endif /* PIOS_STDIO_H */
