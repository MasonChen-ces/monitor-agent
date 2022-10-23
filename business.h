// enum CMD{
//     CMD_SHELL = 1,
//     CMD_SQL
// };

// enum TYPE{
//     TYP_ORACLE = 1,
//     TYP_MYSQL,
//     TYP_POSTGRESQL
// };

#ifndef __BUSINESS__
#define __BUSINESS__
void do_service(int conn,char *addr, int port,char *socketFile);
char *toshell(char *cmd,int len);
char *toOutputFile();
char *execute(char *shell, char *output);
#endif