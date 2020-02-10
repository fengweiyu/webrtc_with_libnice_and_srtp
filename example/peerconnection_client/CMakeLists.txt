cmake_minimum_required (VERSION 2.8)

SET(CMAKE_TOOLCHAIN_FILE "build/ToolChain.cmake")
#SET(CMAKE_SYSTEM_NAME "Linux")
#SET(CMAKE_C_COMPILER "gcc")
#SET(CMAKE_CXX_COMPILER "g++")


add_definitions(-std=c++11)
add_definitions(-DWEBRTC_LINUX)
# -fno-rtti 禁止运行时类型信息 不加会出现某些包含纯虚函数的类 undefine reference 暂时也没搞懂原因
add_definitions(-D -fno-rtti)
project (WebRTC)
 
#SET(CMAKE_C_FLAGS "-g")
SET(CMAKE_CXX_FLAGS "-g -DWEBRTC_POSIX")

SET(common_video_list
    bitrate_adjuster.cc
    h264/h264_bitstream_parser.cc
    h264/h264_common.cc
    h264/pps_parser.cc
    h264/profile_level_id.cc
    h264/sps_parser.cc
    h264/sps_vui_rewriter.cc
    i420_buffer_pool.cc
    incoming_video_stream.cc
    libyuv/webrtc_libyuv.cc
    video_frame.cc
    video_frame_buffer.cc
    video_render_frames.cc
	)		
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}
	${PROJECT_SOURCE_DIR}/../../
	../base
	)
ADD_LIBRARY(common_video_static STATIC ${common_video_list})
SET_TARGET_PROPERTIES(common_video_static PROPERTIES OUTPUT_NAME common_video)

















SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build/lib)

#TARGET_LINK_LIBRARIES(webrtc pthread -lm)

#TARGET_LINK_LIBRARIES(crnode log4cpp thrift)










