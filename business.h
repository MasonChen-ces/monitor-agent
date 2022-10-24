#ifndef __BUSINESS__
#define __BUSINESS__
void do_service(int conn, char *addr, int port, int shmid);
char *toshell(char *cmd,int len);
char *toOutputFile();
char *execute(char *shell, char *output);
#endif
