#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
//ft_strlen check lenght of string
int ft_strlen(char *v)
{
    int i = 0;
    while (v[i] != 0)
        i++;
    return (i);
}
//this function divide the error in two categories: with argument and note
void    error(char *v, char *s, int type)
{
    if (type == 1)
        write(2, v, ft_strlen(v));
    else if (type == -1)
    {
        write(2, v, ft_strlen(v));
        write(2, s, ft_strlen(s));
    }
    write(2, "\n", 1);
}

//find possible pipe
int valid_pipe(char **v, int i)
{
    while (v[i] != NULL && v[i][0] != ';')
    {
        if (v[i][0] == '|')
            return (1);
        i++;
    }
    return (0);
}

void    child(char **v, int i, char **env, int *fd)
{
    int j = i;
    //now start duping
    //first of first i need to close the other not used now end of the pipe in my case reading pipe(fd[0])
    //the make my code write in the pipe(fd[1])
    //end close that checking always possible errors
    if (close(fd[0]) == -1)
        error("error: fatal", NULL, 1);
    while (v[j] != NULL && v[j][0] != ';' && v[j][0] != '|')
        j++;
    //make the the '|' ';' and NULL (NULL) so i can execute until it and in case i end up in ';' or null i write in the terminal
    if (v[j] != NULL && v[j][0] == '|')
    {
        if (dup2(fd[1], 1) == -1)
            error("error: fatal", NULL, 1);
    }
    v[j] = NULL;
    if (close(fd[1]) == -1)
        error("error: fatal", NULL, 1);
    //we are duping so the output of the code is written and transported away by the pipe
    //execute the command checking possible error
    if (execve(v[i], &v[i], env) == -1)
        error("error: cannot execute ", v[i], -1);
    exit(1);
}

int    main(int c, char **v, char **env)
{
    //initialize variable
    int i = 1;
    int j = 0;
    (void)c;
    pid_t   pid;
    int     fd[2];
    //in my case i don't use c so i eliminate in this way
    //start loop
    //check if there is a ';' in that case i skip this one at the start
    while (v[i] != NULL && v[i][0] == ';')
        i++;
    while (v[i] != NULL)
    {
    //start check cd and pipe or execve single
        if (strcmp(v[i], "cd") == 0)
        {
            //check invalid cd
            if (v[i + 1] == NULL || v[i + 1][0] == ';')
                error("error: cd: bad arguments", NULL, 1);
            //execute cd if valid
            else if (chdir(v[i + 1]) == -1)
                error("error: cd: cannot change ", v[i + 1], -1);
        }
        else
        {
            //try working on pipe and single command
            //check if there is a pipe
            if (valid_pipe(v, i) == 1)
            {
                while (v[i] != NULL && v[i][0] != ';')
                {
                    //create pipe for each child and father in loop checking if is valid
                    if (pipe(fd) == -1)
                        error("error: fatal", NULL, 1);
                    //create fork check if valid
                    pid = fork();
                    if (pid == -1)
                        error("error: fatal", NULL, 1);
                    //split parent(> 0) and child(== 0)
                    if (pid == 0)
                        child(v, i, env, fd);
                    else
                    {
                        //is the same as child execpt that we pass the input to the next child with this dup2
                        if (close(fd[1]) == -1)
                            error("error: fatal", NULL, 1);
                        if (dup2(fd[0], 0) == -1)
                            error("error: fatal", NULL, 1);
                        if (close(fd[0]) == -1)
                            error("error: fatal", NULL, 1);
                        //then we wait the child
                        waitpid(pid, 0, 0);
                    }
                    while (v[i] != NULL && v[i][0] != ';')
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
            else
            {
                //in case of single command i execute him in the terminal
                //first i create a fork and check if valid
                pid = fork();
                if (pid == -1)
                    error("error: fatal", NULL, 1);
                //pid 0 = child
                //pid > 0 parent
                if (pid == 0)
                {
                    j = i;
                    while (v[j] != NULL && v[j][0] != ';')
                        j++;
                    v[j] = NULL;
                    //execute child if valid
                    if (execve(v[i], &v[i], env) == -1)
                        error("error cannot execute ", v[i], -1);
                }
                else
                {
                    //reach the past argumnt after the code that the child execute
                    //parent wait end of child
                    waitpid(pid, 0, 0);
                }
            }
        }
        //reach next argument to work with
        while (v[i] != NULL && v[i][0] != ';')
            i++;
        //skip ';'
        while (v[i] != NULL && v[i][0] == ';')
            i++;
    }
}