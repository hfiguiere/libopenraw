/*
 * libopenraw - trace.h
 *
 * Copyright (C) 2006-2013 Hubert Figuiere
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


#ifndef OR_INTERNALS_TRACE_H_
#define OR_INTERNALS_TRACE_H_

#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

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
		Trace & operator<<(const std::string & s);

		template <class T>
		Trace & operator<<(const std::vector<T> & v);

		static void setDebugLevel(debug_level lvl);
	private:
		static void print(int i);
		static int debugLevel; // global debug level
		int m_level;
	};


	template <class T>
	Trace & Trace::operator<<(const std::vector<T> & v)
	{
		if (m_level <= debugLevel) {
			std::for_each(v.begin(), v.end(),
				      std::bind(&print, std::placeholders::_1));
		}
		return *this;
	}

	
}

#endif
