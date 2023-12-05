#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

int ft_strlen(char *s)
{
    int i = 0;

    while (s[i])
        i++;
    return(i);
}

void    error(char *v, char *s)
{
    write(2, v, ft_strlen(v));
    if (s != NULL)
        write(2, s, ft_strlen(s));
    write(2 ,"\n", 1);
}

void    child(char **v, char **env, int *fds, int i, int j)
{
    if (close(fds[0]) == -1)
        error("error: fatal", NULL);
    while (v[j] && v[j][0] != ';')
    {
        if (v[j][0] == '|')
        {
            v[j] = NULL;
            if (dup2(fds[1], 1) == -1)
            error("error: fatal", NULL);
            break ;
        }
        j++;
    }
    v[j] = NULL;
    if (close(fds[1]) == -1)
        error("error: fatal", NULL);
    if (execve(v[i], &v[i], env) == -1)
        error("error: cannot execute ", v[i]);
}

int    main(int c, char **v, char **env)
{
    int i = 1;
    int fds[2];
    pid_t pid;
    c = c;
    while (v[i] != NULL && v[i][0] == ';')
        i++;
    while (v[i] != NULL)
    {
        if (strcmp(v[i], "cd") == 0)
        {
            i++;
            if (v[i] == NULL || v[i][0] == ';')
                error("error: cd: bad arguments", NULL);
            else if (chdir(v[i]) == -1)
                error("error: cd: cannot change directory to ", v[i]);
        }
        else
        {
            while (v[i] && v[i][0] != ';')
            {
                if (pipe(fds) == -1)
                    error("error: fatal", NULL);
                pid = fork();
                if (pid == 0)
                    child(v, env, fds, i, i);
                else
                {
                    if (close(fds[1]) == -1)
                        error("error: fatal", NULL);
                    if (dup2(fds[0], 0) == -1)
                        error("error: fatal", NULL);
                    if (close(fds[0]) == -1)
                        error("error: fatal", NULL);
                    waitpid(pid, 0, 0);
                }
                while(v[i] && v[i][0] != ';')
                {
                    if (v[i][0] == '|')
                    {
                        i++;
                        break ;
                    }
                    i++;
                }
            }
        }
        while (v[i] != NULL && v[i][0] != ';')
            i++;
        while (v[i] != NULL && v[i][0] == ';')
            i++;
    }
}