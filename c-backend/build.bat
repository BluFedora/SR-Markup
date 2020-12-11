
if not defined DevEnvDir (
  call D:\Dev\VS\2019_Preview\VC\Auxiliary\Build\vcvars64.bat
)

cl -nologo /Fo"build/" /LD /DLL /EHsc main.cpp bifrost_json.c /link /out:"bin/sr.GenBlog.dll"
move main.exp build\main.exp
move main.lib build\main.lib