/*
 * Copyright (C) 2008 Brian Quinlan
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

#include <Python.h>

#include <boost/python.hpp>
#include <boost/python/list.hpp>
#include <boost/python/tuple.hpp>
#include <boost/python/extract.hpp>
#include <boost/python/to_python_converter.hpp>

#include <libopenraw++/bitmapdata.h>
#include <libopenraw++/rawdata.h>
#include <libopenraw++/rawfile.h>
#include <libopenraw++/thumbnail.h>

#include <string>

#include "pythonrawfile.h"

using namespace boost::python;
using namespace boost;

/**
 * Implements a converter from an STL container to a Python list.
 */
template < typename ContainerType >
struct to_list
{
    static boost::python::object makeobject(ContainerType const& container)
    {
        boost::python::list result;

        for (typename ContainerType::const_iterator i = container.begin();
             i != container.end();
             ++i) {
            result.append(*i);
        }
        return result;
    }
    static PyObject* convert(ContainerType const& container)
    {
        return boost::python::incref(makeobject(container).ptr());
    }
};

/**
 * Implements a converter from a STL vector to a Python list.
 */
template <typename T>
struct std_vector_to_list
{
    std_vector_to_list()
    {
        to_python_converter< std::vector< T >,
                             to_list< std::vector < T > >  > ();
    }
};

/*
 * An exception containing an OpenRaw or_error value.
 */
class OpenRawException {
public:
    OpenRawException(or_error _error) {
        error = _error;
    }

    or_error get_or_error() const {
        return error;
    }
protected:
    or_error error;
};

/**
 * Translates OpenRawExceptions into Python exceptions.
 */
void OpenRawExceptionTranslator(OpenRawException const& error) {
    PyObject* exception_value = Py_BuildValue("i", (int) error.get_or_error());

    if (exception_value != NULL) {
        PyErr_SetObject(PyExc_Exception, exception_value);
    }
}

/**
 * Provides a memory-safe facade to OpenRaw::BitmapData.data.
 */
std::string BitmapData_data(OpenRaw::BitmapData* bitmap) {
    return std::string((char *) bitmap->data(), bitmap->size());
}

/**
 * Provides a memory-safe facade to OpenRaw::BitmapData.setData.
 */
or_error BitmapData_setData(OpenRaw::BitmapData* bitmap, std::string data) {
    free(bitmap->data());
    void* newData = bitmap->allocData(data.size());
    if (newData == NULL) {
        bitmap->allocData(data.size());
        return OR_ERROR_OUT_OF_MEMORY;
    } else {
        memcpy(newData, data.data(), data.size());
        return OR_ERROR_NONE;
    }
}

/**
 * Provides a facade to OpenRaw::Thumbnail.getAndExtractThumbnail that throws
 * an exception on error.
 */
OpenRaw::Thumbnail* Thumbnail_getAndExtractThumbnail(const char* filename,
                                                     uint32_t preferred_size) {
    or_error error;

    OpenRaw::Thumbnail *thumbnail = OpenRaw::Thumbnail::getAndExtractThumbnail(
        filename, preferred_size, error);

    if (error != OR_ERROR_NONE) {
        throw OpenRawException(error);
    }

    return thumbnail;
}

BOOST_PYTHON_MODULE(_openraw)
{
    OpenRaw::init();

    std_vector_to_list < uint32_t > ();
    register_exception_translator<OpenRawException>(OpenRawExceptionTranslator);

    enum_< or_error > ("or_error")
        .value("OR_ERROR_NONE", OR_ERROR_NONE)
        .value("OR_ERROR_BUF_TOO_SMALL", OR_ERROR_BUF_TOO_SMALL)
        .value("OR_ERROR_NOTAREF", OR_ERROR_NOTAREF)
        .value("OR_ERROR_CANT_OPEN", OR_ERROR_CANT_OPEN)
        .value("OR_ERROR_CLOSED_STREAM", OR_ERROR_CLOSED_STREAM)
        .value("OR_ERROR_NOT_FOUND", OR_ERROR_NOT_FOUND)
        .value("OR_ERROR_INVALID_PARAM", OR_ERROR_INVALID_PARAM)
        .value("OR_ERROR_INVALID_FORMAT", OR_ERROR_INVALID_FORMAT)
        .value("OR_ERROR_OUT_OF_MEMORY", OR_ERROR_OUT_OF_MEMORY)
        .value("OR_ERROR_UNKNOWN", OR_ERROR_UNKNOWN)
        .value("OR_ERROR_LAST_", OR_ERROR_LAST_)
        ;

    enum_ <or_rawfile_type> ("Type")
        .value("OR_RAWFILE_TYPE_UNKNOWN", OR_RAWFILE_TYPE_UNKNOWN)
        .value("OR_RAWFILE_TYPE_CR2", OR_RAWFILE_TYPE_CR2)
        .value("OR_RAWFILE_TYPE_CRW", OR_RAWFILE_TYPE_CRW)
        .value("OR_RAWFILE_TYPE_NEF", OR_RAWFILE_TYPE_NEF)
        .value("OR_RAWFILE_TYPE_MRW", OR_RAWFILE_TYPE_MRW)
        .value("OR_RAWFILE_TYPE_ARW", OR_RAWFILE_TYPE_ARW)
        .value("OR_RAWFILE_TYPE_DNG", OR_RAWFILE_TYPE_DNG)
        .value("OR_RAWFILE_TYPE_ORF", OR_RAWFILE_TYPE_ORF)
        .value("OR_RAWFILE_TYPE_PEF", OR_RAWFILE_TYPE_PEF)
        .value("OR_RAWFILE_TYPE_ERF", OR_RAWFILE_TYPE_ERF)
        ;

    enum_ < or_data_type > ("DataType")
        .value("OR_DATA_TYPE_NONE", OR_DATA_TYPE_NONE)
        .value("OR_DATA_TYPE_PIXMAP_8RGB", OR_DATA_TYPE_PIXMAP_8RGB)
        .value("OR_DATA_TYPE_JPEG", OR_DATA_TYPE_JPEG)
        .value("OR_DATA_TYPE_TIFF", OR_DATA_TYPE_TIFF)
        .value("OR_DATA_TYPE_PNG", OR_DATA_TYPE_PNG)
        .value("OR_DATA_TYPE_CFA", OR_DATA_TYPE_CFA)
        .value("OR_DATA_TYPE_COMPRESSED_CFA", OR_DATA_TYPE_COMPRESSED_CFA)
        .value("OR_DATA_TYPE_UNKNOWN", OR_DATA_TYPE_UNKNOWN)
        ;

    enum_ < or_options > ("or_options")
        .value("OR_OPTIONS_NONE", OR_OPTIONS_NONE)
        .value("OR_OPTIONS_DONT_DECOMPRESS", OR_OPTIONS_DONT_DECOMPRESS)
        ;

    class_ < OpenRaw::BitmapData, noncopyable > ("BitmapData")
        .def("swap", &OpenRaw::BitmapData::swap)
        .def("dataType", &OpenRaw::BitmapData::dataType)
        .def("setDataType", &OpenRaw::BitmapData::setDataType)
        .def("size", &OpenRaw::BitmapData::size)
        .def("data", &BitmapData_data)
        .def("setData", &BitmapData_setData)  // replaces allocData
        .def("x", &OpenRaw::BitmapData::x)
        .def("y", &OpenRaw::BitmapData::y)
        .def("bpc", &OpenRaw::BitmapData::bpc)
        .def("setBpc", &OpenRaw::BitmapData::setBpc)
        .def("setDimensions", &OpenRaw::BitmapData::setDimensions)
        ;

    class_ < OpenRaw::Thumbnail, bases < OpenRaw::BitmapData > ,
           noncopyable > ("Thumbnail")
        .def("getAndExtractThumbnail",
             &Thumbnail_getAndExtractThumbnail,
             return_value_policy<manage_new_object>())
        .staticmethod("getAndExtractThumbnail")
        ;

    class_ < OpenRaw::RawData, bases < OpenRaw::BitmapData > ,
           noncopyable > ("RawData")
        .def("min", &OpenRaw::RawData::min)
        .def("max", &OpenRaw::RawData::max)
        .def("setMin", &OpenRaw::RawData::setMin)
        .def("setMax", &OpenRaw::RawData::setMax)
        .def("compression", &OpenRaw::RawData::compression)
        .def("setCompression", &OpenRaw::RawData::setCompression)
        // Some methods missing.
        ;

    class_ < PythonRawFile, noncopyable > ("RawFile", no_init)
        .def("newRawFile",
             &PythonRawFile::newRawFile,
             return_value_policy < manage_new_object > ())
        .staticmethod("newRawFile")
        .def("newRawFileFromMemory",
             &PythonRawFile::newRawFileFromMemory,
             return_value_policy < manage_new_object > ())
        .staticmethod("newRawFileFromMemory")
        .def("type", &PythonRawFile::type)
        .def("listThumbnailSizes", &PythonRawFile::listThumbnailSizes,
            return_value_policy < copy_const_reference > ())
        .def("getThumbnail", &PythonRawFile::getThumbnail)
        .def("getRawData", &PythonRawFile::getRawData)
        .def("getOrientation", &PythonRawFile::getOrientation)
        ;

}
