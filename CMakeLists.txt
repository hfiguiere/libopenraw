# written by Hubert Figuiere

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

SET(CMAKE_CXX_FLAGS "-ansi -pedantic -g -Wall -Werror -Weffc++")
SET(CMAKE_C_FLAGS "-pedantic -g -Wall")

project(LIBOPENRAW)

subdirs(lib demo test)
include_directories (${LIBOPENRAW_SOURCE_DIR}/include)
if (APPLE)
	include_directories(/sw/include)
endif(APPLE)	
