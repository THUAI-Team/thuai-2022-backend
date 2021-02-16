#!/bin/bash

basedir=$(realpath $(dirname $BASH_SOURCE))

mkdir $basedir/../build
cd $basedir/../build

cmake $basedir
cmake --build .
