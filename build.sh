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


if [ $# == 0 ]; then
	PrintUsage
	exit -1
else
	OutputPath="./build"
	if [ -e "$OutputPath" ]; then
		if [ -e "$OutputPath/include" ]; then
			echo "/build/include exit"
		else
			mkdir $OutputPath/include
		fi
		if [ -e "$OutputPath/lib" ]; then
			echo "/build/lib exit"
		else
			mkdir $OutputPath/lib
		fi
	else
		mkdir $OutputPath
		mkdir $OutputPath/include
		mkdir $OutputPath/lib
	fi
	
	cd src/webrtc
	sh build.sh $1
	if [ $? -ne 0]; then
		exit -1
	fi
	cd ..
	
	cd ..
	GenerateCmakeFile $1
	BuildLib
	CopyLib ../../../build
	
fi




