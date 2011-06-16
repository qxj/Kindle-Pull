/* @(#)Logging.h -*- mode: c++ -*-
 * Time-stamp: <Julian Qian 2011-06-13 12:39:02>
 * Copyright 2011 Julian Qian
 * Version: $Id: Logging.h,v 0.0 2011/06/12 05:10:02 jqian Exp $
 */

#ifndef _LOGGING_H
#define _LOGGING_H 1

#include <cstdio>
#include <string>

#define LDEBUG(FMT, ...) Logger::instance()->logf(Logger::DEBUG_LEVEL,  \
        __FILE__, __LINE__,                                             \
        __FUNCTION__, FMT,                                              \
        ##__VA_ARGS__)
#define LINFO(FMT, ...) Logger::instance()->logf(Logger::INFO_LEVEL,    \
        NULL, 0, NULL,                                                  \
        FMT, ##__VA_ARGS__)
#define LERROR(FMT, ...) Logger::instance()->logf(Logger::ERROR_LEVEL,  \
        __FILE__, __LINE__,                                             \
        __FUNCTION__, FMT,                                              \
        ##__VA_ARGS__)
#define LFATAL(FMT, ...) Logger::instance()->logf(Logger::FATAL_LEVEL,  \
        NULL, 0, NULL,                                                  \
        FMT, ##__VA_ARGS__)
#define LDUMP(DATA, LEN, PROMPT)                                        \
    Logger::instance()->hex_dump(Logger::DEBUG_LEVEL, DATA, LEN, PROMPT)

class Logger
{
public:
    enum {
        DEBUG_LEVEL,
        INFO_LEVEL,
        ERROR_LEVEL,
        FATAL_LEVEL,
        NUM_LOG_LEVELS
    };

    friend class LogAppender;

    static Logger* instance();
    ~Logger(){ close(); }

    int open(std::string file);
    int close();

    int logf(int lvl, const char* file, const int line,
             const char* func, const char *fmt, ...);

    // hex dump
    int hex_dump(int lvl, const char* data, const int len,
                 const char* prompt = NULL);

    // print log to stdout
    void console(bool setting = false) { console_ = setting; }
    // set log level, return old level
    int level(int lv);

private:
    Logger()
        : logPath_(),
          logf_(stdout),
          console_(false),
          level_(ERROR_LEVEL){}

    std::string logPath_;
    FILE *logf_;

    bool console_;
    int level_;

    static Logger *instance_;
};

#endif /* _LOGGING_H */
