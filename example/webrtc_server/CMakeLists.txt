cmake_minimum_required (VERSION 2.8)

SET(CMAKE_TOOLCHAIN_FILE "build/ToolChain.cmake")
#SET(CMAKE_SYSTEM_NAME "Linux")
#SET(CMAKE_C_COMPILER "gcc")
#SET(CMAKE_CXX_COMPILER "g++")


project (WebRTC)
 
#SET(CMAKE_C_FLAGS "-g")


SET(src_list
    webrtc_server.cpp
    webrtc_http_session.cpp
    webrtc_session.cpp
	)		
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}
	${PROJECT_SOURCE_DIR}/include
	${PROJECT_SOURCE_DIR}/../http/include
    ${PROJECT_SOURCE_DIR}/../net/include
	${PROJECT_SOURCE_DIR}/../base64/include
    ${PROJECT_SOURCE_DIR}/../media/include
    ${PROJECT_SOURCE_DIR}/../rtp/include
    ${PROJECT_SOURCE_DIR}/../../build/${CMAKE_ToolChain}/lib/include
    ${PROJECT_SOURCE_DIR}/../../build/${CMAKE_ToolChain}/third_lib/cJSON
	)
ADD_LIBRARY(webrtc_server_static STATIC ${src_list})
SET_TARGET_PROPERTIES(webrtc_server_static PROPERTIES OUTPUT_NAME webrtc_server)


SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build/lib)

#TARGET_LINK_LIBRARIES(webrtc pthread -lm)

#TARGET_LINK_LIBRARIES(crnode log4cpp thrift)










