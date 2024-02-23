#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>

int len(char *s)
{
    int i = 0;
    while (s[i])
        i++;
    return (i);
}

void    error(char *s, char *msg)
{
    write(2, msg, len(msg));
    if (s != NULL)
        write (2, s, len(s));
    write(2, "\n", 1);
}

void    cding(int i, char **v)
{
    i++;
    if (v[i] != NULL && v[i][0] == ';')
        error(NULL, "error: cd: bad arguments");
    else if (chdir(v[i]) != 0)
        error(v[i], "error: cd: cannot change directory to ");
}

void single_ex(int i, char **v, char **env, pid_t pid)
{
    pid = fork();
    if (pid == 0)
    {
        int t = i;
        while (v[i] != NULL && v[i][0] != ';')
            i++;
        if (v[i] != NULL && v[i][0] == ';')
            v[i] = NULL;
        i = t;
        if (execve(v[i], &v[i], env) == -1)
            error(v[i], "error: cannot execute ");
        exit(0);
    }
    else
        waitpid(pid, NULL, 0);
}

void    child(char **v, int i, char **env, int fd[2])
{
    int t = i;
    if (close(fd[0]) == -1)
        error(NULL, "error: fatal");
    while (v[i] != NULL && v[i][0] != '|' && v[i][0] != ';')
        i++;
    if (v[i] != NULL && v[i][0] == '|')
    {
        if (dup2(fd[1], 1) == -1)
            error(NULL, "error: fatal");
    }
    v[i] = NULL;
    if (close(fd[1]) == -1)
        error(NULL, "error: fatal");
    i = t;
    if (execve(v[i], &v[i], env) == -1)
        error(v[i], "error: cannot execute ");
    exit (0);
}

void    piping(int i, char **v, char **env, pid_t pid, int fd[2])
{
    while (v[i] != NULL && v[i][0] != ';')
    {
        if (pipe(fd) == -1)
            error(NULL, "error: fatal");
        pid = fork();
        if (pid < 0)
            error(NULL, "error: fatal");
        if (pid == 0)
            child(v, i, env, fd);
        else
        {
            if (close(fd[1]) == -1)
                error(NULL, "error: fatal");
            if (dup2(fd[0], 0) == -1)
                error(NULL, "error: fatal");
            if (close(fd[0]) == -1)
                error(NULL, "error: fatal");
            waitpid(pid, NULL, 0);
        }
        while (v[i] != NULL && v[i][0] != ';' && v[i][0] != '|')
            i++;
        if (v[i] != NULL && v[i][0] != ';')
            i++;
    }
}

void    execving(int i, char **v, char **env, pid_t pid, int fd[2])
{
    int t = i;
    while (v[i] != NULL && v[i][0] != ';')
    {
        if (v[i][0] == '|')
        {
            i = t;
            piping(i, v, env, pid, fd);
            return ;
        }
        i++;
    }
    i = t;
    single_ex(i, v, env, pid);
}

int main(int c, char **v, char **env)
{
    int i = 1;
    (void)c;
    pid_t pid = 0;
    int fd[2];
    while (v[i] != NULL && v[i][0] == ';')
        i++;
    while (v[i])
    {
        if (strcmp(v[i], "cd") == 0)
            cding(i, v);
        else
            execving(i, v, env, pid, fd);
        while (v[i] != NULL && v[i][0] != ';')
            i++;
        while (v[i] != NULL && v[i][0] == ';')
            i++;
    }
}
