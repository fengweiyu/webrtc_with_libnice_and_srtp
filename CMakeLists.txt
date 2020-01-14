cmake_minimum_required (VERSION 2.8)

SET(CMAKE_TOOLCHAIN_FILE "build/ToolChain.cmake")
#SET(CMAKE_SYSTEM_NAME "Linux")
#SET(CMAKE_C_COMPILER "gcc")
#SET(CMAKE_CXX_COMPILER "g++")

project (WebRTC)
 
#SET(CMAKE_C_FLAGS "-g")
SET(CMAKE_CXX_FLAGS "-g")

SET(SRC_LIST
    src/conductor.cc
	src/defaults.cc
	src/peer_connection_client.cc",
	example/main.cc	
	)
		
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/src
	${PROJECT_SOURCE_DIR}/src/include
	)

ADD_EXECUTABLE(webrtc ${SRC_LIST})

TARGET_LINK_LIBRARIES(webrtc pthread -lm )
TARGET_LINK_LIBRARIES(webrtc ${PROJECT_SOURCE_DIR}/lib/libjingle_peerconnection_test_api.a)
TARGET_LINK_LIBRARIES(webrtc ${PROJECT_SOURCE_DIR}/lib/video_frame_api.a)
TARGET_LINK_LIBRARIES(webrtc ${PROJECT_SOURCE_DIR}/lib/rtc_base.a)
TARGET_LINK_LIBRARIES(webrtc ${PROJECT_SOURCE_DIR}/lib/rtc_base_approved.a)
TARGET_LINK_LIBRARIES(webrtc ${PROJECT_SOURCE_DIR}/lib/rtc_media.a)
TARGET_LINK_LIBRARIES(webrtc ${PROJECT_SOURCE_DIR}/lib/video_capture_module.a)
TARGET_LINK_LIBRARIES(webrtc ${PROJECT_SOURCE_DIR}/lib/libjingle_peerconnection.a)
TARGET_LINK_LIBRARIES(webrtc ${PROJECT_SOURCE_DIR}/lib/field_trial_default.a)
TARGET_LINK_LIBRARIES(webrtc ${PROJECT_SOURCE_DIR}/lib/metrics_default.a)



SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build/lib)
#TARGET_LINK_LIBRARIES(crnode log4cpp thrift)










