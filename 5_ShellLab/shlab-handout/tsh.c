/* 
 * tsh - A tiny shell program with job control
 * 
 * <Put your name and login ID here>
 */

/* Begin of macros and type definition to avoid VS Code complaining. */
// #define _POSIX_SOURCE

// static void print_argv(char **argv) {
//     int i;
//     for (i = 0; i < MAXARGS && argv[i]; ++i) {
//         printf("argv[%d]  %s\n", i, argv[i]);
//     }
// }

// static void printjob(struct job_t *job) {
//     printf("JID: %d, PID: %2d, PGID: %d, STATE: %d, CMD: %s",
//            job->jid, job->pid, getpgid(job->pid), job->state, job->cmdline);
// }
/* End of macros and type definition to avoid VS Code complaining. */

#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/* Misc manifest constants */
#define MAXLINE 1024   /* max line size */
#define MAXARGS 128    /* max args on a command line */
#define MAXJOBS 16     /* max jobs at any point in time */
#define MAXJID 1 << 16 /* max job ID */

/* Job states */
#define UNDEF 0 /* undefined */
#define FG 1    /* running in foreground */
#define BG 2    /* running in background */
#define ST 3    /* stopped */

/* 
 * Jobs states: FG (foreground), BG (background), ST (stopped)
 * Job state transitions and enabling actions:
 *     FG -> ST  : ctrl-z
 *     ST -> FG  : fg command
 *     ST -> BG  : bg command
 *     BG -> FG  : fg command
 * At most 1 job can be in the FG state.
 */

/* Global variables */
extern char **environ;   /* defined in libc */
char prompt[] = "tsh> "; /* command line prompt (DO NOT CHANGE) */
int verbose = 0;         /* if true, print additional output */
int nextjid = 1;         /* next job ID to allocate */
char sbuf[MAXLINE];      /* for composing sprintf messages */

struct job_t {             /* The job struct */
    pid_t pid;             /* job PID */
    int jid;               /* job ID [1, 2, ...] */
    int state;             /* UNDEF, BG, FG, or ST */
    char cmdline[MAXLINE]; /* command line */
};
struct job_t jobs[MAXJOBS]; /* The job list */

sigset_t mask_all, mask_sigchld;
int fg_terminated = 0;
/* End global variables */

/* Function prototypes */

/* Here are the functions that you will implement */
void eval(char *cmdline);
int builtin_cmd(char **argv);
void do_bgfg(char **argv);
void waitfg(pid_t pid);

void sigchld_handler(int sig);
void sigtstp_handler(int sig);
void sigint_handler(int sig);

/* Here are helper routines that we've provided for you */
int parseline(const char *cmdline, char **argv);
void sigquit_handler(int sig);

void clearjob(struct job_t *job);
void initjobs(struct job_t *jobs);
int maxjid(struct job_t *jobs);
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline);
int deletejob(struct job_t *jobs, pid_t pid);
int setjobstate(struct job_t *jobs, pid_t pid, int new_state); /* YY */
pid_t fgpid(struct job_t *jobs);
struct job_t *getjobpid(struct job_t *jobs, pid_t pid);
struct job_t *getjobjid(struct job_t *jobs, int jid);
int pid2jid(pid_t pid);
void listjobs(struct job_t *jobs);

void usage(void);
void unix_error(char *msg);
void app_error(char *msg);
typedef void handler_t(int);
handler_t *Signal(int signum, handler_t *handler);

/* Wrapper functions borrowed from csapp.c  - YY */
pid_t Fork(void);
void Sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
void Sigemptyset(sigset_t *set);
void Sigfillset(sigset_t *set);
void Sigaddset(sigset_t *set, int signum);
void Sigdelset(sigset_t *set, int signum);
int Sigismember(const sigset_t *set, int signum);
int Sigsuspend(const sigset_t *set);

/*
 * main - The shell's main routine 
 */
int main(int argc, char **argv) {
    char c;
    char cmdline[MAXLINE];
    int emit_prompt = 1; /* emit prompt (default) */

    /* Redirect stderr to stdout (so that driver will get all output
     * on the pipe connected to stdout) */
    dup2(1, 2);

    /* Parse the command line */
    while ((c = getopt(argc, argv, "hvp")) != EOF) {
        switch (c) {
            case 'h': /* print help message */
                usage();
                break;
            case 'v': /* emit additional diagnostic info */
                verbose = 1;
                break;
            case 'p':            /* don't print a prompt */
                emit_prompt = 0; /* handy for automatic testing */
                break;
            default:
                usage();
        }
    }

    /* Set value for signal masks. - YY */
    Sigfillset(&mask_all);
    Sigemptyset(&mask_sigchld);
    Sigaddset(&mask_sigchld, SIGCHLD);

    /* Install the signal handlers */
    /* These are the ones you will need to implement */
    Signal(SIGINT, sigint_handler);   /* ctrl-c */
    Signal(SIGTSTP, sigtstp_handler); /* ctrl-z */
    Signal(SIGCHLD, sigchld_handler); /* Terminated or stopped child */

    /* This one provides a clean way to kill the shell */
    Signal(SIGQUIT, sigquit_handler);

    /* Initialize the job list */
    initjobs(jobs);

    /* Execute the shell's read/eval loop */
    while (1) {
        /* Read command line */
        if (emit_prompt) {
            printf("%s", prompt);
            fflush(stdout);
        }
        if ((fgets(cmdline, MAXLINE, stdin) == NULL) && ferror(stdin))
            app_error("fgets error");
        if (feof(stdin)) { /* End of file (ctrl-d) */
            fflush(stdout);
            exit(0);
        }

        /* Evaluate the command line */
        eval(cmdline);
        fflush(stdout);
        fflush(stdout);
    }

    exit(0); /* control never reaches here */
}

/* 
 * eval - Evaluate the command line that the user has just typed in
 * 
 * If the user has requested a built-in command (quit, jobs, bg or fg)
 * then execute it immediately. Otherwise, fork a child process and
 * run the job in the context of the child. If the job is running in
 * the foreground, wait for it to terminate and then return.  Note:
 * each child process must have a unique process group ID so that our
 * background children don't receive SIGINT (SIGTSTP) from the kernel
 * when we type ctrl-c (ctrl-z) at the keyboard.  
*/
void eval(char *cmdline) {
    char *argv[MAXARGS];
    char cmdline_copy[MAXLINE];
    int bg;
    int pid;
    sigset_t prev_mask;

    strcpy(cmdline_copy, cmdline);
    bg = parseline(cmdline, argv);
    if (argv[0] == NULL) return;

    if (!builtin_cmd(argv)) {
        Sigprocmask(SIG_BLOCK, &mask_sigchld, &prev_mask); /* Only block SIG_CHLD, no others */
        if ((pid = Fork()) == 0) {                         /* Child process runs user job */
            setpgid(0, 0);
            Sigprocmask(SIG_SETMASK, &prev_mask, NULL);
            if (execv(argv[0], argv) < 0) {
                printf("%s: Command not found\n", argv[0]);
                exit(0);
            }
        }

        int current_jid = nextjid;
        Sigprocmask(SIG_BLOCK, &mask_all, NULL); /* BLock all signals */
        strcpy(cmdline_copy, cmdline);
        if (bg) {
            addjob(jobs, pid, BG, cmdline_copy);
            printf("[%d] (%d) %s", current_jid, pid, cmdline);
        } else {
            addjob(jobs, pid, FG, cmdline_copy);
        }
        Sigprocmask(SIG_SETMASK, &prev_mask, NULL); /* Original block, SIG_CHLD not blocked */

        if (!bg) {
            waitfg(pid);
        }
    }
    return;
}

/* 
 * builtin_cmd - If the user has typed a built-in command then execute
 *    it immediately.  
 */
int builtin_cmd(char **argv) {
    int is_built_in = 0;

    if (strcmp(argv[0], "quit") == 0) {
        kill(getpid(), SIGQUIT);
        is_built_in = 1;
    } else if (strcmp(argv[0], "jobs") == 0) {
        listjobs(jobs);
        is_built_in = 1;
    } else if (strcmp(argv[0], "bg") == 0) {
        do_bgfg(argv);
        is_built_in = 1;
    } else if (strcmp(argv[0], "fg") == 0) {
        do_bgfg(argv);
        is_built_in = 1;
    }

    return is_built_in;
}

/* 
 * do_bgfg - Execute the builtin bg and fg commands
 */
void do_bgfg(char **argv) {
    if (argv[1] == 0) {
        printf("%s command requires PID or %%jobid argument\n", argv[0]);
        return;
    }

    long num;
    if (*(argv[1]) == '%') {
        num = strtol((argv[1] + 1), NULL, 10);
    } else {
        num = strtol(argv[1], NULL, 10);
    }

    if (num <= 0) {
        printf("%s: argument must be a PID or %%jobid\n", argv[0]);
        return;
    }

    if (argv[2] != 0) {
        printf("Too many arguments.");
        return;
    }

    int bg = strcmp(argv[0], "bg") == 0;
    int pid = -1, jid = -1;
    struct job_t *job;
    sigset_t prev_mask;

    Sigprocmask(SIG_SETMASK, &mask_all, &prev_mask);
    if (*(argv[1]) == '%') {
        jid = num;
        job = getjobjid(jobs, jid);
        if (!job) {
            printf("(%%%d): No such job\n", jid);
            Sigprocmask(SIG_SETMASK, &prev_mask, NULL);
            return;
        }
        pid = job->pid;
    } else {
        pid = num;
        job = getjobpid(jobs, pid);
        if (!job) {
            printf("(%d): No such process\n", pid);
            Sigprocmask(SIG_SETMASK, &prev_mask, NULL);
            return;
        }
        jid = job->jid;
    }
    Sigprocmask(SIG_SETMASK, &prev_mask, NULL);

    if (bg) {
        printf("[%d] (%d) %s", jid, pid, job->cmdline);
        kill(-pid, SIGCONT);
        Sigprocmask(SIG_SETMASK, &mask_all, &prev_mask);
        setjobstate(jobs, pid, BG);
        Sigprocmask(SIG_SETMASK, &prev_mask, NULL);
    } else {
        /**
         * If a job is originally running in the background, "fg" move it to 
         * the foreground(but the terminal need not display any message). 
         */
        Sigprocmask(SIG_SETMASK, &mask_all, &prev_mask);
        setjobstate(jobs, pid, FG);
        Sigprocmask(SIG_SETMASK, &prev_mask, NULL);
        kill(-pid, SIGCONT);
        waitfg(pid);
    }
}

/* 
 * waitfg - Block until process pid is no longer the foreground process
 */
void waitfg(pid_t pid) {
    sigset_t prev_mask;
    while (1) {
        Sigprocmask(SIG_SETMASK, &mask_all, &prev_mask);

        int jid = pid2jid(pid);
        int state = -1;

        struct job_t *fb_job = getjobpid(jobs, pid);
        if (fb_job) {
            state = fb_job->state;
        }
        Sigprocmask(SIG_SETMASK, &prev_mask, NULL);

        if (jid == 0 || (state != -1 && state != FG)) {
            /* Job deleted or no longer foreground */
            return;
        }

        sigsuspend(NULL);
    }
}

/*****************
 * Signal handlers
 *****************/

/* 
 * sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
 *     a child job terminates (becomes a zombie), or stops because it
 *     received a SIGSTOP or SIGTSTP signal. The handler reaps all
 *     available zombie children, but doesn't wait for any other
 *     currently running children to terminate.  
 */
void sigchld_handler(int sig) {
    int old_errno = errno;
    pid_t pid, jid;
    sigset_t prev_mask;
    int status;

    // Reap as many as possible, but should not wait for unexited child!
    // Use WNOHANG option.
    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED)) > 0) {
        Sigprocmask(SIG_BLOCK, &mask_all, &prev_mask);
        jid = pid2jid(pid);

        if (WIFEXITED(status)) {
            // Child process is terminated normally, delete the job.
            // WIFEXITED(wstatus): returns true if the child terminated
            // normally, that is, by calling exit(3) or _exit(2), or by
            // returning from main(). The status is in WEXISTSTATUS(status).
            deletejob(jobs, pid);
        } else if (WIFSIGNALED(status)) {
            // Child process is terminated by signal of WTERMSIG(code)
            deletejob(jobs, pid);
            sio_put("Job [%d] (%d) terminated by signal %d\n",
                   jid, pid, WTERMSIG(status));
            // instead of using printf, use sio_put to avoid interleaving.
        } else if (WIFSTOPPED(status)) {
            // child process was stopped by signal of WSTOPSIG(code)
            setjobstate(jobs, pid, ST);
            sio_put("Job [%d] (%d) stopped by signal %d\n",
                   jid, pid, WSTOPSIG(status));
        }

        Sigprocmask(SIG_SETMASK, &prev_mask, NULL);
    }

    errno = old_errno;
}

/* 
 * sigint_handler - The kernel sends a SIGINT to the shell whenver the
 *    user types ctrl-c at the keyboard.  Catch it and send it along
 *    to the foreground job.  
 */
void sigint_handler(int sig) {
    int old_errno = errno;
    sigset_t prev_mask;
    Sigprocmask(SIG_BLOCK, &mask_all, &prev_mask);
    kill(-fgpid(jobs), SIGINT);
    Sigprocmask(SIG_SETMASK, &prev_mask, NULL);
    errno = old_errno;
}

/*
 * sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
 *     the user types ctrl-z at the keyboard. Catch it and suspend the
 *     foreground job by sending it a SIGTSTP.  
 */
void sigtstp_handler(int sig) {
    int old_errno = errno;
    sigset_t prev_mask;
    Sigprocmask(SIG_BLOCK, &mask_all, &prev_mask);
    kill(-fgpid(jobs), SIGTSTP);
    Sigprocmask(SIG_SETMASK, &prev_mask, NULL);
    errno = old_errno;
}

/*********************
 * End signal handlers
 *********************/

/***********************************************
 * Helper routines that manipulate the job list
 **********************************************/

/* clearjob - Clear the entries in a job struct */
void clearjob(struct job_t *job) {
    job->pid = 0;
    job->jid = 0;
    job->state = UNDEF;
    job->cmdline[0] = '\0';
}

/* initjobs - Initialize the job list */
void initjobs(struct job_t *jobs) {
    int i;

    for (i = 0; i < MAXJOBS; i++)
        clearjob(&jobs[i]);
}

/* maxjid - Returns largest allocated job ID */
int maxjid(struct job_t *jobs) {
    int i, max = 0;

    for (i = 0; i < MAXJOBS; i++)
        if (jobs[i].jid > max)
            max = jobs[i].jid;
    return max;
}

/* addjob - Add a job to the job list */
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline) {
    int i;

    if (pid < 1)
        return 0;

    for (i = 0; i < MAXJOBS; i++) {
        if (jobs[i].pid == 0) {
            jobs[i].pid = pid;
            jobs[i].state = state;
            jobs[i].jid = nextjid++;
            if (nextjid > MAXJOBS)
                nextjid = 1;
            strcpy(jobs[i].cmdline, cmdline);
            if (verbose) {
                printf("Added job [%d] %d %s\n", jobs[i].jid, jobs[i].pid, jobs[i].cmdline);
            }
            return 1;
        }
    }
    printf("Tried to create too many jobs\n");
    return 0;
}

/* deletejob - Delete a job whose PID=pid from the job list */
int deletejob(struct job_t *jobs, pid_t pid) {
    int i;

    if (pid < 1)
        return 0;

    for (i = 0; i < MAXJOBS; i++) {
        if (jobs[i].pid == pid) {
            clearjob(&jobs[i]);
            nextjid = maxjid(jobs) + 1;
            return 1;
        }
    }
    return 0;
}

/* setjobstate - Set the state of a job whose PID=pid from the job list*/
int setjobstate(struct job_t *jobs, pid_t pid, int new_state) {
    int i;

    if (pid < 1)
        return 0;

    for (i = 0; i < MAXJOBS; i++) {
        if (jobs[i].pid == pid) {
            jobs[i].state = new_state;
            return 1;
        }
    }
    return 0;
}

/* fgpid - Return PID of current foreground job, 0 if no such job */
pid_t fgpid(struct job_t *jobs) {
    int i;

    for (i = 0; i < MAXJOBS; i++)
        if (jobs[i].state == FG)
            return jobs[i].pid;
    return 0;
}

/* getjobpid  - Find a job (by PID) on the job list */
struct job_t *getjobpid(struct job_t *jobs, pid_t pid) {
    int i;

    if (pid < 1)
        return NULL;
    for (i = 0; i < MAXJOBS; i++)
        if (jobs[i].pid == pid)
            return &jobs[i];
    return NULL;
}

/* getjobjid  - Find a job (by JID) on the job list */
struct job_t *getjobjid(struct job_t *jobs, int jid) {
    int i;

    if (jid < 1)
        return NULL;
    for (i = 0; i < MAXJOBS; i++)
        if (jobs[i].jid == jid)
            return &jobs[i];
    return NULL;
}

/* pid2jid - Map process ID to job ID */
int pid2jid(pid_t pid) {
    int i;

    if (pid < 1)
        return 0;
    for (i = 0; i < MAXJOBS; i++)
        if (jobs[i].pid == pid) {
            return jobs[i].jid;
        }
    return 0;
}

/* listjobs - Print the job list */
void listjobs(struct job_t *jobs) {
    int i;

    for (i = 0; i < MAXJOBS; i++) {
        if (jobs[i].pid != 0) {
            printf("[%d] (%d) ", jobs[i].jid, jobs[i].pid);
            switch (jobs[i].state) {
                case BG:
                    printf("Running ");
                    break;
                case FG:
                    printf("Foreground ");
                    break;
                case ST:
                    printf("Stopped ");
                    break;
                default:
                    printf("listjobs: Internal error: job[%d].state=%d ",
                           i, jobs[i].state);
            }
            printf("%s", jobs[i].cmdline);
        }
    }
}
/******************************
 * end job list helper routines
 ******************************/

/***********************
 * Other helper routines
 ***********************/

/*
 * usage - print a help message
 */
void usage(void) {
    printf("Usage: shell [-hvp]\n");
    printf("   -h   print this message\n");
    printf("   -v   print additional diagnostic information\n");
    printf("   -p   do not emit a command prompt\n");
    exit(1);
}

/*
 * unix_error - unix-style error routine
 */
void unix_error(char *msg) {
    fprintf(stdout, "%s: %s\n", msg, strerror(errno));
    exit(1);
}

/*
 * app_error - application-style error routine
 */
void app_error(char *msg) {
    fprintf(stdout, "%s\n", msg);
    exit(1);
}

/*
 * Signal - wrapper for the sigaction function
 */
handler_t *Signal(int signum, handler_t *handler) {
    struct sigaction action, old_action;

    action.sa_handler = handler;
    sigemptyset(&action.sa_mask); /* block sigs of type being handled */
    action.sa_flags = SA_RESTART; /* restart syscalls if possible */

    if (sigaction(signum, &action, &old_action) < 0)
        unix_error("Signal error");
    return (old_action.sa_handler);
}

/*
 * sigquit_handler - The driver program can gracefully terminate the
 *    child shell by sending it a SIGQUIT signal.
 */
void sigquit_handler(int sig) {
    // printf("Terminating after receipt of SIGQUIT signal\n");
    exit(1);
}

pid_t Fork(void) {
    pid_t pid;

    if ((pid = fork()) < 0)
        unix_error("Fork error");
    return pid;
}

void Sigprocmask(int how, const sigset_t *set, sigset_t *oldset) {
    if (sigprocmask(how, set, oldset) < 0)
        unix_error("Sigprocmask error");
    return;
}

void Sigemptyset(sigset_t *set) {
    if (sigemptyset(set) < 0)
        unix_error("Sigemptyset error");
    return;
}

void Sigfillset(sigset_t *set) {
    if (sigfillset(set) < 0)
        unix_error("Sigfillset error");
    return;
}

void Sigaddset(sigset_t *set, int signum) {
    if (sigaddset(set, signum) < 0)
        unix_error("Sigaddset error");
    return;
}

void Sigdelset(sigset_t *set, int signum) {
    if (sigdelset(set, signum) < 0)
        unix_error("Sigdelset error");
    return;
}

int Sigismember(const sigset_t *set, int signum) {
    int rc;
    if ((rc = sigismember(set, signum)) < 0)
        unix_error("Sigismember error");
    return rc;
}

int Sigsuspend(const sigset_t *set) {
    int rc = sigsuspend(set); /* always returns -1 */
    if (errno != EINTR)
        unix_error("Sigsuspend error");
    return rc;
}

/* 
 * parseline - Parse the command line and build the argv array.
 * 
 * Characters enclosed in single quotes are treated as a single
 * argument.  Return true if the user has requested a BG job, false if
 * the user has requested a FG job.  
 */
int parseline(const char *cmdline, char **argv) {
    static char array[MAXLINE]; /* holds local copy of command line */
    char *buf = array;          /* ptr that traverses command line */
    char *delim;                /* points to first space delimiter */
    int argc;                   /* number of args */
    int bg;                     /* background job? */

    strcpy(buf, cmdline);
    buf[strlen(buf) - 1] = ' ';   /* replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* ignore leading spaces */
        buf++;

    /* Build the argv list */
    argc = 0;
    if (*buf == '\'') {
        buf++;
        delim = strchr(buf, '\'');
    } else {
        delim = strchr(buf, ' ');
    }

    while (delim) {
        argv[argc++] = buf;
        *delim = '\0';
        buf = delim + 1;
        while (*buf && (*buf == ' ')) /* ignore spaces */
            buf++;

        if (*buf == '\'') {
            buf++;
            delim = strchr(buf, '\'');
        } else {
            delim = strchr(buf, ' ');
        }
    }
    argv[argc] = NULL;

    if (argc == 0) /* ignore blank line */
        return 1;

    /* should the job run in the background? */
    if ((bg = (*argv[argc - 1] == '&')) != 0) {
        argv[--argc] = NULL;
    }
    return bg;
}