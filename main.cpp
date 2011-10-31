// @(#)main.cpp
// Time-stamp: <Julian Qian 2011-10-31 21:12:44>
// Copyright 2011 Julian Qian
// Version: $Id: main.cpp,v 0.0 2011/06/12 05:04:14 jqian Exp $


#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <vector>
#include <fstream>
#include <algorithm>

#include "Logging.h"
#include "HttpGet.h"

#define AMAZON_PROXY "fints-g7g.amazon.com"
#define DOCUMENT_PATH "/mnt/us/documents/"
#define CONFIG_FILE "kindlepull.ini"
#define PID_FILE "/tmp/kindlepull.pid"

using std::string;

typedef std::vector<string> DownloadList;

typedef struct {
    string fsn;
    string whisper;
    string log;
} PullConfig;

string trim_string(string str){
    string::iterator itr = std::remove(str.begin(), str.end(), ' ');
    str.erase(itr, str.end());
    return str;
}

int get_config(const char* cfile, PullConfig& conf){
    std::ifstream cstrm;
    cstrm.open(cfile);
    if(! cstrm.is_open()){
        return -1;
    }
    string line;
    while(std::getline(cstrm, line)){
        // parse line
        if(!line.empty() && line[0] != '#'){
            size_t pos = line.find("=");
            if(pos != string::npos){
                string key = trim_string(line.substr(0, pos));
                if(key == "fsn"){
                    conf.fsn = trim_string(line.substr(pos+1));
                }else if(key == "whisper"){
                    conf.whisper = trim_string(line.substr(pos+1));
                }else if(key == "log"){
                    conf.log = trim_string(line.substr(pos+1));
                }
            }
        }
    }
    cstrm.close();
    return 0;
}

int url2file(string url, string& file){
    size_t start = url.find_last_of("/");
    if(start != string::npos){
        size_t end = url.find("?", start);
        file = DOCUMENT_PATH;
        file.append(url.substr(start + 1,
                end == string::npos ? string::npos : end -start -1));
        return 0;
    }
    return -1;
}

static int lock_file(int fd){
    struct flock fl;
    fl.l_type = F_WRLCK;
    fl.l_start = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len = 0;
    return fcntl(fd, F_SETLK, (long)&fl);
}

static int already_running(void){
    int fd = open(PID_FILE, O_RDWR|O_CREAT,
                  S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    if(fd < 0){
        fprintf(stderr, "Can't open %s: %s.\n", PID_FILE, strerror(errno));
        _exit(1);
    }

    if(lock_file(fd) < 0){
        if (errno == EACCES || errno == EAGAIN){
            close(fd);
            fprintf(stderr, "Another instance is running?\n");
            _exit(1);
        }
        fprintf(stderr, "Can't lock %s: %s.\n", PID_FILE, strerror(errno));
        _exit(1);
    }

    int rval = ftruncate(fd,0);

    char buf[16];
    sprintf(buf,"%d", getpid());
    rval = write(fd, buf, strlen(buf));
    if(rval < 0){
        close(fd);
        fprintf(stderr, "Failed to write pid to %s.\n", PID_FILE);
        _exit(1);
    }
    return fd;
}

int main(int argc, char *argv[]){
    int lockfd = already_running();

    PullConfig conf;
    if(-1 == get_config(CONFIG_FILE, conf) &&
       -1 == get_config("/mnt/us/" CONFIG_FILE, conf) &&
       -1 == get_config("/mnt/us/kindlepull/" CONFIG_FILE, conf)){
        fprintf(stderr, "failed to load conf file.\n");
        return -1;
    }
    if(conf.fsn.empty() || conf.whisper.empty()){
        fprintf(stderr, "please set fsn and whisper url.\n");
        return -1;
    }

    Logger::instance()->level(Logger::DEBUG_LEVEL);
    if(!conf.log.empty()){
        Logger::instance()->open(conf.log);
    }

    // query what to download
    DownloadList dlist;
    {
        string listText;
        HttpGet qlist(AMAZON_PROXY, conf.fsn.c_str());
        qlist.request(conf.whisper);
        qlist.gunzipText(listText);
        LDEBUG("gunzipText: %s", listText.c_str());
        size_t start = 0, end;
        while(1){
            end = listText.find("\n", start);
            if(end != string::npos){
                string line = listText.substr(start, end - start);
                if(line.find("http://") == 0){
                    dlist.push_back(line);
                }
            }else{
                break;
            }
            start = end + 1;    // skip delimiter \n
        }
    }
    LDEBUG("begin download...: %d.", dlist.size());
    // download them
    for (DownloadList::iterator itr = dlist.begin();
         itr != dlist.end(); ++itr) {
        string file, url = *itr;
        LINFO("download %s.\n", url.c_str());
        // validate download file
        HttpGet hget(AMAZON_PROXY, conf.fsn.c_str());
        hget.request(url.c_str());
        hget.gunzipFile(DOCUMENT_PATH);
    }

    // release lock file
    close(lockfd);
    unlink(PID_FILE);

    return 0;
}
