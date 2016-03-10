/*
 * libopenraw - ljpegdecompressor_priv.h
 *
 * Copyright (C) 2007-2013 Hubert Figuiere
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

#ifndef OR_INTERNALS_LJPEGDECOMPRESSOR_PRIV_H
#define OR_INTERNALS_LJPEGDECOMPRESSOR_PRIV_H

#include <string.h>



namespace OpenRaw {
namespace Internals {
		
/*
* The following structure stores basic information about one component.
*/
typedef struct JpegComponentInfo {
	/*
	 * These values are fixed over the whole image.
	 * They are read from the SOF marker.
	 */
	int16_t componentId;		/* identifier for this component (0..255) */
	int16_t componentIndex;	/* its index in SOF or cPtr->compInfo[]   */
	
	/*
	 * Downsampling is not normally used in lossless JPEG, although
	 * it is permitted by the JPEG standard (DIS). We set all sampling 
	 * factors to 1 in this program.
	 */
	int16_t hSampFactor;		/* horizontal sampling factor */
	int16_t vSampFactor;		/* vertical sampling factor   */
	
	/*
	 * Huffman table selector (0..3). The value may vary
	 * between scans. It is read from the SOS marker.
	 */
	int16_t dcTblNo;
} JpegComponentInfo;


/*
* One of the following structures is created for each huffman coding
* table.  We use the same structure for encoding and decoding, so there
* may be some extra fields for encoding that aren't used in the decoding
* and vice-versa.
*/
struct HuffmanTable {
	/*
	 * These two fields directly represent the contents of a JPEG DHT
	 * marker
	 */
	uint8_t bits[17];
	uint8_t huffval[256];
	
	/*
	 * This field is used only during compression.  It's initialized
	 * FALSE when the table is created, and set TRUE when it's been
	 * output to the file.
	 */
	bool sentTable;
	
	/*
	 * The remaining fields are computed from the above to allow more
	 * efficient coding and decoding.  These fields should be considered
	 * private to the Huffman compression & decompression modules.
	 */
	uint16_t ehufco[256];
	char ehufsi[256];
	
	uint16_t mincode[17];
	int32_t maxcode[18];
	int16_t valptr[17];
	int32_t numbits[256];
	int32_t value[256];
};

/*
 * One of the following structures is used to pass around the
 * decompression information.
 */
struct DecompressInfo
{
	// non copyable
	DecompressInfo(const DecompressInfo&) = delete;
	DecompressInfo& operator=(const DecompressInfo&) = delete;

	DecompressInfo()
		: imageWidth(0), imageHeight(0),
			dataPrecision(0), compInfo(NULL),
			numComponents(0),
			compsInScan(0),
			Ss(0), Pt(0),
			restartInterval(0), restartInRows(0),
			restartRowsToGo(0), nextRestartNum(0)
		
		{
			memset(&curCompInfo, 0, sizeof(curCompInfo));
			memset(&MCUmembership, 0, sizeof(MCUmembership));
			memset(&dcHuffTblPtrs, 0, sizeof(dcHuffTblPtrs));
		}
	~DecompressInfo()
		{
			int i;
			for(i = 0; i < 4; i++) {
				if(dcHuffTblPtrs[i]) {
					free(dcHuffTblPtrs[i]);
				}
			}
			if(compInfo) {
				free(compInfo);
			}
		}
	/*
	 * Image width, height, and image data precision (bits/sample)
	 * These fields are set by ReadFileHeader or ReadScanHeader
	 */ 
	int32_t imageWidth;
	int32_t imageHeight;
	int32_t dataPrecision;
	
	/*
	 * compInfo[i] describes component that appears i'th in SOF
	 * numComponents is the # of color components in JPEG image.
	 */
	JpegComponentInfo *compInfo;
	int16_t numComponents;
	
	/*
	 * *curCompInfo[i] describes component that appears i'th in SOS.
	 * compsInScan is the # of color components in current scan.
	 */
	JpegComponentInfo *curCompInfo[4];
	int16_t compsInScan;
	
	/*
	 * MCUmembership[i] indexes the i'th component of MCU into the
	 * curCompInfo array.
	 */
	int16_t MCUmembership[10];
	
	/*
	 * ptrs to Huffman coding tables, or NULL if not defined
	 */
	HuffmanTable *dcHuffTblPtrs[4];
	
	/* 
	 * prediction seletion value (PSV) and point transform parameter (Pt)
	 */
	int32_t Ss;
	int32_t Pt;
	
	/*
	 * In lossless JPEG, restart interval shall be an integer
	 * multiple of the number of MCU in a MCU row.
	 */
	int32_t restartInterval;/* MCUs per restart interval, 0 = no restart */
	int32_t restartInRows; /*if > 0, MCU rows per restart interval; 0 = no restart*/
	
	/*
	 * these fields are private data for the entropy decoder
	 */
	int32_t restartRowsToGo;	/* MCUs rows left in this restart interval */
	int16_t nextRestartNum;	/* # of next RSTn marker (0..7) */
};

}
}


#endif

