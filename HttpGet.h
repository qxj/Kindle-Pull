/* @(#)HttpGet.h -*- mode: c++ -*-
 * Time-stamp: <Julian Qian 2011-06-21 23:36:28>
 * Copyright 2011 Julian Qian
 * Version: $Id: HttpGet.h,v 0.0 2011/06/12 05:05:46 jqian Exp $
 */

#ifndef _HTTPGET_H
#define _HTTPGET_H 1

#include <string>

class HttpGet {
public:
    HttpGet(const char* proxy, const char* fsn);
    ~HttpGet();
    // for simple, only blocking socket
    int request(const std::string& url);
    // for content-length
    int getText(std::string& text);
    // for chunked attachment
    int getFile(const char* filepath);

private:
    int sockfd_;
    FILE* tmpfile_;
    const char* fsn_;
    bool gzip_;
};

#endif /* _HTTPGET_H */





