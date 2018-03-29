#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <string.h>


int next[8][2] = {{1, -2}, {2, -1},
                 {2, 1},   {1, 2},
                 {-1, 2},  {-2, 1},
				 {-2, -1}, {-1, -2}};


int **g_board = NULL;
int m = 0;
int n = 0;
int cnt = 0;
int p_fd = 0;
int no_child = 1;


void my_free() 
{
	if (g_board) {
		for (int i = 0; i < m; ++i) {
			free(g_board[i]);
		}
		free(g_board);
		g_board = NULL;
	}
}

void print()
{
	for (int i = 0; i < m; ++i) {
		printf("PID %d:   ", getpid());
		for (int j = 0; j < n; ++j) {
			printf("%c", g_board[i][j] ? 'k':'.');
		}
		printf("\n");
	}

	fflush(stdout);
}

int init()
{
	int ** board = calloc(m, sizeof(int*));

	if (board == NULL) {
		perror("Error in calloc.");
        return -1;
    }

	for (int i = 0; i < m; ++i) {
    	board[i] = calloc(n, sizeof(int));

    	if (board[i] == NULL) {
			perror("Error in calloc.");

			for (int j = 0; j < i; ++j) {
				free(board[j]);
			}
			free(board);
			
	        return -1;
    	}
    }

    if (g_board != NULL) {
    	for (int i = 0; i < m; ++i) {
    		memcpy((void*) board[i], (const void*) g_board[i], sizeof(int)*n);
    	}
    }

    g_board = board;

    return 0;
}

int solve_knight(int x , int y) 
{
    int pipe_fds[8] = {0};
    int buf = 0;


    int pos[8][2];
    memset(pos, -1, sizeof(pos));
    int step = 0;

    for (int i = 0; i < 8; ++i) {
		int a = x + next[i][0];
    	int b = y + next[i][1];

    	if (a >= 0 && a < m && b >= 0 && b < n && g_board[a][b] == 0) {
    		pos[step][0] = a;
    		pos[step][1] = b;
    		++step;
    	}
    }

    if (step == 0) {
    	no_child = 1;
    	//if (cnt < m*n) {
    		printf("PID %d: Dead end after move #%d\n", getpid(), cnt);
    		fflush(stdout);
    	//} 
    	
    	#ifdef DISPLAY_BOARD
    	print();
    	#endif

    	return EXIT_SUCCESS;
    }

    if (step == 1) {
    	g_board[pos[0][0]][pos[0][1]] = ++cnt;
    	return solve_knight(pos[0][0], pos[0][1]);
    }

    printf("PID %d: %d moves possible after move #%d\n", getpid(), step, cnt);
    fflush(stdout);
    #ifdef DISPLAY_BOARD
    	print();
    #endif
   
    for (int i = 0; i < step; ++i) {
    			
		int fds[2];
	    if (pipe(fds) == -1) {
	        perror("pipe()");
	        my_free();
	        return EXIT_FAILURE;
	    }


	    pid_t pid = fork();
	    if (pid < 0) {
		    perror("fork() failed");
		    return EXIT_FAILURE;
	    }


	    if (pid == 0) {

	    	//printf("PID %d:  parent %d\n", getpid(), getppid());
	    	init();

	    	g_board[pos[i][0]][pos[i][1]] = ++cnt;
	    	p_fd = fds[1];

	    	if (cnt < m*n) {
	    		solve_knight(pos[i][0], pos[i][1]);
	    	} 

			write(p_fd, &cnt, sizeof(cnt));
			if (no_child) {
	    		printf("PID %d: Sent %d on pipe to parent\n", getpid(), cnt);
				fflush(stdout);
			}
			
			my_free();

			exit(0);
	    	
	    } else {
	    	no_child = 0;

		    pipe_fds[i] = fds[0];
		 	#ifdef NO_PARALLEL
		 		int status;
				wait(&status);
			#endif
	    }
    }

    for (int i = 0; i < step; ++i) {

    	if (read(pipe_fds[i], &buf, sizeof(buf)) < 0) {
    		perror("read pipe failed");
    		return EXIT_FAILURE;
    	}

    	if (buf > cnt) {
    		cnt = buf;
    	}

    	printf("PID %d: Received %d from child\n", getpid(), buf);
    	fflush(stdout);
    }

    if (p_fd) {
		printf("PID %d: All child processes terminated; sent %d on pipe to parent\n", getpid(), cnt);
		fflush(stdout);
	} else {
		printf("PID %d: Best solution found visits %d squares (out of %d)\n", getpid(), cnt, m*n);
		fflush(stdout);
	}
	
    return EXIT_SUCCESS;
}

int main(int argc, char** argv)
{
	if (argc != 3 || atoi(argv[1]) <= 2 || atoi(argv[2]) <= 2) {
		fprintf(stderr, "ERROR: Invalid argument(s)\n");
		fprintf(stderr, "USAGE: a.out <m> <n>\n");
		return EXIT_FAILURE;
	}

	m = atoi(argv[1]);
	n = atoi(argv[2]);

	if (init()) {
		return EXIT_FAILURE;
	}

	g_board[0][0] = ++cnt;
	printf("PID %d: Solving the knight's tour problem for a %dx%d board\n", getpid(), m, n);
	fflush(stdout);
	solve_knight(0, 0);

	int status;
	wait(&status);

	return EXIT_SUCCESS;
}