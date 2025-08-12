@echo off

md _out 2> nul
md _out\x86 2> nul
md _out\x64 2> nul
md _out\x86\include 2> nul
md _out\x64\include 2> nul
md _out\x86\pdb 2> nul
md _out\x64\pdb 2> nul
md _tmp 2> nul

call build-libde265 x86
call build-libde265 x64
call build-kvazaar x86
call build-kvazaar x64
call build-libheif x86
call build-libheif x64

pause