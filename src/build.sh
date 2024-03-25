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
#   mkdir -p build
    CmakeFile="$2/ToolChain.cmake"
    echo "SET(CMAKE_SYSTEM_NAME \"Linux\")" > $CmakeFile
    if [ $1 == x86 -o $1 == x64 ]; then
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
    echo -e "Start building src..."
    OutputPath="./build"
    if [ -e "$OutputPath" ]; then
        rm $OutputPath -rf
#防止切换平台编译时由于平台不对应报错，所以删除build重新建立       
#       if [ -e "$OutputPath/lib" ]; then
#           echo "/build/lib exit"
#       else
#           mkdir $OutputPath/lib
#       fi
#   else
#       mkdir $OutputPath
#       mkdir $OutputPath/lib
    fi  
    mkdir $OutputPath
    mkdir $OutputPath/lib
        
    GenerateCmakeFile $1 $OutputPath    
    cd $OutputPath
    cmake ..
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
    cd ..
}

function CopyLib()
{
#   CurPwd = $PWD
    CurPwd=$PWD
    cd $1
    if [ -e "lib" ]; then
        echo "lib exit"
    else
        mkdir lib
    fi
    
    cd lib
    #cp $CurPwd/build/lib/libwebrtc.a .
    
    rm tmp -rf
    mkdir tmp
    cd tmp
    ar x $CurPwd/build/lib/libwebrtc_tmp.a
#    ar x $CurPwd/third_src/cJSON-1.7.12/build/lib/libcjson.a //暂未使用，防止和外部的cjson库重复定义
#指定cd到指定目录，没法指定目录,如果放到同一个目录，同名文件就被替换了
    rm libsrtp -rf
    mkdir libsrtp
    cd libsrtp
    ar x $CurPwd/../lib/$2/libsrtp_v2.3.0/lib/libsrtp2.a
    cd ..
    ar x $CurPwd/third_src/usrsctp-0.9.3.0/build/lib/libusrsctp.a
    ar x $CurPwd/../lib/$2/libnice_0.1.16/lib/libnice.a
    ar x $CurPwd/../lib/$2/openssl-1.1.1d/lib/libssl.a
    ar x $CurPwd/../lib/$2/openssl-1.1.1d/lib/libcrypto.a
    ar x $CurPwd/../lib/$2/glib_2.48.0/lib/libgio-2.0.a
    ar x $CurPwd/../lib/$2/glib_2.48.0/lib/libgobject-2.0.a
    ar x $CurPwd/../lib/$2/glib_2.48.0/lib/libgthread-2.0.a
    ar x $CurPwd/../lib/$2/glib_2.48.0/lib/libgmodule-2.0.a
    ar x $CurPwd/../lib/$2/glib_2.48.0/lib/libglib-2.0.a
    ar x $CurPwd/../lib/$2/libffi_v3.2.1/lib/libffi.a
    ar x $CurPwd/../lib/$2/zlib_v1.2.11/lib/libz.a
#使用q参数代替r参数，r参数会把同名的.o替换，而q参数不会(-s参数打包出来也仅仅是索引不是完整的)
    ar -cqv libwebrtc.a ./*.o ./libsrtp/*.o
    ranlib libwebrtc.a
    cd ..
    cp ./tmp/libwebrtc.a .
    rm tmp -rf
    cp $CurPwd/include . -rf
#由于对外头文件又依赖内部头文件，所以要拷贝，暂时这么处理后续优化   
#   cp $CurPwd/*.h . 已优化

}

if [ $# == 0 ]; then
    PrintUsage
    exit -1
else
    cd third_src
    sh build.sh $1
    if [ $? -ne 0 ]; then
        exit -1
    fi
    cd ..
    
#   GenerateCmakeFile $1
    BuildLib $1
    CopyLib ../build/$1 $1
fi




