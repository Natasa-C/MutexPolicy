#include <stdio.h>     //printf(3)
#include <stdlib.h>    //exit(3)
#include <unistd.h>    //fork(3), chdir(3), sysconf(3)
#include <signal.h>    //signal(3)
#include <sys/stat.h>  //umask(3)
#include <sys/types.h> //pid_t
#include <syslog.h>    //syslog(3), openlog(3), closelog(3)
#include <string.h>
#include <zmq.h>

/*
To compile:  	cc -o mydaemon mydaemon.c -lzmq
To run:		    ./mydaemon
To check if everything is working properly: ps -xj | grep mydaemon
To terminate:	kill pid
To test log:	tail -f /tmp/mydaemon.log or cat /tmp/mydaemon.log
To read the syslog: grep mydaemon /var/log/syslog   ( mydaemon is the name given as parameter for daemonize function)
*/

#define LOG_FILE "mydaemon.log"
#define RUNNING_DIR "/tmp"

void log_message(filename, message) char *filename;
char *message;
{
    FILE *logfile;
    logfile = fopen(filename, "a");
    if (!logfile)
        return;
    fprintf(logfile, "%s\n", message);
    fclose(logfile);
}

void signal_handler(sig) int sig;
{
    switch (sig)
    {
    case SIGHUP:
        log_message(LOG_FILE, "hangup signal catched");
        break;
    case SIGTERM:
        log_message(LOG_FILE, "terminate signal catched");
        exit(0);
        break;
    }
}

int daemonize(char *name, char *path, char *infile, char *outfile, char *errfile)
{
    if (!name)
    {
        name = "medaemon";
    }
    if (!path)
    {
        path = "/";
    }

    if (!infile)
    {
        infile = "/dev/null";
    }
    if (!outfile)
    {
        outfile = "/dev/null";
    }
    if (!errfile)
    {
        errfile = "/dev/null";
    }
    //printf("%s %s %s %s\n",name,path,outfile,infile);

    /* Process ID and Session ID */
    pid_t child, sid;

    /* Fork off the parent process (detach from process group leader)*/
    child = fork();

    /* An error occurred */
    if (child < 0)
    {
        fprintf(stderr, "error: failed fork\n");
        exit(EXIT_FAILURE);
    }

    /* If we got a good PID, then we can exit the parent process. */
    if (child > 0)
    {
        exit(EXIT_SUCCESS);
    }

    /* On success: The child process becomes session leader */
    sid = setsid();
    if (sid < 0)
    { //failed to become session leader
        fprintf(stderr, "error: failed setsid\n");
        exit(EXIT_FAILURE);
    }

    /* Catch, ignore and handle signals */
    // Ignore the signals
    signal(SIGCHLD, SIG_IGN); /* ignore child */
    signal(SIGTSTP, SIG_IGN); /* ignore tty signals */
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    // Catch and handle the signals
    signal(SIGHUP, signal_handler);  /* catch hangup signal */
    signal(SIGTERM, signal_handler); /* catch kill signal */

    /* Fork off for the second time*/
    child = fork();

    /* An error occurred */
    if (child < 0)
    {
        fprintf(stderr, "error: failed fork\n");
        exit(EXIT_FAILURE);
    }

    /* Success: Let the parent terminate */
    if (child > 0)
        exit(EXIT_SUCCESS);

    /* Change the file mode mask. Set new file permissions */
    umask(0);

    /* Change the current working directory to the root directory or another appropriated directory */
    // chdir("/");
    chdir(path);

    /* Close out the standard file descriptors */
    // close(STDIN_FILENO);
    // close(STDOUT_FILENO);
    // close(STDERR_FILENO);

    // Close all open file descriptors
    int fd;
    for (fd = sysconf(_SC_OPEN_MAX); fd > 0; --fd)
    {
        close(fd);
    }

    // Reopen stdin, stdout, stderr
    stdin = fopen(infile, "r");    //fd=0
    stdout = fopen(outfile, "w+"); //fd=1
    stderr = fopen(errfile, "w+"); //fd=2

    // Open syslog
    openlog(name, LOG_PID, LOG_DAEMON);
    return (0);
}

int main()
{
    //  Socket to talk to clients
    void *context = zmq_ctx_new();
    if (context == NULL)
        return -1;

    void *responder = zmq_socket(context, ZMQ_REQ);
    if (responder == NULL)
        return -1;

    int rc = zmq_bind(responder, "tcp://*:5555");
    if (rc != 0)
        return 0;

    int res;
    int session_time_limit = 2000;
    int delay = 5;

    if ((res = daemonize("tester", RUNNING_DIR, NULL, NULL, NULL)) != 0)
    {
        fprintf(stderr, "error: daemonize failed\n");
        exit(EXIT_FAILURE);
    }

    syslog(LOG_NOTICE, "Daemon up and running. %d remaining time.", session_time_limit);
    syslog(LOG_NOTICE, "Waiting for messages");

    while (1)
    {
        //daemon code here
        // syslog(LOG_NOTICE, "%d remaining time.", session_time_limit);

        char buffer[50];
        zmq_recv(responder, buffer, 50, 0);
        syslog(LOG_NOTICE, "%s \n", buffer);
        sleep(1); //  Do some 'work'

        sleep(delay);
        session_time_limit -= delay;
    }

    syslog(LOG_NOTICE, "daemon session_time_limit expired");
    closelog();

    return (EXIT_SUCCESS);
}