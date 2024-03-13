
CC = clang

CFLAGS = -Wall -m64 -g -O0

#指定静态库.a文件路径
LIBRARY = ../lshSbuf/liblshSbuf.a ../lib/liblsh.a

#指定头文件搜索路径为 lib目录
INCLUDE_PATH = -I../lshSbuf -I../lib

#源文件目录
SRCDIR = ./

#所有源文件
SRCS = $(wildcard $(SRCDIR)/*.c)

OBJS = $(SRCS:%.c=%.o)

EXEC = server

#生成可执行文件
all: $(EXEC) 

#$^ 展开所有的.o 文件列表 $@表示所有目标文件路径即 $(EXEC)
$(EXEC): $(OBJS) $(LIBRARY)
	$(CC) $(CFLAGS) $^ -o $@ -L../lshSbuf -llshSbuf -L../lib -llsh

#/%.o 是规则目标， $(SRCS)/%.c 规则的前提或者依赖  $(CC) $(CFLAGS)指定编译器和编译选项
#-c 表示告诉编译器生产对象文件，但不进行链接，编译阶段使用
#$< 表示编译的源文件路径即 $(SRCDIR)
#$@ 表示生产的.o文件路径即 
%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ $(INCLUDE_PATH)

clean: 
	rm -rf *.o $(EXEC)