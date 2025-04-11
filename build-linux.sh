#!/usr/bin/bash

# To run this you need to setup msvc for wine https://github.com/mstorsjo/msvc-wine
# I haven't setup the .clangd file yet so it's gonna be a pain to program in if you're using clangd tho

msbuild "./Project Reboot 3.0/Project Reboot 3.0.vcxproj" /p:configuration=Release /p:platform=x64 /p:OutDir=../Build
