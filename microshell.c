#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>

int ft_strlen(char *s)
{
    int i = 0;

    while (s[i])
        i++;
    return (i);
}

void    ft_error_all(int i, char *s, char *value)
{
    if (i == 1)
        write(2, s, ft_strlen(s));
    else if (i == -1)
    {
        write(2, s, ft_strlen(s));
        write(2, value, ft_strlen(value));
        write(2, "\n", 1);
    }
}

int    ft_do_cd(char **v, int i)
{
    if (v[i + 1] != NULL && v[i + 1][0] == ';')
        ft_error_all(1, "error: cd: bad arguments\n", NULL);
    else if (chdir(v[i + 1]) == -1)
        ft_error_all(1, "error: cd: cannot change directory to ", v[i+ 1]);
    return(i + 1);
}

int valid_pipe(int i, char **v)
{
    while (v[i] != NULL)
    {
        if (v[i][0] == '|')
            return (1);
        i++;
    }
    return (0);
}

void    ft_child(char **v, int i, int *fd, char **env)
{
    int j = i;
    while (v[j] != NULL && v[j][0] != '|' && v[j][0] != ';')
        j++;
    if (close(fd[0]) == -1)
        ft_error_all(1, "error: fatal\n", NULL);
    if (v[j] != NULL && v[j][0] == '|')
    {
        v[j] = NULL;
        if (dup2(fd[1], 1) == -1)
            ft_error_all(1, "error: fatal\n", NULL);
    }
    else if (v[j] != NULL && v[j][0] == ';')
        v[j] = NULL;
    if (close(fd[1]) == -1)
        ft_error_all(1, "error: fatal\n", NULL);
    if (execve(v[i], &v[i], env) == -1)
        ft_error_all(-1, "error: cannot execute ", v[i]);
    exit(1);
}

int update_pipe(char **v, int i)
{
    while (v[i] != NULL && v[i][0] != ';')
    {
        if (v[i][0] == '|')
        {
            i++;
            break ;
        }
        i++;
    }
    return (i);
}

int    ft_do_execution(char **v, int i, char **env)
{
    pid_t   pid = 0;
    int fd[2];

    if (valid_pipe(i, v) == 1)
    {
        while (v[i] != NULL && v[i][0] != ';')
        {
            if (pipe(fd) == -1)
                ft_error_all(1, "error: fatal", NULL);
            pid = fork();
            if (pid == 0)
                ft_child(v, i, fd, env);
            else
            {
                if (close(fd[1]) == -1)
                    ft_error_all(1, "error: fatal\n", NULL);
                if (dup2(fd[0], 0) == -1)
                    ft_error_all(1, "error: fatal\n", NULL);
                if (close(fd[0]) == -1)
                    ft_error_all(1, "error: fatal\n", NULL);
                waitpid(pid, 0, 0);
            }
            i = update_pipe(v, i);
        }
    }
    else
    {
        pid = fork();
        if (pid == 0)
        {
            int j = i;
            while (v[j] && v[j][0] != ';')
                j++;
            if (v[j] != NULL && v[j][0] == ';')
            {
                v[j] = NULL;
                j++;
            }
            else
                v[j] = NULL;
            if (execve(v[i], &v[i], env) == -1)
                ft_error_all(-1, "error: cannot execute ", v[i]);
        }
        else
            waitpid(pid, 0, 0);
    }
    return (i);
}

int main(int c, char **v, char **env)
{
    c = c;
    int i = 1;
    while (v[i])
    {
        while (v[i] != NULL && v[i][0] == ';')
            i++;
        if (v[i] == NULL)
            return (0);
        if (strcmp(v[i], "cd") == 0)
            i = ft_do_cd(v, i);
        else
            i = ft_do_execution(v, i, env);
        while (v[i])
        {
            if (v[i][0] == ';')
            {
                i++;
                break ;
            }
            i++;
        }
        while (v[i] != NULL && v[i][0] == ';')
            i++;
    }
}