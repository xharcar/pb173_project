#include "world_shared.h"

/* Per-thread variable pointing to the signal handler function */
static thread_local signal_handler_t thread_handler = nullptr;

static void signal_handler(int signum, siginfo_t* info, void* context)
{
    signal_handler_t func;

    // fixme: use std::atomic
    func = __sync_fetch_and_or(&thread_handler, (signal_handler_t)0);

    if (func) {
        func(signum, info, context);
    }
}

void set_up_thread_hadler(signal_handler_t new_thread_handler)
{
    signal_handler_t  func;

    do {
        func = thread_handler;
    } while (!__sync_bool_compare_and_swap(&thread_handler, func, new_thread_handler));
}

void process_signal_handling()
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    // sa.sa_sigaction  = World::set_world_signal_status;
    sa.sa_sigaction = signal_handler;
    /* SA_SIGINFO flag must be set to use sa_sigaction handler */
    sa.sa_flags = SA_SIGINFO;
    sigfillset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGUSR1, &sa, NULL);
}

Log::Log(std::string ident_, int facility_, int priority_) {
    facility = facility_;
    priority = priority_;
    strncpy(ident, ident_.c_str(), sizeof(ident_));
    ident[sizeof(ident)-1] = '\0';

    openlog(ident, LOG_PID, facility);
}

Log::~Log() {
    closelog();
}

int Log::sync() {
    if (buffer.length()) {
        syslog(priority, buffer.c_str());
        buffer.erase();
        // priority = LOG_DEBUG; // default to debug for each message
    }
    return 0;
}

int Log::overflow(int c) {
    if (c != EOF) {
        buffer += static_cast<char>(c);
    } else {
        sync();
    }
    return c;
}
