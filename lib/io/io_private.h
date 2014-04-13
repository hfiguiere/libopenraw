/*
 * libopenraw - io_private.h
 *
 * Copyright (C) 2005-2006 Hubert Figuiere
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


#ifndef OR_INTERNALS_IO_PRIVATE_H_
#define OR_INTERNALS_IO_PRIVATE_H_

/*! private structure that define the file */
struct _IOFile {
	/** methods for the file IO  */
	struct io_methods* methods;
	/** private data for the methods implementation */
	void* _private;
	/** file name */
	char * path;
	/** error code */
	int error;
};



#endif
