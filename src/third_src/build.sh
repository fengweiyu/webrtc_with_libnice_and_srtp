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

function BuildcJSON()
{
	echo -e "Start building cJSON-1.7.12..."

	cd cJSON-1.7.12
	cmake .
	if [ -e "Makefile" ]; then	
		make clean
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
	CurPwd = $PWD
	cd ..
	
	cd $1
	if [ -e "third_lib" ]; then
		echo "third_lib exit"
	else
		mkdir third_lib
	fi
	
	cd third_lib
	if [ -e "cJSON" ]; then
		echo "cJSON exit"
	else
		mkdir cJSON
	fi
	
	cd cJSON
	cp $CurPwd/libcjson.so .
#	cp $CurPwd/cJSON.h .
	
}



if [ $# == 0 ]; then
	PrintUsage
	exit -1
else
#	BuildcJSON ../../build
	cp cJSON_build.sh ./cJSON-1.7.12/build.sh
	cp cJSON_CMakeLists.txt ./cJSON-1.7.12/CMakeLists.txt
	cd cJSON-1.7.12
	sh build.sh $1
	if [ $? -ne 0 ]; then
		exit -1
	fi
	cd ..	
	
fi




