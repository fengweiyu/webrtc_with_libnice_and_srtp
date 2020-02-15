#!/bin/bash

function PrintUsage()
{
	echo -e "Usage:"
	echo -e "./build.sh $ToolChain"
	echo -e "ToolChain: arm-linux/x86"
	echo -e "EGG:"
	echo -e "./build.sh arm-linux"
	echo -e " or ./build.sh x86"
}
function GenerateCmakeFile()
{
	mkdir -p build
	CmakeFile="./build/ToolChain.cmake"
	echo "SET(CMAKE_SYSTEM_NAME \"Linux\")" > $CmakeFile
	if [ $1 == x86 ]; then
		echo "SET(CMAKE_C_COMPILER \"gcc\")" >> $CmakeFile	
		echo "SET(CMAKE_CXX_COMPILER \"g++\")" >> $CmakeFile	
	else
		echo "SET(CMAKE_C_COMPILER \"$1-gcc\")" >> $CmakeFile
		echo "SET(CMAKE_CXX_COMPILER \"$1-g++\")" >> $CmakeFile
	fi
}
function Build()
{
	echo -e "Start building ..."
	OutputPath="./build"
	if [ -e "$OutputPath" ]; then
		echo "/build exit"
	else
		mkdir $OutputPath
	fi	
	cd build
	cmake ..
	if [ -e "Makefile" ]; then	
		make -j4 > /dev/null
		if [ $? == 0 ]; then
			echo "make success! "
		else
			echo "make failed! "
			exit -1
		fi
	else
		echo "Makefile generated failed! "
		exit -1
	fi
	cd ..
}

function CopyLib()
{
	CurPwd = $PWD
	cd $1
	cp $CurPwd/build/webrtc .
#拷贝固定早就编译好的第三方so库
#暂时没时间优化直接使用所有编译结果也可以，libnice依赖架构，所以拷贝所有的，不调整架构
	cp $CurPwd/../lib/glib/* ./third_lib/glib/
	cp $CurPwd/../lib/glib/* ./third_lib/glib/
	cp $CurPwd/../lib/glib/* ./third_lib/glib/
	
#拷贝目的保持和源码编译结果一样,后续源码编译就可以不用从lib拷贝(直接删除如下语句)
}

if [ $# == 0 ]; then
	PrintUsage
	exit -1
else

	cd http
	sh build.sh $1
	if [ $? -ne 0 ]; then
		exit -1
	fi
	cd ..
	
	cd peerconnection_client
	sh build.sh $1
	if [ $? -ne 0 ]; then
		exit -1
	fi
	cd ..

	cd rtp
	sh build.sh $1
	if [ $? -ne 0 ]; then
		exit -1
	fi
	cd ..	
		
	GenerateCmakeFile $1
	Build
	CopyLib ../build $1
fi



