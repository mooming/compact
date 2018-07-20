cmake_minimum_required (VERSION 2.6)
project (Source)

if(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${ CMAKE_CXX_FLAGS } -Wall -Werror")
endif(CMAKE_COMPILER_IS_GNUCXX)
add_definitions (-std=c++17 -D__UNIT_TEST__ )

include_directories ( ${CMAKE_SOURCE_DIR}
 ${CMAKE_SOURCE_DIR}/include
 )

link_directories (${CMAKE_SOURCE_DIR}/lib)



add_executable (Source
 Main.cpp
 MeasureSec.cpp
 SymbolData.cpp
 SymbolDataTest.cpp
 MeasureSec.h
 SymbolData.h
 SymbolDataTest.h
)

if (APPLE)
include_directories ( /System/Library/Frameworks )
endif (APPLE)

install (TARGETS Source DESTINATION ${CMAKE_SOURCE_DIR}/bin)



