cmake_minimum_required (VERSION 2.8)

SET(CMAKE_TOOLCHAIN_FILE "build/ToolChain.cmake")
#SET(CMAKE_SYSTEM_NAME "Linux")
#SET(CMAKE_C_COMPILER "gcc")
#SET(CMAKE_CXX_COMPILER "g++")


project (WebRTC)
 
#SET(CMAKE_C_FLAGS "-g")


SET(src_list
    peerconnection_client.cpp
	)		
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}
	${PROJECT_SOURCE_DIR}/include
	${PROJECT_SOURCE_DIR}/../http/include
	)
ADD_LIBRARY(peerconnection_client_static STATIC ${src_list})
SET_TARGET_PROPERTIES(peerconnection_client_static PROPERTIES OUTPUT_NAME peerconnection_client)


SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build/lib)

#TARGET_LINK_LIBRARIES(webrtc pthread -lm)

#TARGET_LINK_LIBRARIES(crnode log4cpp thrift)










