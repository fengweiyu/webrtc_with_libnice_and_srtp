﻿webrtcServer:
1.编译：
	使用cmake进行编译，必须先安装cmake，安装后(执行./build.sh x86 或者 ./build.sh x86 Server)：
	ywf@ywf-pc:/work/workspace/webrtc_with_libnice_and_srtp$ ./build.sh x86 
	编译成功后，会在如下路径生成webrtc应用程序：
	ywf@ywf-pc:/work/workspace/webrtc_with_libnice_and_srtp/build/x86$ ls webrtcServer
	webrtcServer


2.使用：
    1.启动webrtcServer服务(将example/WebDemo/H264G711A.flv拷贝到webrtcServer程序同目录)
        ./webrtcServer
    2.配置nginx代理(不使用https的url可不用配置，不过由于浏览器的安全机制http的url不支持对讲(浏览器对于http的链接无法获取麦克风或者摄像头的权限))
        使用nginx的https代理webrtcServer的http，可将doc/webrtc.conf导入到nginx配置文件中(webrtc.conf放到/etc/nginx/conf.d/目录下，同时在/etc/nginx/nginx.conf中添加一行 include /etc/nginx/conf.d/*.conf;)，
        并重启nginx(nginx -s reload),
        如果遇到跨域问题，可在 proxy_set_header和proxy_pass之间加入if ($request_method = 'OPTIONS') {...}这些配置即可
            proxy_set_header X-Real-IP $remote_addr;
            proxy_set_header X-Real-Port $remote_port;
            if ($request_method = 'OPTIONS') {
                add_header Access-Control-Allow-Headers 'access-control-allow-headers,accessol-allow-origin,content-type';
                #add_header Access-Control-Allow-Origin '*' ;
                add_header Access-Control-Allow-Methods 'GET, POST, OPTIONS';        
            }
            proxy_pass http://_server;
    3.启动浏览器页面
        使用浏览器打开example/WebDemo/index.html
    4.输入播放url或者对讲url
        播放，点击call出图，注意默认静音，需手动点击一下
        对讲，点击startTalk出流，注意webrtcServer接收到浏览器的声音会有打印输出"RecvData %p,iFrameLen %d \r\n"
    5.查看WebRTC运行状态
        直接在地址栏输入：chrome://webrtc-internals/

Webrtc协议不支持h265，但是谷歌浏览器支持H265的硬解(显卡交过专利费，谷歌不用交)，可以用webrtc中的sctp协议传输音视频数据(注意设置sctp数据类型为PPID_BINARY_LAST = 53,)，从而前端可解码
******************************************************************************
webrtcClient:
1.编译：
	使用cmake进行编译，必须先安装cmake，安装后：
	ywf@ywf-pc:/work/workspace/webrtc_with_libnice_and_srtp$ ./build.sh x86 client
	编译成功后，会在如下路径生成webrtc应用程序：
	ywf@ywf-pc:/work/workspace/webrtc_with_libnice_and_srtp/build/x86$ ls webrtcClient
	webrtcClient


2.使用：
    1.启动web服务器
        一般第一次启动后，后面就不需要再次启动，开机自动运行的。
        https://github.com/fengweiyu/webrtc_server/tree/master/Apache24/bin
        https://github.com/fengweiyu/webrtc_server/tree/master/webrtchtml
    2.启动信令服务器
        https://github.com/fengweiyu/webrtc_with_libnice_and_srtp/tree/master/doc/webrtcgateway32.exe或webrtcgateway64.exe
    3.启动offer端(StunIP StunPort 就是信令服务器webrtcgateway)，即推流端
        ./webrtc StunIP StunPort SelfName VideoFile offer/answer
        eg:
        ./webrtc 192.168.2.111 9898 ywf test.h264 offer
    4.启动谷歌浏览器，即拉流端
        输入第一步web服务器的ip地址访问网页，然后点击call出图
    5.查看WebRTC运行状态
        直接在地址栏输入：chrome://webrtc-internals/
    6.参考 https://www.cnblogs.com/yuweifeng/p/17578695.html

设置端口范围需要修改代码中宏定义LIBNICE_MIN_PORT_NUM 和 LIBNICE_MAX_PORT_NUM
******************************************************************************

1.compile
	before compile,must install cmake,then 
	eg:
	ywf@ywf-pc:/work/workspace/webrtc_with_libnice_and_srtp$ ./build.sh x86
	after make success:
	ywf@ywf-pc:/work/workspace/webrtc_with_libnice_and_srtp/build/x86$ ls webrtc
	webrtc
2.usage	
	./webrtc StunIP StunPort SelfName VideoFile offer/answer
	eg:
	./webrtc 192.168.0.103 9898 ywf555 sintel.h264 offer
 
	
******************************************************************************
需要和webrtc_server()配合使用

https://www.apachehaus.com/cgi-bin/download.plx



先说解决办法:   chrome://flags/#enable-webrtc-hide-local-ips-with-mdns 中将 Anonymize local IPs exposed by WebRTC. 
选项设置成disable 即可恢复原来ip地址， 不再使用 xxx.local地址.


httpd.conf
httpd-ahssl.conf
https://www.cnblogs.com/pdspkj/p/8044072.html
servertwo serverone



目前实现的是客户端，并且是answer端，作为answer必须对方offer支持h264
	
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


！！！！！！！！！！！！收集到本地信息再登录这样应该更安全，或者增加等待操作。
!!!!!!根据ready概率来选择sendanswer是在connecting之前还是之后


//m_pVideoHandle =NULL;//Init顺序在这之后，按道理可以去掉注释

//提前发回应然后再：
iRet=m_Libnice.SetRemoteCandidateAndSDP(acRemoteCandidate);//
结果还是：
SIGNAL: state changed 1 1 connecting[2]
SIGNAL: state changed 1 1 failed[5]  //抓包分析，对方回应错误码：487 Role conflict，对端关闭重启多次好，由controlling改为controlled无用
//先链接服务端(等收集到再另一个对端来启动)似乎不会failed
//可能SDP不完整，补充了所有candidate没用
//或者其他阿里云等待试试，webrtc例子是好的
//判断dtls包，加了

ICE-CONTROLLED和ICE-CONTROLLING
在每次会话中,每个终端都有一个身份,有两种身份,即受控方(controlled role)和主控方(controlling role).
主控方负责选择最终用来通讯的候选地址对,受控方被告知哪个候选地址对用来进行哪次媒体流传输,
并且不生成更新过的offer来提示此次告知.发起ICE处理进程(即生成offer)的一方必须是主控方,而另一方则是受控方.
如果终端是受控方,那么在request中就必须加上ICE-CONTROLLED属性,同样,如果终端是主控方,就需要ICE-CONTROLLING属性.
！！！！失败响应
如果STUN传输返回487(Role Conflict)错误响应,终端首先会检查其是否包含了ICE-CONTROLLED或ICE-CONTROLLING
属性.如果有ICE-CONTROLLED,终端必须切换为controlling role;如果请求包含ICE-CONTROLLING属性,
则必须切换为controlled role.切换好之后,终端必须使产生487错误的候选地址对进入检查队列中,
并将此地址对的状态设置为Waiting.
成功响应,一次连接检查在满足下列所有情况时候就被认为成功:
STUN传输产生一个Success Response
response的源IP和端口等于Binding Request的目的IP和端口
response的目的IP和端口等于Binding Request的源IP和端口

(peerconnection.cc:5750): The order of m-lines in answer doesn't match order in offer.
vp8试试
sdp是双向选择(offer表示我支持这些格式，answer就得在里面选，收发格式应该要一致)
answer信息不仅是自己支持(推流)的格式(还应该是对方支持的格式)

(port.cc:1297): Received conflicting role from the peer.
answer controlled ，offer controlling

:96 VP8/90000




Send :
POST /message?peer_id=8&to=0 HTTP/1.0
Content-Length:574
Content-Type:text/plain

{"sdp":"m=- 39240 ICE/SDP\n
c=IN IP4 192.168.0.105\n
a=ice-ufrag:F77M\n
a=ice-pwd:xIem6dTc1rcgJMt3QUzDcE\n
a=candidate:1 1 UDP 2013266431 fe80::20c:29ff:fe7e:5629 36651 typ host\n
a=candidate:2 1 TCP 1015022847 fe80::20c:29ff:fe7e:5629 9 typ host tcptype active\n
a=candidate:3 1 TCP 1010828543 fe80::20c:29ff:fe7e:5629 53316 typ host tcptype passive\n
a=candidate:4 1 UDP 2013266430 192.168.0.105 54379 typ host\n
a=candidate:5 1 TCP 1015022079 192.168.0.105 9 typ host tcptype active\n
a=candidate:6 1 TCP 1010827775 192.168.0.105 39240 typ host tcptype passive\n
","type":"answer"}


ready才能发送报文？
出现ready但是协商还是收不到报文，


抓包分析
自己已经发出，抓包出来，对方没回serverhello
webrtc是有dtls hello的
DTLSv1_2_method无用


打包为webrtc_v1.0_(android)-----已备份，重命名u盘，经验证风险低所以可以，
测试gn win结果ok  ，测试ok
由于不影响源码不影响其他编译结果，只是在out多个h264,所以可备份，打包为webrtc_v2.0_win
打包备份解压，把win工程拷贝出来到本地---------（搞好sln，再次备份进去，原始数据就无所谓了）
(由于压缩包存在，所以删除文件夹webrtc_v2.0_win，，再解压一个出来，看大小看h264(应该有))
查看本地是否有h264,不大的话可以保留 ，有h264开机62g比原先57g多5g，关机60g比原先(v1.0)








offer  端：
登录后选择对方id
不断post sdp给对方(先post sdp后不断post candidate) ，直到对方回answer(可能要读第二次才读到，RecvBody)
然后get请求对方candidate

answer端：
登录后
不断get请求对方的offer消息
成功后，不断get请求对方的candidate消息
成功后，post answer 消息


一种是一个类包含所有函数，一种是采用两个类即两个子类，两个类更清晰也方便使用
信令端：peer 里面放 offer之类的处理，并里面有两个子类，signal对其进行封装
处理端：webrtc类放两个子类，HandleMsg GetMsg 分别对应子类的不同处理方式



offer端错误：
不回复answer信息
(peerconnection.cc:5345): UseCandidatesInSessionDescription: Not ready to use candidate.
(peerconnection.cc:4762): Non-rejected SCTP m= section is needed to get the SSL Role of the SCTP transport.




libnice会创建多个通道，分别用于视频，音频，数据传输，创建时会产生一个通道id，一般是1开始然后递增，
同时sdp信息里对于每一个通道数据都有一个mid属性，一般是0开始递增，
由于都是递增的，所以就可以使用对应关系来传递指定类型的数据。如：
第一路通道会传输mid等于0的数据，第二路数据会传输mid等于1的数据，以此类推



offer端错误：
webrtc demo在libnice还没有ready的时候就发出client hello，这时候程序是没法回应的，回应是需要收到answer的，但是webrtc不知为何没发，
所以逻辑走不通。为何不回answer?日志看不出来 对比正确answer日志


answer端错误：
(peer_connection.cc:6820): The order of m-lines in answer doesn't match order in offer. Rejecting answer. (INVALID_PARAMETER)
(peer_connection.cc:2646): Failed to set remote answer sdp: The order of m-lines in answer doesn't match order in offer. Rejecting answer.
//可能是顺序不一致，m=video要在后面，m=audio要在前面
虚拟机不支持视频没有camera，可以虚拟摄像头

编译openssl，拷贝编译结果不要拷贝share目录，文件太多也没用
试试私有静态成员函数是否能被直接访问