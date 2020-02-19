
1.编译：
	使用cmake进行编译，必须先安装cmake，安装后：
	ywf@ywf-pc:/work/workspace/webrtc_with_libnice_and_srtp$ ./build.sh x86
	编译成功后，会在如下路径生成webrtc应用程序：
	ywf@ywf-pc:/work/workspace/webrtc_with_libnice_and_srtp/build/x86$ ls webrtc
	webrtc


2.使用：
	./webrtc StunIP StunPort SelfName VideoFile
	eg:
	./webrtc 192.168.0.103 8888 ywf555 sintel.h264
	
******************************************************************************

1.compile
	before compile,must install cmake,then 
	eg:
	ywf@ywf-pc:/work/workspace/webrtc_with_libnice_and_srtp$ ./build.sh x86
	after make success:
	ywf@ywf-pc:/work/workspace/webrtc_with_libnice_and_srtp/build/x86$ ls webrtc
	webrtc
2.usage	
	./webrtc StunIP StunPort SelfName VideoFile
	eg:
	./webrtc 192.168.0.103 8888 ywf555 sintel.h264

	
******************************************************************************
目前实现的是客户端，并且是answer端
	
lib目录是已经编译(编译安装)好的
#第三方库安装目录要带版本号(lib目录下要，third_lib由于有源码所以不要)，后续优化
build 为已经编译好的，不想编译可直接使用



libnice只添加一路流nice_agent_add_stream，则设置远端sdp会报错：
(process:39447): libnice-CRITICAL **: More streams in SDP than in agent
LibniceSetRemoteSDP nice_agent_parse_remote_sdp fail 
这是由于远端SDP音频视频都有。
解决方法：
1.使用其他接口设置远端SDP如nice_agent_parse_remote_stream_sdp
2.事先添加两路流，只不过增加管理难度

目前是添加两路流，
并且只接收视频流的信息，
发也只发视频流
如果支持双路则应该要把依赖关系改成libnice依赖dtls即dtls是libnice的成员变量，底层就变成libnice和srtp，
而不是现在srtp,libnice,dtls三足鼎立的局面


libnice后续可以把音视频流做成map或list(特别是流类型比较多的时候)，
这样通过streamid进行分发
本层就可以不关心音频还是视频了交给上层管理

webrtc对方sdp中没有包含candidate,所以要组合offer和candidate两条消息


收集到本地信息再登录这样应该更安全，或者增加等待操作。

//m_pVideoHandle =NULL;//Init顺序在这之后，按道理可以去掉注释







