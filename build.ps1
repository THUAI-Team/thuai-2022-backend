#!/usr/bin/env pwsh

mkdir $PSScriptRoot/bin
Set-Location $PSScriptRoot/bin

if ($CI) {
  cmake -DCMAKE_BUILD_TYPE=Release $PSScriptRoot
  cmake --build . --config Release
} else {
  cmake $PSScriptRoot
  cmake --build . 
}
