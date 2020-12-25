/*
 * libopenraw - io.h
 *
 * Copyright (C) 2005-2020 Hubert Figuière
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef LIBOPENRAW_IO_H_
#define LIBOPENRAW_IO_H_

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

/** @defgroup io_api IO API
 * @ingroup public_api
 *
 * @brief API to implement custom IO.
 *
 * In most case you don't need to use the IO API. The default implementation
 * uses POSIX IO. But if you need an alternative, this is what you should use.
 *
 * @todo This API is incomplete.
 *
 * @{
 */
#ifdef __cplusplus
extern "C" {
#endif

/** @brief A file reference */
typedef struct _IOFile *IOFileRef;

/** @brief IO methods for the IO subsystem.
 *
 * This allow implementing custom IO callback.
 */
struct io_methods {
    /** @brief open method
     * @return a descriptor
     */
    IOFileRef (*open)(const char *path, int mode);
    /** @brief close method */
    int (*close) (IOFileRef f);
    /** @brief seek in the file */
    int (*seek) (IOFileRef f, off_t offset, int whence);
    /** @brief read method */
    int (*read) (IOFileRef f, void *buf, size_t count);

    /** @brief filesize method */
    off_t (*filesize) (IOFileRef f);
    /** @brief mmap method */
    void* (*mmap) (IOFileRef f, size_t l, off_t offset);
    /** @brief munmap method */
    int (*munmap) (IOFileRef f, void *addr, size_t l);
};

/** @defgroup io_internals Internal IO API
 * @ingroup io_api
 *
 * @brief API called internally for IO.
 *
 * @{
 */

/** @brief Get the default IO methods
 *
 * @return the default io_methods instance, currently posix_io_methods
 */
extern struct io_methods* get_default_io_methods(void);

/** @brief Raw open function
 *
 * @param methods The IO methods to use for this file.
 * @param path The file path to open.
 * @param mode The open mode.
 * @return A file reference. Will be freed when closing.
 */
extern IOFileRef raw_open(struct io_methods * methods, const char *path,
			      int mode);
extern int raw_close(IOFileRef f);
extern int raw_seek(IOFileRef f, off_t offset, int whence);
extern int raw_read(IOFileRef f, void *buf, size_t count);
extern off_t raw_filesize(IOFileRef f);
extern void *raw_mmap(IOFileRef f, size_t l, off_t offset);
extern int raw_munmap(IOFileRef f, void *addr, size_t l);

extern int raw_get_error(IOFileRef f);
extern char *raw_get_path(IOFileRef f);

/** @} */

#ifdef __cplusplus
}
#endif

/** @} */

#endif
