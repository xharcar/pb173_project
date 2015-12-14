#include "world_shared.h"

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
