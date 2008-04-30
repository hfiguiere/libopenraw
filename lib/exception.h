/*
 * libopenraw - exception.h
 *
 * Copyright (C) 2006-2007 Hubert Figuiere
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



#ifndef _OPENRAW_EXCEPTION_H_
#define _OPENRAW_EXCEPTION_H_

#include <exception>
#include <string>
#include <typeinfo>

namespace OpenRaw {
	namespace Internals {

		/** generic OpenRaw exception */
		class Exception
			: public std::exception
		{
		protected:
			std::string m_what;
		public:
			Exception()
				: std::exception(),
					m_what()
				{}
			Exception(const std::string &w)
				: std::exception(),
					m_what(w)
				{}
			virtual ~Exception() throw()
				{}
			const char *what() const throw()
				{ 
					if(m_what.empty()) {
						return typeid(this).name();
					}
					return m_what.c_str();
				}
		};

		/** IO exception */
		class IOException
			: public Exception
		{
		public:
			IOException(const std::string &w)
				: Exception(w)
				{}
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

		class DecodingException
			: public Exception
		{
		public:
			DecodingException(const std::string &w)
				: Exception(w)
				{}
		};

	}
}

#endif
