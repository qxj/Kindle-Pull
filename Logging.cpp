// @(#)Logging.cpp
// Time-stamp: <Julian Qian 2011-06-13 12:41:43>
// Copyright 2011 Julian Qian
// Version: $Id: Logging.cpp,v 0.0 2011/06/12 05:13:40 jqian Exp $

#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>

#include "Logging.h"

using std::string;

Logger* Logger::instance_ = NULL;

Logger*
Logger::instance(){
    if(instance_ == NULL){
        instance_ = new Logger();
    }
    return instance_;
}

int
Logger::open(string file){
    logPath_ = file;
    logf_ = ::fopen(file.c_str(), "a");
    if (logf_ == NULL) {
        return -1;
    }
    ::setlinebuf(logf_);
    return 0;
}

int
Logger::close(){
    if (logf_ != NULL){
        ::fclose(logf_);
    }
    logf_ = NULL;
    return 0;
}

static void timestring(char *buf, int bufsize){
    struct timeval tval;
    struct tm tmval;
    gettimeofday(&tval, NULL);
    localtime_r(&tval.tv_sec, &tmval);
    strftime(buf, bufsize, "%T", &tmval);

    int bufused = strlen(buf);
    snprintf(&buf[bufused], bufsize - bufused, ".%06lu", tval.tv_usec);
}

int
Logger::hex_dump(int lvl, const char* data, const int len,
                 const char* prompt){
    int outlen = 3 * len + (len/16 + 1) + 1;
    char* out = (char*)malloc(outlen);
    int i, outp = 0, ret;
    unsigned char c;
    for (i = 0; i < len; ++i) {
        c = static_cast<unsigned char>(data[i]);
        sprintf(out + outp, "%02X ", c);
        outp += 3;
        if(!((i + 1) % 16)){
            sprintf(out + outp, "\n");
            ++ outp;
        }
    }
    if(prompt){
        ret = this->logf(lvl, NULL, 0, NULL, "%s:\n%s", prompt, out);
    }else{
        ret = this->logf(lvl, NULL, 0, NULL, "%s", out);
    }
    free(out);
    return ret;
}

int
Logger::logf(int lvl, const char* file, const int line,
             const char* func, const char *fmt, ...){
    va_list args;
    char buf[1024];

    if (lvl < level_) return 0;

    // timestamp
    timestring(buf, 1024);

    // log level & position
    int len = strlen(buf);
    snprintf(&buf[len], 1024 - len, " [%d] ", lvl);

    // position
    if(file && line && func){
        len = strlen(buf);
        snprintf(&buf[len], 1024 - len, "%s:%d(%s) ", file, line, func);
    }

    // fmt
    len = strlen(buf);
    va_start(args, fmt);
    vsnprintf(&buf[len], 1024 - len, fmt, args);
    va_end(args);

    // auto wrap
    len = strlen(buf);
    if (buf[len-1] != '\n') {
        strncat(buf, "\n", 1024);
    }

    if (logf_ != NULL) {
        fputs(buf, logf_);
    }
    if (console_) ::write(1, buf, 1024);
    return 0;
}

int
Logger::level(int newlvl){
    int old = level_;
    level_ = newlvl;
    return old;
}

