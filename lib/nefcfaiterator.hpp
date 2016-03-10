/* -*- mode:c++; tab-width:4; c-basic-offset:4 -*- */
/*
 * libopenraw - nefcfaiterator.h
 *
 * Copyright (C) 2008 Rafael Avila de Espindola.
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

#ifndef OR_INTERNALS_NEFCFAITERATOR_H_
#define OR_INTERNALS_NEFCFAITERATOR_H_

#include <stddef.h>
#include <stdint.h>

#include "nefdiffiterator.hpp"

namespace OpenRaw {
namespace Internals {

class NefCfaIterator {
	NefDiffIterator m_diffs;
	size_t m_rows;
	size_t m_columns;
	unsigned int m_row;
	unsigned int m_column;
	uint16_t m_vpred[2][2];
	uint16_t m_hpred[2];

public:

	NefCfaIterator (const NefDiffIterator&, size_t, size_t,
					const uint16_t(*)[2]);
	uint16_t get();
};

}
}

#endif
