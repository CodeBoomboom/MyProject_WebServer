CXX = g++
#g++：编译方式为C++，若是C语言则为gcc
CFLAGS = -std=c++14 -O2 -g -pthread -lmysqlclient
#-std=c++14：指定c++库版本为c++14
#-O2：编译优化参数，常见的-O0(不启用优化)，-O2/-O3(全局优化)
#-Wall：输出警告信息
#-g：带调试信息
#-pthread：使用线程库
#-lmysqlclient：使用sql客户端相关的库

TARGET = My_Webserver

OBJS = $(wildcard ../Code/Log/*.cpp ../Code/Pool/*.cpp ../Code/Timer/*.cpp \
				../Code/Http/*.cpp ../Code/Server/*.cpp ../Code/Wrap/*.cpp \
				../Code/Buffer/*.cpp ../Code/main.cpp)#匹配相关目录下的所有.cpp文件

ALL:$(OBJS)
	$(CXX) $^ -o ../bin/$(TARGET)  $(CFLAGS) 
#等价于：	$(CXX) $(OBJS) -o ../bin/$(TARGET) $(CFLAGS)
#注意：$(TARGET)不能用$@代替，否则会直接认为最终目标名为ALL，而不是My_Webserver

clean:
	rm -rf ../bin/$(OBJS) $(TARGET)
