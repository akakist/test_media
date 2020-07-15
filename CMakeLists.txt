project(MFPipe_Test)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14  ")

set(SOURCES
	unittest_mfpipe.cpp
	ioBuffer.cpp
	url.cpp
	commonError.cpp
)

set(HEADERS
	MFPipe.h
	MFPipeImpl.h
	MFTypes.h
)

add_executable(MFPipe_Test ${SOURCES} ${HEADERS})
