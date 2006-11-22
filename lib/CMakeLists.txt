# written by Hubert Figuiere

find_package(JPEG REQUIRED) 

include_directories (${LIBOPENRAW_SOURCE_DIR}/lib/ 
  ${LIBOPENRAW_SOURCE_DIR}/lib/io/
  ${JPEG_DIR}
  )


set(LIBOPENRAW_SRCS ${LIBOPENRAW_SRCS}
		    arwfile.cpp 
		    ifddir.cpp 
		    neffile.cpp  
		    rawcontainer.cpp
		    cr2file.cpp  
		    ifdentry.cpp 
		    orfcontainer.cpp  
		    rawfile.cpp
		    ifdfile.cpp
		    debug.cpp 
		    ifdfilecontainer.cpp
		    orffile.cpp 
		    rawfilefactory.cpp
		    dngfile.cpp 
		    peffile.cpp 
		    thumbnail.cpp
                    ciffcontainer.cpp 
		    crwfile.cpp
		    jfifcontainer.cpp
		    io/io.c 
		    io/posix_io.c 
		    io/stream.cpp
		    io/file.cpp
		    io/streamclone.cpp
		    capi/capi.cpp
		    )

add_library(openraw SHARED ${LIBOPENRAW_SRCS})

target_link_libraries(openraw ${JPEG_LIBRARY})

