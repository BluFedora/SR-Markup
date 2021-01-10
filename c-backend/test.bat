call build
sr-blog-gen.exe -l bin\\sr.GenBlog.dll -i TestInput.txt > TestOutput.txt 2> Error.txt
type TestOutput.txt