
CC = clang

CFLAGS = -Wall -m64 -g -O0

#指定静态库.a文件路径
LIBRARY = liblshSbuf.a

#指定头文件搜索路径为 lib目录
INCLUDE_PATH = -IlshSbuf

#源文件目录
SRCDIR = ./

#所有源文件
SRCS = $(wildcard $(SRCDIR)/*.c)

OBJS = $(SRCS:%.c=%.o)

EXEC = main

#生成可执行文件
all: $(EXEC) 

$(LIBRARY): $(OBJS)
	ar rcs $@ $(OBJS)

#$^ 展开所有的.o 文件列表 $@表示所有目标文件路径即 $(EXEC)
$(EXEC): $(OBJS) $(LIBRARY)
	$(CC) $(CFLAGS) $^ -o $@ -L. -llshSbuf


%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ $(INCLUDE_PATH)
	
clean: 
	rm -rf *.o $(EXEC)