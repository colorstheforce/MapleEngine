
cmake_minimum_required(VERSION 3.10)

project(zlib)

set(SRC 
	src/adler32.c
	src/compress.c
	src/crc32.c
	src/deflate.c
	src/gzclose.c
	src/gzlib.c
	src/gzread.c
	src/gzwrite.c
	src/infback.c
	src/inffast.c
	src/inflate.c
	src/inftrees.c
	src/trees.c
	src/uncompr.c
	src/zutil.c
	src/ioapi.c
	src/unzip.c
)

add_library(zlib STATIC ${SRC})

target_include_directories(zlib PUBLIC "${CMAKE_CURRENT_LIST_DIR}/src")

