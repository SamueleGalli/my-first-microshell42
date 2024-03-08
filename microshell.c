#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int	len(char *s)
{
	int	i = 0;
	while (s[i])
		i++;
	return (i);
}

void	error(char *s, char *msg)
{
	write(2, msg, len(msg));
	if (s)
		write(2, s, len(s));
	write(2, "\n", 1);
}

void	cding(char **v, int i)
{
	i++;
	if (v[i] && v[i][0] == ';')
		error(NULL, "error: cd: bad arguments");
	else if (chdir(v[i]) == -1)
		error(v[i], "error: cd: cannot change directory to ");
}

int	ispipe(char **v, int i)
{
	while(v[i] && v[i][0] != '|' &&v[i][0] != ';')
		i++;
	if (v[i] && v[i][0] == '|')
		return (1);
	return (0);
}

void	piping(char **v, int i, char **e, pid_t p, int fd[2])
{
	while(v[i] && v[i][0] != ';')
	{
		pipe(fd);
		p = fork();
		if (p == 0)
		{
			int	j = i;
			if (close(fd[0]) == -1)
				error(NULL, "error: fatal");
			while (v[i] && v[i][0] != ';')
			{
				if (v[i][0] == '|')
				{
					if (dup2(fd[1], 1) == -1)
						error(NULL, "error: fatal");
					break ;
				}
				i++;
			}
			v[i] = 0;
			i = j;	
			if (close(fd[1]) == -1)
				error(NULL, "error: fatal");
			if (execve(v[i], &v[i], e) == -1)
				error(v[i], "error: cannot execute ");
			exit(1);
		}
		else
		{

			if (close(fd[1]) == -1)
				error(NULL, "error: fatal");
			if (dup2(fd[0], 0) == -1)
				error(NULL, "error: fatal");
			if (close(fd[0]) == -1)
				error(NULL, "error: fatal");
			waitpid(p, 0, 0);
		}
		while (v[i] && v[i][0] != '|' && v[i][0] != ';')
			i++;
		if (v[i] && v[i][0] == '|')
			i++;
	}
}

void	single(char **v, int i, char **e, pid_t p)
{
	p = fork();
	if (p == 0)
	{
		int j = i;
		while (v[i] && v[i][0] != ';')
			i++;
		v[i] = 0;
		i = j;
		if (execve(v[i], &v[i], e) == -1)
			error(v[i], "error: cannot execute ");
	}
	else
		waitpid(p, 0, 0);
}

int	main(int c, char **v, char **e)
{
	(void)c;
	int	i = 1;
	int	fd[2];
	pid_t	p = 0;

	while (v[i] && v[i][0] == ';')
		i++;
	while (v[i])
	{
		if (strcmp(v[i], "cd") == 0)
			cding(v, i);
		else if (ispipe(v, i) == 1)
			piping(v, i, e, p, fd);
		else
			single(v, i, e, p);
		while (v[i] && v[i][0] != ';')
			i++;
		while (v[i] && v[i][0] == ';')
			i++;
	}
}
