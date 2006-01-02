/*
 * libopenraw - thumbnails.h
 *
 * Copyright (C) 2005 Hubert Figuiere
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#ifndef __OR_THUMBNAILS_H__
#define __OR_THUMBNAILS_H__


#include <libopenraw/libopenraw.h>

/** real thumbnail extracted */
struct _ORThumbnail 
{
	/** raw data */
	char *data;
	/** size in bytes of raw data */
	size_t data_size;
	/** type of thumbnail data */
	or_data_type data_type;
	/** x dimension in pixels of thumbnail data */
	int x;
	/** y dimension in pixels of thumbnail data */
	int y;
	/** size type of thumbnail */
	or_thumb_size thumb_size;
};


#endif
