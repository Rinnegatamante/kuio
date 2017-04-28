#ifndef _KUIO_H_
#define _KUIO_H_

/**
 * Open or create a file for reading or writing
 *
 * @par Example1: Open a file for reading
 * @code
 * SceUID fd;
 * kuIoOpen("device:/path/to/file", SCE_O_RDONLY, &fd)
 * if(!(fd) {
 *	// error
 * }
 * @endcode
 *
 * @param file - Pointer to a string holding the name of the file to open
 * @param flags - Libc styled flags that are or'ed together
 * @param res - Pointer to where to save the returned file descriptor.
 *
 */
int kuIoOpen(const char *file, int flags, SceUID* res);

/**
 * Delete a descriptor
 *
 * @code
 * kuIoClose(fd);
 * @endcode
 *
 * @param fd - File descriptor to close
 *
 */
int kuIoClose(SceUID fd);

/**
 * Read input
 *
 * @par Example:
 * @code
 * kuIoRead(fd, data, 100);
 * @endcode
 *
 * @param fd - Opened file descriptor to read from
 * @param data - Pointer to the buffer where the read data will be placed
 * @param size - Size of the read in bytes
 *
 */
int kuIoRead(SceUID fd, void *data, SceSize size);

/**
 * Write output
 *
 * @par Example:
 * @code
 * kuIoWrite(fd, data, 100);
 * @endcode
 *
 * @param fd - Opened file descriptor to write to
 * @param data - Pointer to the data to write
 * @param size - Size of data to write
 *
 */
int kuIoWrite(SceUID fd, const void *data, SceSize size);


/**
 * Reposition read/write file descriptor offset
 *
 * @par Example:
 * @code
 * SceOff pos
 * kuIoLseek(fd, -10, SEEK_END, &pos);
 * @endcode
 *
 * @param fd - Opened file descriptor with which to seek
 * @param offset - Relative offset from the start position given by whence
 * @param whence - Set to SEEK_SET to seek from the start of the file, SEEK_CUR
 * seek from the current position and SEEK_END to seek from the end.
 *
 */
int kuIoLseek(SceUID fd, int offset, int whence);

/**
 * Remove directory entry
 *
 * @param file - Path to the file to remove
 *
 */
int kuIoRemove(const char *file);

/**
 * Make a directory file
 *
 * @param dir
 * @param mode - Access mode.
 */
int kuIoMkdir(const char *dir);

/**
 * Remove a directory file
 *
 * @param path - Removes a directory file pointed by the string path
 */
int kuIoRmdir(const char *path);

/**
 * Return the position in the file
 *
 * @param fd - Opened file descriptor with which to seek
 * @param pos - The position in the file after the seek
 *
 */
int kuIoTell(SceUID fd, SceOff* pos);

#endif