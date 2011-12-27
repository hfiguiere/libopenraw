

#ifndef _RAWFILE_PRIV_H__
#define _RAWFILE_PRIV_H__

namespace OpenRaw {
namespace Internals {

/** describe the location of a thumbnail in an RAW file */
struct ThumbDesc
{
	ThumbDesc(uint32_t _x, uint32_t _y, ::or_data_type _type,
            size_t _offset, size_t _length)
		: x(_x), y(_y), type(_type)
    , offset(_offset), length(_length)
		{
		}
	ThumbDesc()
		: x(0), y(0), type(OR_DATA_TYPE_NONE)
    , offset(0), length(0)
		{
		}
	uint32_t x;    /**< x size. Can be 0 */
	uint32_t y;    /**< y size. Can be 0 */
	::or_data_type type; /**< the data type format */
  size_t   offset; /**< offset if the thumbnail data */
  size_t   length;
};

}
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0))
  tab-width:2
  c-basic-offset:2
  indent-tabs-mode:nil
  fill-column:80
  End:
*/
#endif
