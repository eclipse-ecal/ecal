Please first adjust the CMAKE_PREFIX_PATH in `win_set_vars.bat` so that cmake can detect your QT installation, then:

If you want to build complete eCAL setup for windows then simply call. 

```bash
win_make_all.bat
```

If you want to cmake / build / setup eCAL step by step do the following steps:

```bash
win_make_cmake.bat
win_make_build.bat
win_make_setup.bat
```
