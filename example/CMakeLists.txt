cmake_minimum_required (VERSION 2.8)

SET(CMAKE_TOOLCHAIN_FILE "build/ToolChain.cmake")
#SET(CMAKE_SYSTEM_NAME "Linux")
#SET(CMAKE_C_COMPILER "gcc")
#SET(CMAKE_CXX_COMPILER "g++")

project (WebRTC)
 
#SET(CMAKE_C_FLAGS "-g")
SET(CMAKE_CXX_FLAGS "-g")

if(${CMAKE_ToolChain} STREQUAL "")

else()

endif()

SET(SRC_LIST
    rtp_interface.cpp
	signalling_interface.cpp
	main.cpp	
	)
		
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/../build/${CMAKE_ToolChain}/lib/include
#webrtc.h又依赖其内部头文件	
	${PROJECT_SOURCE_DIR}/../build/${CMAKE_ToolChain}/lib
	${PROJECT_SOURCE_DIR}/../build/${CMAKE_ToolChain}/peerconnection_client/include
#peerconnection_client中又依赖http	
	${PROJECT_SOURCE_DIR}/../build/${CMAKE_ToolChain}/http/include
	${PROJECT_SOURCE_DIR}/../build/${CMAKE_ToolChain}/rtp/include
#rtp.h又依赖其内部头文件		
	${PROJECT_SOURCE_DIR}/../build/${CMAKE_ToolChain}/rtp
#	${PROJECT_SOURCE_DIR}/../build/${CMAKE_ToolChain}/third_lib/glib/include
#.h中又依赖
	${PROJECT_SOURCE_DIR}/../build/${CMAKE_ToolChain}/third_lib/libnice/include/nice
	${PROJECT_SOURCE_DIR}/../build/${CMAKE_ToolChain}/third_lib/glib/include/glib-2.0
	${PROJECT_SOURCE_DIR}/../build/${CMAKE_ToolChain}/third_lib/libsrtp/include/srtp2
	${PROJECT_SOURCE_DIR}/../build/${CMAKE_ToolChain}/third_lib/openssl/include
	)

ADD_EXECUTABLE(webrtc ${SRC_LIST})

TARGET_LINK_LIBRARIES(webrtc ${PROJECT_SOURCE_DIR}/../build/${CMAKE_ToolChain}/lib/libwebrtc.a)
TARGET_LINK_LIBRARIES(webrtc ${PROJECT_SOURCE_DIR}/../build/${CMAKE_ToolChain}/peerconnection_client/libpeerconnection_client.a)
TARGET_LINK_LIBRARIES(webrtc ${PROJECT_SOURCE_DIR}/../build/${CMAKE_ToolChain}/rtp/librtp.a)
TARGET_LINK_LIBRARIES(webrtc ${PROJECT_SOURCE_DIR}/../build/${CMAKE_ToolChain}/http/libhttp.a)

#第三方库
TARGET_LINK_LIBRARIES(webrtc ${PROJECT_SOURCE_DIR}/../build/${CMAKE_ToolChain}/third_lib/cJSON/libcjson.a)
TARGET_LINK_LIBRARIES(webrtc ${PROJECT_SOURCE_DIR}/../build/${CMAKE_ToolChain}/third_lib/libsrtp/lib/libsrtp2.a)
#so
#//初始脚本ln -s到libglib-2.0.so快捷方式操作，或者直接拷贝成libglib-2.0.so
TARGET_LINK_LIBRARIES(webrtc ${PROJECT_SOURCE_DIR}/../build/${CMAKE_ToolChain}/third_lib/libnice/lib/libnice.a)

TARGET_LINK_LIBRARIES(webrtc ${PROJECT_SOURCE_DIR}/../build/${CMAKE_ToolChain}/third_lib/openssl/lib/libssl.a)
TARGET_LINK_LIBRARIES(webrtc ${PROJECT_SOURCE_DIR}/../build/${CMAKE_ToolChain}/third_lib/openssl/lib/libcrypto.a)

TARGET_LINK_LIBRARIES(webrtc ${PROJECT_SOURCE_DIR}/../build/${CMAKE_ToolChain}/third_lib/glib/lib/libgio-2.0.a)
TARGET_LINK_LIBRARIES(webrtc ${PROJECT_SOURCE_DIR}/../build/${CMAKE_ToolChain}/third_lib/glib/lib/libgobject-2.0.a)
TARGET_LINK_LIBRARIES(webrtc ${PROJECT_SOURCE_DIR}/../build/${CMAKE_ToolChain}/third_lib/glib/lib/libgthread-2.0.a)
TARGET_LINK_LIBRARIES(webrtc ${PROJECT_SOURCE_DIR}/../build/${CMAKE_ToolChain}/third_lib/glib/lib/libgmodule-2.0.a)
#glib是依赖的底层，放在后面
TARGET_LINK_LIBRARIES(webrtc ${PROJECT_SOURCE_DIR}/../build/${CMAKE_ToolChain}/third_lib/glib/lib/libglib-2.0.a)
TARGET_LINK_LIBRARIES(webrtc ${PROJECT_SOURCE_DIR}/../build/${CMAKE_ToolChain}/third_lib/libffi/lib/libffi.a)
TARGET_LINK_LIBRARIES(webrtc ${PROJECT_SOURCE_DIR}/../build/${CMAKE_ToolChain}/third_lib/zlib/lib/libz.a)
#TARGET_LINK_LIBRARIES(webrtc ${PROJECT_SOURCE_DIR}/../build/${CMAKE_ToolChain}/third_lib/glib/libglib-2.0.so) #优化后才这个路径

TARGET_LINK_LIBRARIES(webrtc -pthread -lm -lrt -ldl -lresolv)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build)
#TARGET_LINK_LIBRARIES(crnode log4cpp thrift)










