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
	CmakeFile="linux.cache"
	echo "glib_cv_long_long_format=ll" > $CmakeFile
	echo "glib_cv_stack_grows=no" >> $CmakeFile
	echo "glib_cv_have_strlcpy=no" >> $CmakeFile
	echo "glib_cv_have_qsort_r=yes" >> $CmakeFile
	echo "glib_cv_va_val_copy=yes" >> $CmakeFile
	echo "glib_cv_uscore=no" >> $CmakeFile
	echo "glib_cv_rtldglobal_broken=no" >> $CmakeFile
	echo "ac_cv_func_posix_getpwuid_r=yes" >> $CmakeFile
	echo "ac_cv_func_posix_getgrgid_r=yes" >> $CmakeFile
	
}
function BuildLib()
{
	echo -e "Start building glib-2.48.0 ..."
	OutputPath="./build"
	if [ -e "$OutputPath" ]; then
		echo "$OutputPath exit"
	else
		mkdir $OutputPath			
	fi	
	if [ -e "$OutputPath/install" ]; then
		echo "$OutputPath/install exit"
	else
		mkdir $OutputPath/install			
	fi
	
	cd $2
	CurPwd=$PWD
	cd -
	make distclean
	rm linux.cache
	./autogen.sh
	GenerateCmakeFile $1 $OutputPath
	./configure --prefix=$PWD/build/install --host=$1 CC=$1-gcc \
--cache-file=linux.cache --enable-libmount=no --with-pcre=internal --disable-selinux  --disable-xattr --disable-libelf --enable-static \
LIBFFI_CFLAGS=-I$CurPwd/libffi_v3.2.1/lib/libffi-3.2.1/include LIBFFI_LIBS="-L\"$CurPwd/libffi_v3.2.1/lib/\" -lffi" LIBFFI_LDFLAGS=-L$CurPwd/libffi_v3.2.1/lib \
ZLIB_CFLAGS=-I$CurPwd/zlib_v1.2.11/include ZLIB_LIBS="-L\"$CurPwd/zlib_v1.2.11/lib/\" -lz" ZLIB_LDFLAGS=-L$CurPwd/zlib_v1.2.11/lib
	if [ -e "Makefile" ]; then	
#		make clean
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
}

function CopyLib()
{
#	CurPwd = $PWD
	CurPwd=$PWD
	cd $1
	LibName="glib_2.48.0"
	
	if [ -e $LibName ]; then
		echo "\"$LibName\" exit"
	else
		mkdir $LibName
	fi	
	cd $LibName
	
	
#	cp $CurPwd/build/install/lib/*.a .
#	cp $CurPwd/build/install/include . -rf

	cp $CurPwd/build/install/lib . -rf
	cp $CurPwd/build/install/include . -rf
}

if [ $# == 0 ]; then
	PrintUsage
	exit -1
else
	
#	GenerateCmakeFile $1
	BuildLib $1 ../build/$1
	CopyLib ../build/$1
fi




