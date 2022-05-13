ALL:
	mkdir -p bin
	cd Build && make
#当前目录下创建bin文件夹，-p的意思是递归创建；然后切换到build目录下并执行make

