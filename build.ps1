#!/usr/bin/env pwsh

mkdir $PSScriptRoot/build
Set-Location $PSScriptRoot/build

cmake $PSScriptRoot
cmake --build .
