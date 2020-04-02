cmake_minimum_required (VERSION 2.8)

SET(CMAKE_TOOLCHAIN_FILE "build/ToolChain.cmake")
#SET(CMAKE_SYSTEM_NAME "Linux")
#SET(CMAKE_C_COMPILER "gcc")
#SET(CMAKE_CXX_COMPILER "g++")


project (WebRTC)
 
#SET(CMAKE_C_FLAGS "-g")


SET(src_list
    dtls_only_handshake.cpp
	libnice_interface.cpp
    srtp_interface.cpp
	sctp_interface.cpp
	webrtc.cpp	
	)		
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}
	${PROJECT_SOURCE_DIR}/include
	${PROJECT_SOURCE_DIR}/../lib/${CMAKE_ToolChain}/libnice_0.1.16/include/nice
	${PROJECT_SOURCE_DIR}/../lib/${CMAKE_ToolChain}/openssl-1.1.1d/include
	${PROJECT_SOURCE_DIR}/../lib/${CMAKE_ToolChain}/libsrtp_v2.3.0/include/srtp2
	${PROJECT_SOURCE_DIR}/../lib/${CMAKE_ToolChain}/glib_2.48.0/include/glib-2.0
	${PROJECT_SOURCE_DIR}/third_src/cJSON-1.7.12
	${PROJECT_SOURCE_DIR}/third_src/usrsctp-0.9.3.0/usrsctplib
	)
ADD_LIBRARY(webrtc_static STATIC ${src_list})
SET_TARGET_PROPERTIES(webrtc_static PROPERTIES OUTPUT_NAME webrtc)


SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build/lib)

#TARGET_LINK_LIBRARIES(webrtc pthread -lm)

#TARGET_LINK_LIBRARIES(crnode log4cpp thrift)










