#!/usr/bin/env pwsh

mkdir $PSScriptRoot/bin
Set-Location $PSScriptRoot/bin

cmake $PSScriptRoot
cmake --build .
