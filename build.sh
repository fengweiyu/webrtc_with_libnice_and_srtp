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
#build目录是给main执行程序编译用的，同时也是直接提供库，类似make install
#所以内部编译的原则是main的编译脚本(库的使用者)才会依赖build里面的东西，其他模块的编译脚本是不依赖的	
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
		if [ -e "$OutputPath/third_lib" ]; then
			echo "/build/third_lib exit"
		else
			mkdir $OutputPath/third_lib
		fi				
	else
		mkdir $OutputPath
		mkdir $OutputPath/include
		mkdir $OutputPath/lib
		mkdir $OutputPath/third_lib
	fi
	
	cd src
	sh build.sh $1
	if [ $? -ne 0]; then
		exit -1
	fi
	cd ..
	
	cd example
	sh build.sh $1
	if [ $? -ne 0]; then
		exit -1
	fi
	cd ..
	
fi




