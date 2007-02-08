/*
 * libopenraw - exception.h
 *
 * Copyright (C) 2006-2007 Hubert Figuiere
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



#ifndef _OPENRAW_EXCEPTION_H_
#define _OPENRAW_EXCEPTION_H_

#include <exception>

namespace OpenRaw {
	namespace Internals {

		/** generic OpenRaw exception */
		class Exception
			: public std::exception
		{

		};


		/** data is of bad type */
		class BadTypeException
			: public Exception
		{

		};

		/** data is of too big */
		class TooBigException
			: public Exception
		{
		};

		class OutOfRangeException
			: public Exception
		{
		};

	}
}

#endif
