cmake_minimum_required (VERSION 2.8)

SET(CMAKE_TOOLCHAIN_FILE "build/ToolChain.cmake")
#SET(CMAKE_SYSTEM_NAME "Linux")
#SET(CMAKE_C_COMPILER "gcc")
#SET(CMAKE_CXX_COMPILER "g++")


project (WebRTC)
 
#SET(CMAKE_C_FLAGS "-g")
# 设置C++标准为C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(src_list
    MediaHandle.cpp
    FMP4/FMP4HandleInterface.cpp
    FMP4/FMP4Handle.cpp
    FMP4/FMP4.cpp
    FLV/FlvHandleInterface.cpp
    FLV/FlvHandle.cpp
    RawVideoHandle.cpp
    RawAudioHandle.cpp
	)		
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}
	${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/FLV/include
    ${PROJECT_SOURCE_DIR}/FLV
    ${PROJECT_SOURCE_DIR}/FMP4/include
    ${PROJECT_SOURCE_DIR}/FMP4
	)
ADD_LIBRARY(media_static STATIC ${src_list})
SET_TARGET_PROPERTIES(media_static PROPERTIES OUTPUT_NAME media)


SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build/lib)

#TARGET_LINK_LIBRARIES(webrtc pthread -lm)

#TARGET_LINK_LIBRARIES(crnode log4cpp thrift)










