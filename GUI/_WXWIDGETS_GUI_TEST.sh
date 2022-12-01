#!/bin/sh

echo [*] Create test GUI wx-program for linux ...
`wx-config --cxx --cxxflags` -o wxWidgets_gui_test_linux *.cpp `wx-config --libs`

echo [*] Create test GUI wx-program for windows ...
x86_64-w64-mingw32-g++ *.cpp $(/usr/x86_64-w64-mingw32/bin/wx-config --cxxflags --libs) -o wxWidgets_gui_test_win64 -static
echo

echo [*] Run assembled linux program...
./wxWidgets_gui_test_linux &

echo [*] Run assembled win64 program...
wine ./wxWidgets_gui_test_win64.exe &
