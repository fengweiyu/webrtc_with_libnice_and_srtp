webrtcServer:
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