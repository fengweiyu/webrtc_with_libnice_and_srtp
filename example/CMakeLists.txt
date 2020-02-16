cmake_minimum_required (VERSION 2.8)

SET(CMAKE_TOOLCHAIN_FILE "build/ToolChain.cmake")
#SET(CMAKE_SYSTEM_NAME "Linux")
#SET(CMAKE_C_COMPILER "gcc")
#SET(CMAKE_CXX_COMPILER "g++")

project (WebRTC)
 
#SET(CMAKE_C_FLAGS "-g")
SET(CMAKE_CXX_FLAGS "-g")

SET(SRC_LIST
    rtp_interface.cpp
	signalling_interface.cpp
	main.cpp	
	)
		
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/../build/lib/include
	${PROJECT_SOURCE_DIR}/../build/peerconnection_client/include
	${PROJECT_SOURCE_DIR}/../build/rtp/include
#	${PROJECT_SOURCE_DIR}/../build/third_lib/glib/include
	)

ADD_EXECUTABLE(webrtc ${SRC_LIST})

TARGET_LINK_LIBRARIES(webrtc pthread -lm )
TARGET_LINK_LIBRARIES(webrtc ${PROJECT_SOURCE_DIR}/../build/lib/libwebrtc.a)
TARGET_LINK_LIBRARIES(webrtc ${PROJECT_SOURCE_DIR}/../build/peerconnection_client/libpeerconnection_client.a)
TARGET_LINK_LIBRARIES(webrtc ${PROJECT_SOURCE_DIR}/../build/rtp/librtp.a)
TARGET_LINK_LIBRARIES(webrtc ${PROJECT_SOURCE_DIR}/../build/http/libhttp.a)

#第三方库
TARGET_LINK_LIBRARIES(webrtc ${PROJECT_SOURCE_DIR}/../build/third_lib/cJSON/libcjson.so)
TARGET_LINK_LIBRARIES(webrtc ${PROJECT_SOURCE_DIR}/../build/third_lib/libsrtp/lib/libsrtp2.a)
#so
#//初始脚本ln -s到libglib-2.0.so快捷方式操作，或者直接拷贝成libglib-2.0.so
TARGET_LINK_LIBRARIES(webrtc ${PROJECT_SOURCE_DIR}/../build/third_lib/libnice/lib/libnice.so)

TARGET_LINK_LIBRARIES(webrtc ${PROJECT_SOURCE_DIR}/../build/third_lib/openssl/lib/libssl.a)
TARGET_LINK_LIBRARIES(webrtc ${PROJECT_SOURCE_DIR}/../build/third_lib/openssl/lib/libcrypto.a)
TARGET_LINK_LIBRARIES(webrtc ${PROJECT_SOURCE_DIR}/../build/third_lib/zlib/lib/libz.a)
TARGET_LINK_LIBRARIES(webrtc ${PROJECT_SOURCE_DIR}/../build/third_lib/libffi/lib/libffi.a)

TARGET_LINK_LIBRARIES(webrtc ${PROJECT_SOURCE_DIR}/../build/third_lib/glib/lib/libglib-2.0.so)
TARGET_LINK_LIBRARIES(webrtc ${PROJECT_SOURCE_DIR}/../build/third_lib/glib/lib/libgio-2.0.so)
TARGET_LINK_LIBRARIES(webrtc ${PROJECT_SOURCE_DIR}/../build/third_lib/glib/lib/libgmodule-2.0.so)
TARGET_LINK_LIBRARIES(webrtc ${PROJECT_SOURCE_DIR}/../build/third_lib/glib/lib/libgobject-2.0.so)
TARGET_LINK_LIBRARIES(webrtc ${PROJECT_SOURCE_DIR}/../build/third_lib/glib/lib/libgthread-2.0.so)

#TARGET_LINK_LIBRARIES(webrtc ${PROJECT_SOURCE_DIR}/../build/third_lib/glib/libglib-2.0.so) #优化后才这个路径



SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build)
#TARGET_LINK_LIBRARIES(crnode log4cpp thrift)










