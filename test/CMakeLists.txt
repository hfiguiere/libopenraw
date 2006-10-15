# written by Hubert Figuiere

link_directories (${LIBOPENRAW_BINARY_DIR}/lib)
include_directories (${LIBOPENRAW_SOURCE_DIR}/lib)

add_executable(fileio fileio.t.c)
target_link_libraries(fileio openraw)
add_executable(ciffcontainertest ciffcontainertest.cpp)
target_link_libraries(ciffcontainertest openraw)

if(APPLE)
	set (CMAKE_LIBRARY_PATH /sw/lib)
	set (CMAKE_INCLUDE_PATH /sw/include)
endif(APPLE)
