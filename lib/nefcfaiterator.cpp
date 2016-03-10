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

#include "nefcfaiterator.hpp"

namespace OpenRaw {
namespace Internals {

NefCfaIterator::NefCfaIterator(const NefDiffIterator& diffs, size_t rows,
							   size_t columns, const uint16_t init[2][2])
	: m_diffs(diffs), m_rows(rows),
	  m_columns(columns), m_row(0),
	  m_column(0)
{
	for (int i = 0; i < 2; ++i) {
		for (int j = 0; j < 2; ++j) {
			m_vpred[i][j] = init[i][j];
		}
		m_hpred[i] = 0x148;
	}
}

uint16_t NefCfaIterator::get()
{
	int diff = m_diffs.get();
	uint16_t ret;
	if (m_column < 2) {
		ret = m_vpred[m_row & 1][m_column] + diff;
		m_vpred[m_row & 1][m_column] = ret;

	} else {
		ret = m_hpred[m_column & 1] + diff;
	}
	m_hpred[m_column & 1] = ret;

	m_column++;
	if (m_column == m_columns) {
		m_column = 0;
		m_row++;
	}
	return ret;
}

}
}

