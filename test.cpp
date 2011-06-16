// @(#)test.cpp
// Time-stamp: <Julian Qian 2011-06-15 17:30:01>
// Copyright 2011 Julian Qian
// Version: $Id: test.cpp,v 0.0 2011/06/14 03:51:29 jqian Exp $

#include <assert.h>
#include <iostream>
#include "Logging.h"
#include "HttpGet.h"

// #define AMAZON_PROXY "fints-g7g.amazon.com"
#define AMAZON_PROXY "betterexplained.com"

int main(int argc, char *argv[]){
    Logger::instance()->level(Logger::DEBUG_LEVEL);
    // std::string logfile = "/tmp/kindlepull.log";
    // Logger::instance()->open(logfile);
    HttpGet qlist(AMAZON_PROXY);
    // qlist.request("http://betterexplained.com/examples/compressed/index.php");
    qlist.request("http://betterexplained.com/examples/compressed/index.htm");
    std::string text;
    int ret = qlist.gunzipFile("test.txt");
    assert(ret == 0);
    ret = qlist.gunzipText(text);
    assert(ret == 0);
    std::cout << text << std::endl;
}


