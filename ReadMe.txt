
1.编译：
	使用cmake进行编译，必须先安装cmake，安装后：
	book@book-virtual-machine:/work/workspace/dhcp$ cmake .
	book@book-virtual-machine:/work/workspace/dhcp$ make clean;make

2.使用：
	sudo ./dhcp client eth0
	or
	sudo ./dhcp server eth0
	注意，使用server时必须先配置网卡ip，
	eg:
	sudo ifconfig eth0 192.168.1.1
	
******************************************************************************
1.compile
	before compile,must install cmake,then 
	eg:
	book@book-virtual-machine:/work/workspace/dhcp$ cmake .
	book@book-virtual-machine:/work/workspace/dhcp$ make clean;make
2.usage	
	sudo ./dhcp client eth0
	or
	sudo ./dhcp server eth0
	attention,before use dhcp server must config interface ip
	eg:
	sudo ifconfig eth0 192.168.1.1
	

******************************************************************************	
/work/workspace/WebRTC_Without_RTP/src/webrtc/base/../third_party/openssl/include/openssl/e_os2.h:13:34: fatal error: openssl/opensslconf.h: 没有那个文件或目录
 # include <openssl/opensslconf.h>
改源码需要拉分支，当前主干不需要依赖的openssl等库先安装。

	