/*
 * libopenraw - tiffepfile.h
 *
 * Copyright (C) 2007-2008 Hubert Figuiere
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


#ifndef _TIFF_EP_FILE_H_
#define _TIFF_EP_FILE_H_

#include "ifdfile.h"


namespace OpenRaw {
	namespace Internals {


		/** This is for TIFF EP conformant files. This include DNG, NEF, 
		 *  ERF */
		class TiffEpFile
			: public IFDFile
		{

		protected:
			TiffEpFile(const char *_filename, Type _type);

			virtual IFDDir::Ref  _locateCfaIfd();
			virtual IFDDir::Ref  _locateMainIfd();

		};

	}
}

#endif
