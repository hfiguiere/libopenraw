/*
 * libopenraw - jfifcontainer.cpp
 *
 * Copyright (C) 2006-2017 Hubert Figuière
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


#include <setjmp.h>
#include <fcntl.h>
#include <stddef.h>
#include <string.h>
#include <memory>

/*
 * The extern "C" below is REQUIRED for libjpeg-mmx-dev
 * as found on debian because some people have this installed.
 */
extern "C" {
#include <jpeglib.h>
}

#include <libopenraw/debug.h>

#include "bitmapdata.hpp"
#include "io/stream.hpp"
#include "io/streamclone.hpp"
#include "trace.hpp"
#include "jfifcontainer.hpp"
#include "ifdfilecontainer.hpp"

namespace OpenRaw {

using namespace Debug;

namespace Internals {

namespace {

/* libjpeg callbacks j_ is the prefix for these callbacks */
void j_init_source(::j_decompress_ptr cinfo);
::boolean j_fill_input_buffer(::j_decompress_ptr cinfo);
void j_skip_input_data(::j_decompress_ptr cinfo,
                                long num_bytes);
void j_term_source(::j_decompress_ptr cinfo);
void j_error_exit(::j_common_ptr cinfo);

}

/** private source struct for libjpeg
 */
#define BUF_SIZE 1024

typedef struct {
  struct jpeg_source_mgr pub; /**< the public libjpeg struct */
  JfifContainer * self;       /**< pointer to the owner */
  off_t offset;
  JOCTET* buf;
} jpeg_src_t;

JfifContainer::JfifContainer(const IO::Stream::Ptr &_file, off_t _offset)
  : RawContainer(_file, _offset),
    m_cinfo(), m_jerr(),
    m_headerLoaded(false)
{
  setEndian(ENDIAN_BIG);
  /* this is a hack because jpeg_create_decompress is
   * implemented as a Macro
   */

  m_cinfo.err = jpeg_std_error(&m_jerr);
  m_jerr.error_exit = &j_error_exit;
  jpeg_create_decompress(&m_cinfo);

  /* inspired by jdatasrc.c */

  jpeg_src_t *src = (jpeg_src_t *)
    (*m_cinfo.mem->alloc_small)((j_common_ptr)&m_cinfo,
                                JPOOL_PERMANENT,
                                sizeof(jpeg_src_t));
  m_cinfo.src = (jpeg_source_mgr*)src;
  src->pub.init_source = j_init_source;
  src->pub.fill_input_buffer = j_fill_input_buffer;
  src->pub.skip_input_data = j_skip_input_data;
  src->pub.resync_to_restart = jpeg_resync_to_restart;
  src->pub.term_source = j_term_source;
  src->self = this;
  src->pub.bytes_in_buffer = 0;
  src->pub.next_input_byte = nullptr;
  src->buf = (JOCTET*)(*m_cinfo.mem->alloc_small)
    ((j_common_ptr)&m_cinfo,
     JPOOL_PERMANENT,
     BUF_SIZE * sizeof(JOCTET));
}

JfifContainer::~JfifContainer()
{
  jpeg_destroy_decompress(&m_cinfo);
}


bool JfifContainer::getDimensions(uint32_t &x, uint32_t &y)
{
  if(!m_headerLoaded) {
    if (_loadHeader() == 0) {
      LOGDBG1("load header failed\n");
      return false;
    }
  }
  x = m_cinfo.output_width;
  y = m_cinfo.output_height;
  return true;
}


bool JfifContainer::getDecompressedData(BitmapData &data)
{
  if(!m_headerLoaded) {
    if (_loadHeader() == 0) {
      LOGDBG1("load header failed\n");
      return false;
    }
  }
  if (::setjmp(m_jpegjmp) != 0) {
    return false;
  }
  jpeg_start_decompress(&m_cinfo);
  int row_size = m_cinfo.output_width * m_cinfo.output_components;
  char *dataPtr
    = (char*)data.allocData(row_size * m_cinfo.output_height);
  char *currentPtr = dataPtr;
  JSAMPARRAY buffer
    = (*m_cinfo.mem->alloc_sarray)((j_common_ptr)&m_cinfo,
                                   JPOOL_IMAGE, row_size,
                                   1);
  while (m_cinfo.output_scanline < m_cinfo.output_height) {
    jpeg_read_scanlines(&m_cinfo, buffer, 1);
    memcpy(currentPtr, buffer, row_size);
    currentPtr += row_size;
  }
  data.setDimensions(m_cinfo.output_width, m_cinfo.output_height);

  jpeg_finish_decompress(&m_cinfo);
  return true;
}


int JfifContainer::_loadHeader()
{

  m_file->seek(0, SEEK_SET);

  if (::setjmp(m_jpegjmp) == 0) {
    int ret = jpeg_read_header(&m_cinfo, TRUE);

    jpeg_calc_output_dimensions(&m_cinfo);
    m_headerLoaded = (ret == 1);
    return ret;
  }
  return 0;
}

namespace {

void j_error_exit(j_common_ptr cinfo)
{
  (*cinfo->err->output_message) (cinfo);
  JfifContainer *self = ((jpeg_src_t *)(((j_decompress_ptr)cinfo)->src))->self;
  ::longjmp(self->jpegjmp(), 1);
}

void j_init_source(j_decompress_ptr)
{
}


boolean j_fill_input_buffer(j_decompress_ptr cinfo)
{
  jpeg_src_t *src = (jpeg_src_t*)cinfo->src;
  JfifContainer *self = src->self;
  int n = self->file()->read(src->buf, BUF_SIZE * sizeof(*src->buf));
  if (n >= 0) {
    src->pub.next_input_byte = src->buf;
    src->pub.bytes_in_buffer = n;
  }
  else {
    src->pub.next_input_byte = nullptr;
    src->pub.bytes_in_buffer = 0;
  }
  return TRUE;
}


void j_skip_input_data(j_decompress_ptr cinfo,
                                      long num_bytes)
{
  jpeg_src_t *src = (jpeg_src_t*)cinfo->src;
  if (num_bytes > 0) {
    while ((size_t)num_bytes > src->pub.bytes_in_buffer) {
      num_bytes -= src->pub.bytes_in_buffer;
      j_fill_input_buffer(cinfo);
    }
    src->pub.next_input_byte += (size_t) num_bytes;
    src->pub.bytes_in_buffer -= (size_t) num_bytes;
  }
}


void j_term_source(j_decompress_ptr)
{
}

}

std::unique_ptr<IfdFileContainer> & JfifContainer::ifdContainer()
{
  if(!m_ifd) {
    m_file->seek(0, SEEK_SET);

    // XXX check results and bail.
    auto result = readUInt16(m_file); // SOI
    result = readUInt16(m_file); // APP0
    result = readUInt16(m_file); // ignore

    char delim[7];
    delim[6] = 0;
    m_file->read(delim, 6);
    if(memcmp(delim, "Exif\0\0", 6) == 0) {
      size_t exif_offset = m_file->seek(0, SEEK_CUR);
      m_ifd.reset(new IfdFileContainer(
        IO::Stream::Ptr(
          std::make_shared<IO::StreamClone>(m_file, exif_offset)), 0));
    }
  }
  return m_ifd;
}

IfdDir::Ref JfifContainer::mainIfd()
{
  if(ifdContainer()) {
    return m_ifd->setDirectory(0);
  }
  return IfdDir::Ref();
}

IfdDir::Ref JfifContainer::getIfdDirAt(int idx)
{
  if(ifdContainer()) {
    return m_ifd->setDirectory(idx);
  }
  return IfdDir::Ref();
}


IfdDir::Ref JfifContainer::exifIfd()
{
  IfdDir::Ref main = mainIfd();
  return main->getExifIFD();
}

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
