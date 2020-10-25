call D:\Dev\VS\2019_Preview\VC\Auxiliary\Build\vcvars64.bat
cl -nologo /Fo"build/" /LD /DLL /EHsc main.cpp /link /out:"bin/sr.GenBlog.dll"
move main.exp build\main.exp
move main.lib build\main.lib