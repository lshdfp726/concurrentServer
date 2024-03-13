#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <libc.h>
#include <pthread.h>

#include "lshIO.h"

static ssize_t lshRio_read(lshRio_t *rp, char *buf, size_t n);

ssize_t lsh_readn(int fd, void *buf, size_t n) {
    size_t nleft = n;
    ssize_t nread;
    char *bufp = (char *)buf;
    // printf("start lsh_readn\n");
    while (nleft > 0)
    {
        if ((nread = read(fd, buf, nleft)) < 0) 
        {
            if (errno == EINTR) //系统中断打断
                nread = 0;
            else 
                return -1;
        }
        else if (nread == 0)
            break;
        
        nleft -= nread;
        bufp += nread;
    }
    return (n - nleft);
}   

ssize_t lsh_writen(int fd, void *buf, size_t n) {
    size_t nleft = n;
    ssize_t nwritten;
    char *bufp = (char *)buf;
    // printf("start lsh_writen\n");
    while (nleft > 0)
    {
        if ((nwritten = write(fd, bufp, nleft)) <= 0) 
        {
            if (errno == EINTR)
                nwritten = 0;
            else 
                return -1;  
        }

        nleft -= nwritten;
        bufp += nwritten;
    }
    return n;
}

void lshRio_readinitb(lshRio_t *rp, int fd) 
{
    printf("start lshRio_readinitb\n");
    rp->rio_fd = fd;
    rp->rio_cnt = 0;
    rp->rio_bufptr = rp->rio_buf;
}

ssize_t lshc_readline(lshRio_t *rp, void *buf, size_t maxlen)
{
    int n, rc;
    char c, *bufp = buf;
    // printf("start lshc_readline\n");
    for (n = 1; n < maxlen; n++)
    {
        if ((rc = lshRio_read(rp, &c, 1)) == 1) 
        {
            *bufp++ = c;
            if (c == '\n') {
                break;
            }
        } 
        else if (rc == 0) {
            if (n == 1) 
                return 0; //还没开始读就EOF，说明无数据
            else 
                break; // 读了一段数据之后，读到文件EOF结尾了
        } else  
            return -1; //ERROR
    }
    *bufp = 0; //尾0操作？
    return n - 1;
}

ssize_t lshc_read(lshRio_t *rp, void *buf, size_t n) 
{
    size_t nleft = n;
    ssize_t nread;
    char *bufp = buf;
    // printf("start lshc_read\n");
    while (nleft > 0)
    {
        if ((nread = lshRio_read(rp, bufp, nleft)) < 0) 
            return -1;   //error
        else if (nread == 0)
            break;      //EOF 
        
        nleft -= nread;
        bufp += nread;
    }

    return (n - nleft);
}

static ssize_t lshRio_read(lshRio_t *rp, char *buf, size_t n) 
{
    int cnt;
    // printf("start lshRio_read\n");
    while (rp ->rio_cnt <= 0)
    {
        rp ->rio_cnt = read(rp->rio_fd, rp->rio_buf, sizeof(rp->rio_buf));
        if (rp ->rio_cnt < 0) 
        {
            if (errno != EINTR) 
                return -1; //中断导致失败
        } 
        else if (rp->rio_cnt == 0)  //EOF
        {
            return 0;
        }
        else 
            rp->rio_bufptr = rp->rio_buf;
    }

    cnt = n;
    if (rp-> rio_cnt < n)
        cnt = rp->rio_cnt;

    memcpy(buf, rp->rio_bufptr, cnt);
    rp->rio_bufptr += cnt;
    rp->rio_cnt -= cnt;
    return cnt;
}

int fstatcheck(int fd, struct stat *buf) {
    fstat(fd, buf);
    char *type, *readok;
    // printf("start fstatcheck\n");
    if (S_ISREG(buf->st_mode))
        type = "regular";
    else if (S_ISDIR(buf->st_mode))
        type = "directory";
    else 
        type = "other";
    
    if (buf->st_mode & S_IRUSR) 
        readok = "yes";
    else 
        readok = "no";
    
    return 1;
}

size_t strlen_s(const char *str) 
{
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    size_t len;

    pthread_mutex_lock(&mutex);
    len = strlen(str);
    pthread_mutex_unlock(&mutex);
    return len;
}

void reverse(char s[]) 
{
    int i, j;
    char c;
    for (i = 0, j = (int)strlen_s(s) - 1; i < j; i++, j--)
    {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}


//lei 转换的进制
void lshSio_ltoa(int n, char s[], unsigned int lei) 
{
    int i, sign;
    unsigned int leisure = lei;
    if ((sign = n) < 0) 
        n = -n;
    i = 0;
    do {
        s[i++] = n % leisure + '0';
    } while ((n / leisure) > 0);

    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';

    reverse(s);
}

/**
 * @brief 
 * 多线程安全的输入字符串到标准输出
 * 
 * @param s 输入的字符串
 * @return ssize_t 返回输入的字节
 */
ssize_t lshSio_puts(char s[]) 
{
    return write(STDOUT_FILENO, s, strlen_s(s));
}

/**
 * @brief 
 * 多线程安全的输入long 数值 串到标准输出
 * @param v 输入的整型
 * @return ssize_t 返回输入的字节
 */
ssize_t lshSio_putl(long v) 
{
    char s[128];

    lshSio_ltoa(v,s,10);

    return lshSio_puts(s);
}

/**
 * @brief 
 * 多线程安全的输入错误信息到标准输出，然后exit(1)
 * @param s 输入的字符串
 */
void lshSio_error(char s[]) 
{
    lshSio_puts(s);
    _exit(1);
}

void lshUnix_error(char *msg) {
    fprintf(stderr, "%s: %s\n",msg, strerror(errno));
    exit(0);
}