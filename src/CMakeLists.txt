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
    webrtc_interface.cpp	
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
SET_TARGET_PROPERTIES(webrtc_static PROPERTIES OUTPUT_NAME webrtc_tmp)


SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build/lib)

#必须DEPENDS webrtc_static，否则会先执行下面的语句，调整顺序也没用,必须ar x解析出来再打包，否则去掉-s参数打包出来也仅仅是索引不是完整的
add_custom_command(OUTPUT libwebrtc
    COMMAND ar -crsT ${PROJECT_SOURCE_DIR}/build/lib/libwebrtc.a ${PROJECT_SOURCE_DIR}/build/lib/libwebrtc_tmp.a
    ${PROJECT_SOURCE_DIR}/third_src/cJSON-1.7.12/build/lib/libcjson.a
    ${PROJECT_SOURCE_DIR}/../lib/${CMAKE_ToolChain}/libsrtp_v2.3.0/lib/libsrtp2.a
    ${PROJECT_SOURCE_DIR}/third_src/usrsctp-0.9.3.0/build/lib/libusrsctp.a
    ${PROJECT_SOURCE_DIR}/../lib/${CMAKE_ToolChain}/libnice_0.1.16/lib/libnice.a
    ${PROJECT_SOURCE_DIR}/../lib/${CMAKE_ToolChain}/openssl-1.1.1d/lib/libssl.a
    ${PROJECT_SOURCE_DIR}/../lib/${CMAKE_ToolChain}/openssl-1.1.1d/lib/libcrypto.a
    ${PROJECT_SOURCE_DIR}/../lib/${CMAKE_ToolChain}/glib_2.48.0/lib/libgio-2.0.a
    ${PROJECT_SOURCE_DIR}/../lib/${CMAKE_ToolChain}/glib_2.48.0/lib/libgobject-2.0.a
    ${PROJECT_SOURCE_DIR}/../lib/${CMAKE_ToolChain}/glib_2.48.0/lib/libgthread-2.0.a
    ${PROJECT_SOURCE_DIR}/../lib/${CMAKE_ToolChain}/glib_2.48.0/lib/libgmodule-2.0.a
    ${PROJECT_SOURCE_DIR}/../lib/${CMAKE_ToolChain}/glib_2.48.0/lib/libglib-2.0.a
    ${PROJECT_SOURCE_DIR}/../lib/${CMAKE_ToolChain}/libffi_v3.2.1/lib/libffi.a
    ${PROJECT_SOURCE_DIR}/../lib/${CMAKE_ToolChain}/zlib_v1.2.11/lib/libz.a
    DEPENDS webrtc_static)
add_custom_target(_merge ALL DEPENDS libwebrtc)



#TARGET_LINK_LIBRARIES(webrtc pthread -lm)

#TARGET_LINK_LIBRARIES(crnode log4cpp thrift)










