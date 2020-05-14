#!/bin/bash

function PrintUsage()
{
	echo -e "Usage:"
	echo -e "./build.sh $ToolChain"
	echo -e "ToolChain: arm-hisiv200-linux/x86"
	echo -e "EGG:"
	echo -e "./build.sh arm-hisiv200-linux"
	echo -e " or ./build.sh x86"
}


if [ $# == 0 ]; then
	PrintUsage
	exit -1
else
	OutputPath="./build"
#rm glib-2.48.0 -rf
#tar -xzf glib_2.48.0.tar.gz 
	if [ -e "$OutputPath" ]; then
		if [ -e "$OutputPath/$1" ]; then
			echo "/build/$1 exit"
		else
			mkdir $OutputPath/$1			
		fi
		
	else
		mkdir $OutputPath
		mkdir $OutputPath/$1
	fi
	
	cd zlib-1.2.11
	sh build.sh $1
	if [ $? -ne 0 ]; then
		exit -1
	fi
	cd ..
	
	cd libffi-3.2.1
	sh build.sh $1
	if [ $? -ne 0 ]; then
		exit -1
	fi
	cd ..
	
	cd glib-2.48.0
	sh build.sh $1
	if [ $? -ne 0 ]; then
		exit -1
	fi
	cd ..
	
	cd openssl-1.1.1d
	sh build.sh $1
	if [ $? -ne 0 ]; then
		exit -1
	fi
	cd ..
	
	cd libnice-0.1.16
	sh build.sh $1
	if [ $? -ne 0 ]; then
		exit -1
	fi
	cd ..
	
	
fi




