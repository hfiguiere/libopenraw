# written by Hubert Figuiere

link_directories (${LIBOPENRAW_BINARY_DIR}/lib)
include_directories (${LIBOPENRAW_SOURCE_DIR}/lib)

add_executable(thumb thumb.cpp)
target_link_libraries(thumb openraw)
add_executable(thumbc thumbc.c)
target_link_libraries(thumbc openraw stdc++)

if(APPLE)
	set (CMAKE_LIBRARY_PATH /sw/lib)
	set (CMAKE_INCLUDE_PATH /sw/include)
endif(APPLE)