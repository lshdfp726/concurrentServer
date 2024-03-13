#ifndef LSHIO_H
#define LSHIO_H

#include <unistd.h>
#include <sys/stat.h>

//在函数栈里面开辟内存一定要注意别把栈帧撑爆了导致段错误
#define RIO_BUFSIZE (1024 * 8)

typedef struct {
    int rio_fd;  //文件句柄
    int rio_cnt; //剩余未读字节数
    char *rio_bufptr; //指向下个未读的字节
    char rio_buf[RIO_BUFSIZE]; //缓存buffer
} lshRio_t;

/**
 * @brief 不带缓存读
 * 
 * @param fd 文件句柄(文件描述符)
 * @param buf IO读操作写入的内存
 * @param n 需要读的字节数
 * @return ssize_t 返回剩余未读字节或错误
 */
ssize_t lsh_readn(int fd, void *buf, size_t n);

/**
 * @brief 不带缓存的写
 * 
 * @param fd 文件句柄(文件描述符)
 * @param buf IO写操作的内存
 * @param n 需要写的字节数
 * @return ssize_t 返回 n, 或者错误信息。不会返回还剩余未写入的值
 */
ssize_t lsh_writen(int fd, void *buf, size_t n);

void lshRio_readinitb(lshRio_t *rp, int fd);

/**
 * @brief 带缓存的读行
 * 
 * @param rp 缓存结构体
 * @param buf IO读操作写入的内存
 * @param maxlen 读一行循环操作的最大次数
 * @return ssize_t 返回实际读了多少个字节数据
 */
ssize_t lshc_readline(lshRio_t *rp, void *buf, size_t maxlen);

/**
 * @brief 
 * 
 * @param rp 缓存结构体
 * @param buf IO读操作写入的内存
 * @param n  需要读的字节数
 * @return ssize_t 
 */
ssize_t lshc_read(lshRio_t *rp, void *buf, size_t n);

int fstatcheck(int fd, struct stat *buf);

/**
 * @brief 
 * 多线程安全的输入字符串到标准输出
 * 
 * @param s 输入的字符串
 * @return ssize_t 返回输入的字节
 */
ssize_t lshSio_puts(char s[]);

/**
 * @brief 
 * 多线程安全的输入long 数值 串到标准输出
 * @param v 输入的整型
 * @return ssize_t 返回输入的字节
 */
ssize_t lshSio_putl(long v);

/**
 * @brief 
 * 多线程安全的输入错误信息到标准输出，然后_exit(1)
 * @param s 输入的字符串
 */
void lshSio_error(char s[]);

/**
 * @brief 
 * 打印完错误就exit(0);
 * @param msg 和strerror 一起打印的自定义字符串
 */
void lshUnix_error(char *msg);
#endif