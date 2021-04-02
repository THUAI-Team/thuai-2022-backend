#!/usr/bin/env pwsh

mkdir $PSScriptRoot/bin
Set-Location $PSScriptRoot/bin

cmake -DLOCAL=ON $PSScriptRoot
cmake --build . 
