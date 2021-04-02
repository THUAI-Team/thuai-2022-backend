#!/bin/bash

basedir=$(realpath $(dirname $BASH_SOURCE))

mkdir $basedir/bin
cd $basedir/bin

cmake -DLOCAL=ON $basedir 
cmake --build . 

