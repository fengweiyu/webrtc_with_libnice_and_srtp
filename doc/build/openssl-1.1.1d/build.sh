#!/bin/bash

function PrintUsage()
{
	echo -e "Usage:"
	echo -e "./build.sh $ToolChain"
	echo -e "ToolChain: arm-himix200-linux/x86"
	echo -e "EGG:"
	echo -e "./build.sh arm-himix200-linux"
	echo -e " or ./build.sh x86"
}
function GenerateCmakeFile()
{
#	mkdir -p build
	CmakeFile="$2/ToolChain.cmake"
	echo "SET(CMAKE_SYSTEM_NAME \"Linux\")" > $CmakeFile
	if [ $1 == x86 ]; then
		echo "SET(CMAKE_C_COMPILER \"gcc\")" >> $CmakeFile	
		echo "SET(CMAKE_CXX_COMPILER \"g++\")" >> $CmakeFile		
	else
		echo "SET(CMAKE_C_COMPILER \"$1-gcc\")" >> $CmakeFile
		echo "SET(CMAKE_CXX_COMPILER \"$1-g++\")" >> $CmakeFile
		
	fi
	echo "SET(CMAKE_ToolChain \"$1\")" >> $CmakeFile
}
function BuildLib()
{
	echo -e "Start building openssl-1.1.1d ..."
	OutputPath="./build"
	if [ -e "$OutputPath" ]; then
		rm $OutputPath -rf
	fi	
	mkdir $OutputPath
	mkdir $OutputPath/install
	
    ./Configure shared --prefix=$PWD/build/install --cross-compile-prefix=$1- linux-elf no-asm
	if [ -e "Makefile" ]; then	
		make clean
#		make -j4 > /dev/null
        make -j4 
		if [ $? == 0 ]; then
			echo "make success! "
		else
			echo "make failed! "
			exit -1
		fi
		make install
	else
		echo "Makefile generated failed! "
		exit -1
	fi
#	cd ..
}

function CopyLib()
{
#	CurPwd = $PWD
	CurPwd=$PWD
	cd $1
	
	LibName="openssl-1.1.1d"
	if [ -e $LibName ]; then
		echo "\"$LibName\" exit"
	else
		mkdir $LibName
	fi	
	cd $LibName
	
	
#	cp $CurPwd/build/install/lib/*.a .
#	cp $CurPwd/build/install/lib/libffi-3.2.1/include . -rf

	cp $CurPwd/build/install/lib . -rf
	cp $CurPwd/build/install/include . -rf
}

if [ $# == 0 ]; then
	PrintUsage
	exit -1
else
	BuildLib $1
	CopyLib ../build/$1
fi




