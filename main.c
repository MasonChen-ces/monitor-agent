
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <signal.h>
#include <pthread.h>

#include "log.h"
#include "business.h"

#define SIG_ACTION                             \
    do                                         \
    {                                          \
        sigset_t mask;                         \
        sigemptyset(&mask);                    \
        sigaddset(&mask, SIGKILL);             \
        sigaddset(&mask, SIGINT);              \
        sigaddset(&mask, SIGQUIT);             \
        sigaddset(&mask, SIGTERM);             \
        sigaddset(&mask, SIGSTOP);             \
        sigaddset(&mask, SIGCHLD);             \
        sigprocmask(SIG_SETMASK, &mask, NULL); \
    } while (0)

#define ERR_EXIT(m)         \
    do                      \
    {                       \
        perror(m);          \
        exit(EXIT_FAILURE); \
    } while (0)



pthread_mutex_t filelock =PTHREAD_MUTEX_INITIALIZER;
int listenfd = 0;
char socketFile[1024];
void signal_handler(int sig)
{
    int fd;
    int brk = 1;
    if (sig == SIGABRT || sig == SIGKILL || sig == SIGTERM)
    {
        fd = open(socketFile, O_WRONLY | O_CREAT | O_SYNC, 0644);
        lseek(fd, 0, SEEK_SET);
        if (pthread_mutex_lock(&filelock) == 0)
        {
            write(fd, &brk, sizeof(int));
            printf("pid:%d writing socket file.\n",getpid());
            fsync(fd);
            pthread_mutex_unlock(&filelock);
        }
        close(fd);
        if (listenfd > 0)
            close(listenfd);
        printf("user abort the service.\n");

        exit(EXIT_SUCCESS);
    }
}

typedef struct args
{
    int sPort;   //服务端口
    char *sAddr; //服务地址
    int backlog;
} ENVOPTS;

static void usage(int argc, char *argv[])
{
    printf("Usage:\n %s -h <listener address> -p <listener port> -b <number of backlog>\n", argv[0]);
    exit(EXIT_FAILURE);
}

/**
 * @brief 处理main传入的参数
 *
 * @param argc
 * @param argv
 * @return ENVOPTS
 */
static ENVOPTS operateOPT(int argc, char *argv[])
{
    ENVOPTS env;
    int result;

    if (argc < 2)
    {
        usage(argc, argv);
    }
    char *optStr = "h:p:b:";
    env.sAddr = "127.0.0.1";
    env.sPort = 8888;
    env.backlog = 128;

    while ((result = getopt(argc, argv, optStr)) >= 0)
    {
        switch (result)
        {
        case 'h':
            env.sAddr = optarg;
            break;
        case 'p':
            env.sPort = atoi(optarg);
            break;
        case 'b':
            env.backlog = atoi(optarg);
            break;
        default:
            usage(argc, argv);
            break;
        }
    }
    return env;
}

/**
 * @brief 入口函数
 *
 * @param argc 参数个数
 * @param argv 参数数组
 * @return int 返回值
 */
int main(int argc, char *argv[])
{
    ENVOPTS env;
    int fd;
    size_t ret;
    int conn;
    struct sockaddr_in peeraddr;
    socklen_t peerlen = sizeof(peeraddr);

    if (signal(SIGABRT, signal_handler) == SIG_ERR)
    {
        perror("signal abort");
        exit(EXIT_FAILURE);
    }

    SIG_ACTION;

    /*忽略CHLD信号*/
    signal(SIGCHLD, SIG_IGN);

    /*处理参数*/
    env = operateOPT(argc, argv);
    bzero(socketFile, 1024);
    /*create tempoary dir*/
    char *cwd = getcwd(NULL, 1024);
    if (!cwd)
    {
        perror("getcwd");
        exit(EXIT_FAILURE);
    }
    debug("current path:%s\n", cwd);

    char *tmpdir = calloc(1, 1024);
    // snprintf(tmpdir, 1024, "%s/tmp", cwd);
    snprintf(socketFile, 1024, "%s/.socket", cwd);
    fd = open(socketFile, O_WRONLY | O_CREAT, 0644);
    if (fd < 0)
        ERR_EXIT("open socket file.");
    int brk = 0;
    ret = write(fd, &brk, sizeof(int));
    if (ret != sizeof(int))
    {
        // printf("%s ret=%s\n", socketFile, ret);
        ERR_EXIT("write socket file.");
    }
    close(fd);
    if (access(tmpdir, F_OK) < 0)
    {
        mkdir(tmpdir, 0744);
    }

    free(tmpdir);
    free(cwd);

    /*start listener*/
    info("Listener Address:%s\n", env.sAddr);
    info("Listener port:%d\n", env.sPort);
    info("Listener backlog:%d\n", env.backlog);

    if ((listenfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        ERR_EXIT("socket");
    }

    /*listen地址*/
    struct sockaddr_in srvAddr;
    memset(&srvAddr, 0, sizeof(struct sockaddr_in));
    srvAddr.sin_family = AF_INET;
    srvAddr.sin_port = htons(env.sPort);

    if (!inet_aton(env.sAddr, &srvAddr.sin_addr))
        ERR_EXIT("inet_aton");

    /*将listenfd设置为reuse*/
    int on = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
        ERR_EXIT("setsockopt");

    /*绑定地址*/
    if (bind(listenfd, (struct sockaddr *)&srvAddr, sizeof(struct sockaddr)) < 0)
        ERR_EXIT("bind");

    /*设置listener*/
    if (listen(listenfd, env.backlog) < 0)
        ERR_EXIT("listen");

    printf("Start listener on %s:%d\n", inet_ntoa(srvAddr.sin_addr), htons(srvAddr.sin_port));
    printf("kill -6 %d, will stop the service.\n", getpid());

    pid_t pid;
    while (1)
    {
        memset(&peeraddr, 0, sizeof(struct sockaddr_in));
        if ((conn = accept(listenfd, (struct sockaddr *)&peeraddr, &peerlen)) < 0)
            ERR_EXIT("accept");
        info("client %s:%d connected.\n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));
        pid = fork();
        if (pid == -1)
            ERR_EXIT("fork");
        else if (pid == 0)
        {
            close(listenfd);
            SIG_ACTION;
            do_service(conn, inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port), socketFile);
            exit(EXIT_SUCCESS);
        }
        else
        {

            close(conn);
        }
        // printf("------------------------------\n");
    }

    exit(EXIT_SUCCESS);
}