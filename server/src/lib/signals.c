#include <stdio.h>
#include <signal.h>
#include <strings.h>
#include <string.h>

int *exit_flag;

static void exit_signal() {
    printf("Exit signal trapped...\n");

    *exit_flag = 1;
}

void trap_exit(int *exit) {
    struct sigaction action;

    memset(&action, 0, sizeof(action));
    action.sa_handler = exit_signal;
    exit_flag = exit;

    sigaction(SIGINT, &action, 0);
    sigaction(SIGTERM, &action, 0);
}
