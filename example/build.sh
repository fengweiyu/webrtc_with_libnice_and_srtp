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
function BuildLib()
{
	echo -e "Start building ..."
	OutputPath="./build"
	if [ -e "$OutputPath" ]; then
		if [ -e "$OutputPath/lib" ]; then
			echo "/build/lib exit"
		else
			mkdir $OutputPath/lib
		fi
	else
		mkdir $OutputPath
		mkdir $OutputPath/lib
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
	if [ -e "lib" ]; then
		echo "lib exit"
	else
		mkdir lib
	fi
	
	cd lib
	if [ -e "webrtc" ]; then
		echo "webrtc exit"
	else
		mkdir webrtc
	fi
	
	cd webrtc
	if [ -e "common_video" ]; then
		echo "common_video exit"
	else
		mkdir common_video
	fi
	
	cd common_video
	cp $CurPwd/build/lib/libaudio_encoder_interface.a .
	cp $CurPwd/build/lib/libvideo_frame_api.a .
	cp $CurPwd/build/lib/libaudio_codecs_api.a .
	cp $CurPwd/build/lib/libbuiltin_audio_decoder_factory.a .
#	cp $CurPwd/build/lib/libvideo_frame_api.a .
}

if [ $# == 0 ]; then
	PrintUsage
	exit -1
else
	GenerateCmakeFile $1
	BuildLib
	CopyLib ../../../build
fi




