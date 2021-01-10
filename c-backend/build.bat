
if not defined DevEnvDir (
  call D:\Dev\VS\2019_Preview\VC\Auxiliary\Build\vcvars64.bat
)

cl -nologo /Od /MDd /std:c++17 /Fo"build/" /LD /DLL /EHsc main.cpp /link BF_JsonCpp.lib /out:"bin/sr.GenBlog.dll"
move main.exp build\main.exp
move main.lib build\main.lib