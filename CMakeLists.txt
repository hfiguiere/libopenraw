# written by Hubert Figuiere

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

project(LIBOPENRAW)

subdirs(lib demo test)
include_directories (${LIBOPENRAW_SOURCE_DIR}/include)
if (APPLE)
	include_directories(/sw/include)
endif(APPLE)	
