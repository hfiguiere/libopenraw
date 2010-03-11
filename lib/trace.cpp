/*
 * libopenraw - trace.cpp
 *
 * Copyright (C) 2006-2007, 2010 Hubert Figuiere
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


#include <iostream>

#include "trace.h"



namespace Debug {
		
	int Trace::debugLevel = NOTICE;

	void Trace::setDebugLevel(debug_level lvl)
	{
		debugLevel = lvl;
	}

	void Trace::print(int i)
	{
		std::cerr << i << " ";
	}

	Trace & Trace::operator<<(int i)
	{
		if (m_level <= debugLevel) {
			std::cerr << i;
		}
		return *this;
	}

	Trace & Trace::operator<<(const char * s)
	{
		if (m_level <= debugLevel) {
			std::cerr << s;
		}
		return *this;
	}

	Trace & Trace::operator<<(void *p)
	{
		if (m_level <= debugLevel) {
			std::cerr << p;
		}
		return *this;
	}

	Trace & Trace::operator<<(const std::string & s)
	{
		if (m_level <= debugLevel) {
			std::cerr << s;
		}
		return *this;
	}

}
