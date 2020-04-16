If you want to build complete eCAL setup for windows then simply call. The default Visual Studio Version is 2019.

win_make_all.bat

-----------------------------------------------------------------------------------------------------

If you want to cmake / build eCAL step by step do the following steps

for Visual Studio 2015:

    win_make_cmake.bat v140
    win_make_build.bat

for Visual Studio 2017:

    win_make_cmake.bat v141
    win_make_build.bat

for Visual Studio 2019:

    win_make_cmake.bat v142
    win_make_build.bat

To create a windows setup finally call:

    win_make_setup.bat
