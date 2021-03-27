#!/bin/bash

basedir=$(realpath $(dirname $BASH_SOURCE))

mkdir $basedir/bin
cd $basedir/bin

if [ $CI ]; then
    cmake $basedir -DCMAKE_BUILD_TYPE=Release
    cmake --build . -config Release
else
    cmake $basedir 
    cmake --build . 
fi

