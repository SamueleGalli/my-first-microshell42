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
        write(2, s, len(s));
    write(2, "\n", 1);
}

void    cding(char **v, int i)
{
    i++;
    if (v[i] != NULL && v[i][0] == ';')
        error(NULL, "error: cd: bad arguments");
    else if (chdir(v[i]) == -1)
        error(v[i], "cannot change directory to ");
}

int piping(char **v, int i)
{
    while (v[i] != NULL && v[i][0] != '|')
        i++;
    if (v[i] != NULL && v[i][0] == '|')
        return (1);
    return (0);
}

void    child(char **v, int fd[2], int i, char **env)
{
    int j = i;
    if (close(fd[0]) == -1)
        error(NULL, "error: fatal");
    while (v[i] != NULL && v[i][0] != ';')
    {
        if (v[i][0] == '|')
        {
            if (dup2(fd[1], 1) == -1)
                error(NULL, "error: fatal");
            break;
        }
        i++;
    }
    v[i] = NULL;
    if (close(fd[1]) == -1)
        error(NULL, "error: fatal");
    i = j;
    if (execve(v[i], &v[i], env) == -1)
        error(v[i], "error: cannot execute ");
    exit(0);
}

void    pipe_exec(char **v, char **env, int i, pid_t pid, int fd[2])
{
    while (v[i] != NULL && v[i][0] != ';')
    {
        if (pipe(fd) == -1)
        {
                error(NULL, "error: fatal");
            exit(0);
        }
        pid = fork();
        if (pid < 0)
        {
            error(NULL, "error: fatal");
            exit(0);
        }
        if (pid == 0)
            child(v, fd, i, env);
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
        if (v[i] != NULL && v[i][0] == '|')
            i++;
    }
}

void    single(pid_t pid, char **env, int i, char **v)
{
    pid = fork();
    if (pid == -1)
    {
        error(NULL, "error: fatal");
        exit(0);
    }
    if (pid == 0)
    {
        int j = i;
        while (v[i] != NULL && v[i][0] != ';')
            i++;
        if (v[i] != NULL && v[i][0] == ';')
            v[i] = NULL;
        i = 0;
        i = j;
        if (execve(v[i], &v[i], env) == -1)
            error (v[i], "error: cannot execute ");
    }
    else
        waitpid(pid, 0, 0);
}

int main(int c, char **v, char **env)
{
    (void)c;
    int i = 1;
    pid_t pid = 0;
    int fd[2];
    while (v[i] != NULL && v[i][0] == ';')
        i++;
    while (v[i])
    {
        if (strcmp(v[i], "cd") == 0)
            cding(v, i);
        else if (piping(v, i) == 1)
            pipe_exec(v, env, i, pid, fd);
        else
        single(pid,env,i, v);
        while (v[i] != NULL && v[i][0] != ';')
            i++;
        while (v[i] != NULL && v[i][0] == ';')
            i++;
    }
}
