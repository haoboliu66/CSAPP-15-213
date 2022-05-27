/*
 * tsh - A tiny shell program with job control
 *
 * <Put your name and login ID here>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/* Misc manifest constants */
#define MAXLINE    1024   /* max line size */
#define MAXARGS     128   /* max args on a command line */
#define MAXJOBS      16   /* max jobs at any point in time */
#define MAXJID    1<<16   /* max job ID */

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
extern char **environ;      /* defined in libc */
char prompt[] = "tsh> ";    /* command line prompt (DO NOT CHANGE) */
int verbose = 0;            /* if true, print additional output */
int nextjid = 1;            /* next job ID to allocate */
char sbuf[MAXLINE];         /* for composing sprintf messages */

struct job_t {              /* The job struct */
    pid_t pid;              /* job PID */
    int jid;                /* job ID [1, 2, ...] */
    int state;              /* UNDEF, BG, FG, or ST */
    char cmdline[MAXLINE];  /* command line */
};
struct job_t jobs[MAXJOBS]; /* The job list */
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


void test_print(); // useless for test

int validate_id(char **argv); // validate id following bg or fg


/* Here are helper routines that we've provided for you */
int parseline(const char *cmdline, char **argv);
void sigquit_handler(int sig);

void clearjob(struct job_t *job);
void initjobs(struct job_t *jobs);
int maxjid(struct job_t *jobs);
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline);
int deletejob(struct job_t *jobs, pid_t pid);
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
pid_t Fork(void);
void Execve(const char *filename, char *const argv[], char *const envp[]);
void Kill(pid_t pid, int signum);

/*
 * main - The shell's main routine
 */
int main(int argc, char **argv){
    char c;
    char cmdline[MAXLINE];
    int emit_prompt = 1; /* emit prompt (default) */

    /* Redirect stderr to stdout (so that driver will get all output
     * on the pipe connected to stdout) */
    dup2(1, 2);

    /* Parse the command line */
    while ((c = getopt(argc, argv, "hvp")) != EOF) {
        switch (c) {
        case 'h':             /* print help message */
            usage();
            break;
        case 'v':             /* emit additional diagnostic info */
            verbose = 1;
            break;
        case 'p':             /* don't print a prompt */
            emit_prompt = 0;  /* handy for automatic testing */
            break;
        default:
            usage();
        }
    }

    /* Install the signal handlers */

    /* These are the ones you will need to implement */
    Signal(SIGINT,  sigint_handler);   /* ctrl-c */
    Signal(SIGTSTP, sigtstp_handler);  /* ctrl-z */
    // Signal(SIGCONT, sigcont_handler);
    Signal(SIGCHLD, sigchld_handler);  /* Terminated or stopped child */
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
void eval(char *cmdline){
    char *argv[MAXARGS];
    char buf[MAXLINE];
    strcpy(buf, cmdline);
    int bg = parseline(buf, argv);
    if(argv[0] == NULL) return;
    
    if(builtin_cmd(argv)){
        return;
    }

    /* not built-in commands */
    pid_t cpid;
    
    sigset_t mask, prev_mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    sigaddset(&mask, SIGTSTP);

    sigprocmask(SIG_BLOCK, &mask, &prev_mask);
    /* parent must block SIGCHLD before fork;
       if not, when the parent adds this new child to the jobs list
       meanwhile another child terminates(then deletejob is called), race condition happens to the jobs list
       - See CSAPP 8.5.6
     */
    if((cpid = Fork()) == 0){ // child

        /* make sure: tsh process is the only process in the foreground process group(of bash shell)
        other spawned child processes from tsh process will be staying in their own process groups
         */
        setpgid(0, 0);
        
         printf("running child process : %d, group id %d\n", getpid(), getpgid(getpid()));


        // printf("contains: %d\n",sigismember(&prev_mask, SIGTSTP)); 0

        // sigset_t mask;
        // sigemptyset(&mask);
        // sigaddset(&mask, SIGCHLD);
        // sigprocmask(SIG_BLOCK, &mask, NULL);
        sigprocmask(SIG_SETMASK, &prev_mask, NULL);

        Execve(argv[0], argv, environ);

    }else{
        addjob(jobs, cpid, (bg? BG: FG), cmdline);

        /* must unblock SIGCHLD in parent after addjob, 
        otherwise when children terminate, SIGCHLD cannot be received by parent process, sigchld_handler cannot be triggered 
        */
        sigprocmask(SIG_SETMASK, &prev_mask, NULL);

        if(bg){
             printf("[%d] (%d) %s", pid2jid(cpid), cpid, cmdline);
             /* when bg job is done, how is the child process reaped? => sigchld_handler will be triggered */
        }else{
            /*  fg job, parent process blocked;
                delete fg job after it's complete(in sigchld_handler)
            */
            waitfg(cpid);
        }
    }
}

/*
 * waitfg - Block until process pid is no longer the foreground process
 */
void waitfg(pid_t pid){
    /* 
        busy wait according to the write-up
        when the fg job complete, sigchld_handler will be triggered to reap the child
    */
   struct job_t *fg_job;
   pid_t fg_job_pid;
    while( (fg_job_pid = fgpid(jobs)) != 0 
    && (fg_job = getjobpid(jobs, fg_job_pid)) != NULL
    && fg_job->state == FG){
        sleep(1);
    }
}

/*
 * builtin_cmd - If the user has typed a built-in command then execute
 *    it immediately.
 * 
 * builtin_cmd is executed in the current process
 */
int builtin_cmd(char **argv){
    char *cmd = argv[0];
    
    if(!strcmp(cmd, "quit")){
        exit(0);
    }
    if(!strcmp(cmd, "&")) return 1;
    
    if(!strcmp(cmd, "jobs")){
        listjobs(jobs);
        return 1;
    }
    if(!strcmp(cmd, "bg") || !strcmp(cmd, "fg")){
        do_bgfg(argv);
        return 1;
    }
    return 0;
}

/*
 * do_bgfg - Execute the builtin bg and fg commands
 */
void do_bgfg(char **argv){
    char *cmd = argv[0];
    
    if(!validate_id(argv)){
        return;
    }

    char *id = argv[1];
    struct job_t *cur_job;
    int groupId;
    
    if(id[0] == '%'){
        int jid = atoi(id + 1); // get rid of %
        cur_job = getjobjid(jobs, jid);
        if(cur_job == NULL || cur_job->state == UNDEF){
            printf("%s: No such job\n", id);
            return;
        }
    }else{
        int pid = atoi(id);
        cur_job = getjobpid(jobs, pid);
        if(cur_job == NULL || cur_job->state == UNDEF){
            printf("(%d): No such process\n", pid);
            return;
        }
    }

    groupId = getpgid(cur_job->pid);
 
    /* current process is the parent process
       when this process is making changes to the jobs list, is it necessary to block SIGCHLD here?
    */

    if(!strcmp(cmd, "bg")){ // make one job to bg => run it while ppid not blocking
        if(cur_job->state == ST){
            cur_job->state = BG;
            printf("[%d] (%d) %s", cur_job->jid, cur_job->pid, cur_job->cmdline);

            // printf("sending SIGCONT to process:%d, group: %d in bg...\n", cur_job->pid, getpgid(cur_job->pid));
            Kill(-groupId, SIGCONT);
            // kill(job->pid, SIGCONT);

        }
    }
    if(!strcmp(cmd, "fg")){ // make one job to fg => run it while ppid blocking until the job is done
        
        if(cur_job->state == ST){
            // printf("sending SIGCONT to process:%d, group: %d in fg...\n", cur_job->pid, getpgid(cur_job->pid));
            Kill(-groupId, SIGCONT);
        }
        cur_job->state = FG;

            // kill(-getpgid(job->pid), SIGCONT); //if job is ST, resume execution

        // sigprocmask(SIG_SETMASK, &prev_mask, NULL);

        waitfg(cur_job->pid);
    }
}

/* validate id following bg or fg */
int validate_id(char **argv){
    if(argv[1] == NULL || !strcmp(argv[1], "")){
        printf("%s command requires PID or %%jobid argument\n", argv[0]);
        return 0 ;
    }
    char *id = argv[1];
    if(!atoi(id) && !atoi(id + 1)){
        printf("%s: argument must be a PID or %%jobid\n", argv[0]);
        return 0;
    }
    return 1;
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
void sigchld_handler(int sig){
    printf("=== in sigchld_handler: %d, process id: %d, process group id: %d\n", sig, getpid(), getpgid(getppid()));
    pid_t cpid;
    int status; 
    // /* 
    //     这里如果用wait(&status)会出问题, wait(&status) 相当于 waitpid(-1, &status, 0) 
    //     waitpid default behavior only works for terminated children
    //     For stopped children, WUNTRACED must be used here
    //  */
    if((cpid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0){
        // printf("child process......%d\n", cpid);

        // printf("exited: %d\n", WIFEXITED(status));
        // printf("stopped: %d\n", WIFSTOPPED(status));
        // printf("sig: %d\n", WIFSIGNALED(status));

        if(WIFEXITED(status)){
            // printf("in sigchld_handler, terminated cpid: %d\n", cpid);
            deletejob(jobs, cpid);
        }
        if(WIFSTOPPED(status)){
            struct job_t *stopped_job = getjobpid(jobs, cpid);
            stopped_job->state = ST;
            printf("Job [%d] (%d) stopped by signal %d\n", stopped_job->jid, stopped_job->pid, SIGTSTP);
        }
        if(WIFSIGNALED(status)){
            deletejob(jobs, cpid);
        }
        // stopped process signalled to continue
        // if(WIFCONTINUED(status)){
        //     struct job_t *resumed_job = getjobpid(jobs, cpid);
        //     // printf("Job [%d] (%d) resumed by signal %d\n", resumed_job->jid, resumed_job->pid, sig);
        // }
    }
}

/* why this cannot be invoked */
// void sigcont_handler(int sig){
//     printf("sig in sigcont_handler: %d\n", sig);
//     pid_t cpid;
//     int status;
//     while((cpid = waitpid(-1, &status, WCONTINUED)) > 0){
//         if(WIFCONTINUED(status)){

//         }
//     }
// }

/*
 * sigint_handler - The kernel sends a SIGINT to the shell whenver the
 *    user types ctrl-c at the keyboard.  Catch it and send it along
 *    to the foreground job.
 */
void sigint_handler(int sig){
    pid_t foreground_pid;
    printf("sig int from where???\n");
    if((foreground_pid = fgpid(jobs)) == 0) return;

    struct job_t *fg_job = getjobpid(jobs, foreground_pid);
    if(fg_job->state == FG){
        printf("Job [%d] (%d) terminated by signal %d\n", fg_job->jid, fg_job->pid, sig);
    }
    int pgid = getpgid(foreground_pid);
    /* 
    kill the process group, this child process dies, 
    then SIGCHLD will be sent to handler which does the jobs list clean-up
    */
    Kill(-pgid, SIGKILL); 
}

/*
 * sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
 *     the user types ctrl-z at the keyboard. Catch it and suspend the
 *     foreground job by sending it a SIGTSTP.
 */
void sigtstp_handler(int sig){
    printf("=== in sigtstp_handler, sig: %d, from %d\n", sig, getpid());
    pid_t foreground_pid = fgpid(jobs);
    printf("sig tstp from where: %d\n", getpid());

    printf("foreground_pid: %d\n", foreground_pid);
    if(foreground_pid == 0) return;
    // fg job always belongs to a group whose id equals pid
    struct job_t *fg_job = getjobpid(jobs, foreground_pid);
    if(fg_job != NULL && fg_job->state == FG){
        pid_t pgid = getpgid(fg_job->pid);
        Kill(-pgid, SIGTSTP);
    }
}

/*
 * parseline - Parse the command line and build the argv array.
 *
 * Characters enclosed in single quotes are treated as a single
 * argument.  Return true if the user has requested a BG job, false if
 * the user has requested a FG job.
 */
int parseline(const char *cmdline, char **argv)
{
    static char array[MAXLINE]; /* holds local copy of command line */
    char *buf = array;          /* ptr that traverses command line */
    char *delim;                /* points to first space delimiter */
    int argc;                   /* number of args */
    int bg;                     /* background job? */

    strcpy(buf, cmdline);
    buf[strlen(buf)-1] = ' ';  /* replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* ignore leading spaces */
        buf++;

    /* Build the argv list */
    argc = 0;
    if (*buf == '\'') {
        buf++;
        delim = strchr(buf, '\'');
    }
    else {
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
        }
        else {
            delim = strchr(buf, ' ');
        }
    }
    argv[argc] = NULL;

    if (argc == 0)  /* ignore blank line */
        return 1;
    /* should the job run in the background? */
       if ((bg = (*argv[argc-1] == '&')) != 0) {
           argv[--argc] = NULL;
       }
       return bg;
   }


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
int maxjid(struct job_t *jobs)
{
    int i, max=0;

    for (i = 0; i < MAXJOBS; i++)
        if (jobs[i].jid > max)
            max = jobs[i].jid;
    return max;
}


/* addjob - Add a job to the job list */
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline){
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
            if(verbose){
                printf("Added job [%d] %d %s\n", jobs[i].jid, jobs[i].pid, jobs[i].cmdline);
            }
            return 1;
        }
    }
    printf("Tried to create too many jobs\n");
    return 0;
}

/* deletejob - Delete a job whose PID=pid from the job list */
int deletejob(struct job_t *jobs, pid_t pid){
    int i;

    if (pid < 1)
        return 0;

    for (i = 0; i < MAXJOBS; i++) {
        if (jobs[i].pid == pid) {
            clearjob(&jobs[i]);
            nextjid = maxjid(jobs)+1;
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
struct job_t *getjobjid(struct job_t *jobs, int jid)
{
    int i;

    if (jid < 1)
        return NULL;
    for (i = 0; i < MAXJOBS; i++)
        if (jobs[i].jid == jid)
            return &jobs[i];
    return NULL;
}

/* pid2jid - Map process ID to job ID */
int pid2jid(pid_t pid){
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
void listjobs(struct job_t *jobs){
    for (int i = 0; i < MAXJOBS; i++) {
        if (jobs[i].pid != 0) {
            printf("[%d] (%d) ", jobs[i].jid, jobs[i].pid);
            switch (jobs[i].state) {
                case BG:
                    printf("Running ");
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
void usage(void)
{
    printf("Usage: shell [-hvp]\n");
    printf("   -h   print this message\n");
    printf("   -v   print additional diagnostic information\n");
    printf("   -p   do not emit a command prompt\n");
    exit(1);
}

/*
 * unix_error - unix-style error routine
 */
void unix_error(char *msg){
    fprintf(stdout, "%s: %s\n", msg, strerror(errno));
    exit(1);
}

/*
 * app_error - application-style error routine
 */
void app_error(char *msg){
    fprintf(stdout, "%s\n", msg);
    exit(1);
}


/*
 * Signal - wrapper for the sigaction function
 */
handler_t *Signal(int signum, handler_t *handler){
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
void sigquit_handler(int sig){
    printf("Terminating after receipt of SIGQUIT signal\n");
    exit(1);
}

pid_t Fork(void) 
{
    pid_t pid;

    if ((pid = fork()) < 0)
	unix_error("Fork error");
    return pid;
}

void Execve(const char *filename, char *const argv[], char *const envp[]) 
{
    if (execve(filename, argv, envp) < 0)
	unix_error("Command not found");
}

void Kill(pid_t pid, int signum) {
    int rc;

    if ((rc = kill(pid, signum)) < 0)
	unix_error("Kill error");
}

void test_print(){
    printf("%d\n", WNOHANG); // 1
    printf("%d\n", WUNTRACED); // 2
    printf("%d\n", WCONTINUED); // 16
}