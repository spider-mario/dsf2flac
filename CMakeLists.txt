cmake_minimum_required(VERSION 2.6)
project(dsf2flac)

set(CMAKE_CXX_FLAGS "-O3 -Wall")
set(CMAKE_EXE_LINKER_FLAGS "-static")
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmakemodules)

set( DSF2FLAC_SOURCE_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/src/tagConversion.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/dop_packer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/dsd_sample_reader.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/dsf_file_reader.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/filters.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/fstream_plus.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/dsd_decimator.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/dsdiff_file_reader.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/cmdline.c
)

set( LIBDSTDEC_SOURCE_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/src/libdstdec/ccp_calc.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/libdstdec/unpack_dst.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/libdstdec/dst_ac.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/libdstdec/dst_data.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/libdstdec/dst_fram.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/libdstdec/dst_init.c
)

find_package(Boost REQUIRED COMPONENTS system timer filesystem chrono) 
find_package(Ogg REQUIRED)
find_package(Flac REQUIRED)
find_package(Id3 REQUIRED)
find_package(Z REQUIRED)
find_package(Rt REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/libdstdec)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${Ogg_INCLUDE_DIRS})
include_directories(${Flac_INCLUDE_DIRS})
include_directories(${Id3_INCLUDE_DIRS})
include_directories(${Z_INCLUDE_DIRS})
include_directories(${Rt_INCLUDE_DIRS})

add_executable(dsf2flac
	${DSF2FLAC_SOURCE_FILES}
	${LIBDSTDEC_SOURCE_FILES}
)

target_link_libraries(dsf2flac
	${Flac_LIBRARIES}
	${Boost_LIBRARIES}
	${Ogg_LIBRARIES}
	${Id3_LIBRARIES}
	${Z_LIBRARIES}
	${Rt_LIBRARIES}
)


