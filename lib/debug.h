/*
 * libopenraw - debug.h
 *
 * Copyright (C) 2006 Hubert Figuiere
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */


#ifndef _OPENRAWPP_DEBUG_H_
#define _OPENRAWPP_DEBUG_H_

#include <libopenraw/debug.h>

namespace Debug {
	
	/** a basic Trace class for debug */
	class Trace 
	{
	public:
		
		Trace(debug_level level)
			: m_level(level)
			{
			}
		Trace & operator<<(int i);
		Trace & operator<<(const char * s);
		Trace & operator<<(void *);

		static void setDebugLevel(debug_level lvl);
	private:
		int m_level;
	};
	
}

#endif
