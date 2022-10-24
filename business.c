
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/shm.h>

#include "business.h"
#include "log.h"
#include "business.h"
#include "recvMSGParser.h"
#include "sndMessage.h"

static fpos_t pos;
static int fd_stdout;
int shmid;
/*判断连接是否正常的宏,mac和linux不同*/
#ifdef MAC
#define CONN_INFO(m)                                                                             \
    {                                                                                            \
        struct tcp_connection_info info;                                                         \
        int length = sizeof(struct tcp_connection_info);                                         \
        getsockopt(conn, IPPROTO_TCP, TCP_CONNECTION_INFO, (void *)&info, (socklen_t *)&length); \
        if (info.tcpi_state == 5)                                                                \
        {                                                                                        \
            break;                                                                               \
        }                                                                                        \
    }                                                                                            \
    while (0)
#else
#define CONN_INFO(m)                                                                  \
    {                                                                                 \
        struct tcp_info info;                                                         \
        int length = sizeof(struct tcp_info);                                         \
        getsockopt(conn, IPPROTO_TCP, TCP_INFO, (void *)&info, (socklen_t *)&length); \
        if (info.tcpi_state != TCP_ESTABLISHED)                                       \
        {                                                                             \
            break;                                                                    \
            ;                                                                         \
        }                                                                             \
    }                                                                                 \
    while (0)
#endif

int ifBreak(int shmid)
{
    int ret = 1;
    int *p;
    if ((p = shmat(shmid, 0, 0)) < 0)
    {
        perror("shmat");
        ret = 1;
    }
    else
    {
        ret = *p;
        shmdt(p);
    }
    return ret;
}

/*生成临时存放命令结果文件的名称*/
char *toOutputFile()
{
    char *outfile = NULL;
    char *cwd;
    outfile = calloc(1, 1024);
    pid_t pid = getpid();
    if (!(cwd = getcwd(NULL, 1024)))
    {
        perror("getcwd");
        return NULL;
    }
    snprintf(outfile, 1024, "%s/tmp/out_%d.out", cwd, pid);
    // debug("output filename:%s\n", outfile);s
    return outfile;
}

/*生成shell文件名*/
char *toshell(char *cmd, int len)
{
    char *shellName = NULL;
    char *cwd = NULL;

    int fd;
    shellName = calloc(1, 1024);
    pid_t pid = getpid();
    if (!(cwd = getcwd(NULL, 1024)))
    {
        perror("getcwd");
        return NULL;
    }
    snprintf(shellName, 1024, "%s/tmp/tmp_%d.sh", cwd, pid);
    debug("filename:%s\n", shellName);

    fd = open(shellName, O_CREAT | O_WRONLY | O_TRUNC, 0755);
    if (fd < 0)
    {
        perror("open tmp shell");
        return NULL;
    }

    if (write(fd, cmd, len) < 0)
    {
        perror("write tmp shell");
        exit(EXIT_FAILURE);
    }
    close(fd);
    return shellName;
}

/*切换标准输出到文件*/
void switchStdout(const char *newStream)
{
    fflush(stdout);
    fgetpos(stdout, &pos);
    fd_stdout = dup(fileno(stdout));
    freopen(newStream, "w", stdout);
}

/*重置标准输出*/
resetStdout()
{
    fflush(stdout);
    dup2(fd_stdout, fileno(stdout));
    close(fd_stdout);
    clearerr(stdout);
    fsetpos(stdout, &pos);
}

/*删除临时文件*/
void rmShell(char *shell, char *output)
{
    remove(shell);
    remove(output);
}

/*执行shell，并将结果输出到临时文件中*/
char *execute(char *shell, char *output)
{

    off_t offset;
    char *outStr;
    fflush(NULL);
    switchStdout(output);
    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork execvp");
        rmShell(shell, output);
        return NULL;
    }
    else if (pid == 0)
    {
        // child process
        char *argv[] = {shell, NULL};
        if (execvp(argv[0], argv) < 0)
        {
            perror("execvp");
            rmShell(shell, output);
            return NULL;
        }
        fflush(NULL);
        return outStr;
    }

    wait(NULL);
    resetStdout();
    int outfd = open(output, O_RDONLY);
    if (outfd < 0)
    {
        perror("open output file");
        rmShell(shell, output);
        return NULL;
    }

    offset = lseek(outfd, 0, SEEK_END);
    if (offset <= 0)
    {
        offset = 1;
    }

    debug("offset:%ld.\n", offset);

    lseek(outfd, 0, SEEK_SET);
    outStr = calloc(1, offset);
    if (outStr == NULL)
    {
        debug("outStr is NULL.\n");
    }
    int ret = read(outfd, outStr, offset);
    if (ret < 0)
    {
        perror("read output file");
        info("outStr is empty\n");
        outStr = "No data found";
    }

    close(outfd);
    rmShell(shell, output);
    return outStr;
}

/*业务主函数，由子进程直接调用*/
void do_service(int conn, char *addr, int port, int shmid)
{
    if (!conn)
    {
        printf("connection invalid.\n");
        return;
    }
    MParser *mp = MParser_New();
    while (mp)
    {
        RecvMessage *m = NULL;
        SndMessage *sm = NULL;
        char *result = NULL;
        m = MParser_ReadFd(mp, conn);
        if (m)
        {
            debug("type:   %d\n", m->type);
            debug("cmd:    %d\n", m->cmd);
            debug("index:  %d\n", m->index);
            debug("total:  %d\n", m->total);
            debug("length: %d\n", m->length);
            debug("payload:%s\n", m->payload);

            if (m->cmd == CMD_SHELL)
            {
                char *shell = toshell(m->payload, m->length);
                if (!shell)
                {
                    info("shell is failed.\n");
                    break;
                }
                char *outputfile = toOutputFile();
                if (!outputfile)
                {
                    info("outputfile failed.\n");
                    break;
                }

                result = execute(shell, outputfile);
                int slen = strlen(result);
                int i;

                sm = SndMessage_new(slen, result);
                write(conn, sm, sizeof(SndMessage) + slen);
                fflush(NULL);
                free(result);
                free(shell);
                free(outputfile);
            }

            fflush(NULL);
        }

        if (m)
        {
            free(m);
            m = NULL;
        }
        if (sm)
        {
            free(sm);
            sm = NULL;
        }

        /*判断tcp状态，如果不是TCP_ESTABLISHED,就跳出循环，关闭conn*/
        CONN_INFO(conn);
        usleep(1000);
        if (ifBreak(shmid))
        {
            break;
        }
    }
    info("client %s:%d closed.\n", addr, port);
    if (mp)
        MParser_Del(mp);
    info("server socket closed.(%s:%d)\n", addr, port);
    close(conn);
    fflush(NULL);

    return;
}
