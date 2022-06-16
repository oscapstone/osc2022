#ifndef _SIGNAL_INNER_H_
#define _SIGNAL_INNER_H_

#include "kernel/signal.h"
struct sigaction default_actions[SIG_MAX_NUM] = {
    {SIG_FLAG_KERNEL, sig_default},
    {SIG_FLAG_KERNEL, sig_default},
    {SIG_FLAG_KERNEL, sig_default},
    {SIG_FLAG_KERNEL, sig_default},
    {SIG_FLAG_KERNEL, sig_default},
    {SIG_FLAG_KERNEL, sig_default},
    {SIG_FLAG_KERNEL, sig_default},
    {SIG_FLAG_KERNEL, sig_default},
    {SIG_FLAG_KERNEL, sig_default},
    {SIG_FLAG_KERNEL, sig_terminate}, // 9
    {SIG_FLAG_KERNEL, sig_default},
    {SIG_FLAG_KERNEL, sig_default},
    {SIG_FLAG_KERNEL, sig_default},
    {SIG_FLAG_KERNEL, sig_default},
    {SIG_FLAG_KERNEL, sig_default},
    {SIG_FLAG_KERNEL, sig_default},
    {SIG_FLAG_KERNEL, sig_default},
    {SIG_FLAG_KERNEL, sig_default},
    {SIG_FLAG_KERNEL, sig_default},
    {SIG_FLAG_KERNEL, sig_default},
    {SIG_FLAG_KERNEL, sig_default},
    {SIG_FLAG_KERNEL, sig_default},
    {SIG_FLAG_KERNEL, sig_default},
    {SIG_FLAG_KERNEL, sig_default},
    {SIG_FLAG_KERNEL, sig_default},
    {SIG_FLAG_KERNEL, sig_default},
    {SIG_FLAG_KERNEL, sig_default},
    {SIG_FLAG_KERNEL, sig_default},
    {SIG_FLAG_KERNEL, sig_default},
    {SIG_FLAG_KERNEL, sig_default},
    {SIG_FLAG_KERNEL, sig_default},
    {SIG_FLAG_KERNEL, sig_default}
};

char* sig_to_str[SIG_MAX_NUM] = {
    "SIGUNKOWN",
    "SIGHUP",
    "SIGINT",
    "SIGQUIT",
    "SIGILL",
    "SIGTRAP",
    "SIGABRT",
    "SIGBUS",
    "SIGFPE",
    "SIGKILL",
    "SIGUSR1",
    "SIGSEGV",
    "SIGUSR2",
    "SIGPIPE",
    "SIGALRM",
    "SIGTERM",
    "SIGSTKFLT",
    "SIGCHLD",
    "SIGCONT",
    "SIGSTOP",
    "SIGTSTP",
    "SIGTTIN",
    "SIGTTOU",
    "SIGURG",
    "SIGXCPU",
    "SIGXFSZ",
    "SIGVTALRM",
    "SIGPROF",
    "SIGWINCH",
    "SIGIO",
    "SIGPWR",
    "SIGSYS" 
};
#endif
