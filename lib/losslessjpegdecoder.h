




#ifndef __OR_INTERNALS_LOSSLESSJPEGDECODER_H_
#define __OR_INTERNALS_LOSSLESSJPEGDECODER_H_


#include <stdint.h>

#include <vector>

#include <boost/scoped_ptr.hpp>

#include "huffmandecoder.h"

namespace OpenRaw {

	class RawContainer;

	namespace Internals {
		
		
		class LosslessJPEGDecoder
		{
		private:

		private:
			LosslessJPEGDecoder();
			void reset (RawContainer & c) throw();
			uint8_t channelCount() const
				{ return m_channelCount; }
			uint16_t height() const
				{ return m_height; }
			uint16_t width() const
				{ return m_width; }
			int rowSize() const
				{ return m_rowSize; }
			std::vector<short> &  loadRow (RawContainer & c) throw();
		};


	}
}

#endif
