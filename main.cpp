// @(#)main.cpp
// Time-stamp: <Julian Qian 2011-08-29 19:11:39>
// Copyright 2011 Julian Qian
// Version: $Id: main.cpp,v 0.0 2011/06/12 05:04:14 jqian Exp $


#include <vector>
#include <fstream>
#include <algorithm>

#include "Logging.h"
#include "HttpGet.h"

#define AMAZON_PROXY "fints-g7g.amazon.com"
#define DOCUMENT_PATH "/mnt/us/documents/"
#define CONFIG_FILE "/mnt/us/kindlepull.ini"

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

int main(int argc, char *argv[]){
    PullConfig conf;
    if(get_config(CONFIG_FILE, conf)){
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
        size_t start = 0, end;
        while(1){
            end = listText.find("\n", start);
            if(end != string::npos){
                string line = listText.substr(start, end - start);
                dlist.push_back(line);
            }else{
                break;
            }
            start = end + 1;    // skip delimiter \n
        }
    }

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

    return 0;
}
