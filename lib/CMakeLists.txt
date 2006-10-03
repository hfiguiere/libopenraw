# written by Hubert Figuiere

include_directories (${LIBOPENRAW_SOURCE_DIR}/lib/ ${LIBOPENRAW_SOURCE_DIR}/lib/io)
set(LIBOPENRAW_SRCS ${LIBOPENRAW_SRCS}
		    arwfile.cpp  ifddir.cpp 
		    neffile.cpp  rawcontainer.cpp
		    cr2file.cpp  ifdentry.cpp 
		    orfcontainer.cpp  rawfile.cpp
		    debug.cpp ifdfilecontainer.cpp
		    orffile.cpp rawfilefactory.cpp
		    dngfile.cpp iofile.cpp
		    peffile.cpp thumbnail.cpp
		    io/io.c io/posix_io.c capi/capi.cpp)
add_library(openraw SHARED ${LIBOPENRAW_SRCS})

