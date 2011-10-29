// @(#)HttpGet.cpp
// Time-stamp: <Julian Qian 2011-10-29 19:16:04>
// Copyright 2011 Julian Qian
// Version: $Id: HttpGet.cpp,v 0.0 2011/06/12 05:07:03 jqian Exp $

#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>
#include <errno.h>

#include <algorithm>

#include "gun.h"
#include "Logging.h"
#include "HttpGet.h"

#define MAX_HEADER_LENGTH 1024
#define PORT 80
#define CRLF "\r\n"
#define CLOSEFP(fp) if(fp){ fclose(fp); fp = NULL; }
#define CLOSESOCK(sock) if(sock!=-1){ close(sock); sock = -1; }

using std::string;

HttpGet::HttpGet(const char* proxy, const char* fsn)
    : sockfd_(-1), tmpfile_(tmpfile()), fsn_(fsn), filename_(), gziped_(false) {
    struct sockaddr_in serv_addr;
    struct hostent *server;

    sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd_ < 0) {
        LERROR("failed to open socket.\n");
    }else{
        server = gethostbyname(proxy);
        if (server == NULL) {
            LERROR("no such host.\n");
        }else{
            bzero((char *) &serv_addr, sizeof(serv_addr));
            serv_addr.sin_family = AF_INET;
            bcopy((char *)server->h_addr,
                  (char *)&serv_addr.sin_addr.s_addr,
                  server->h_length);
            serv_addr.sin_port = htons(PORT);
            if (connect(sockfd_,
                    (struct sockaddr *) &serv_addr,
                    sizeof(serv_addr)) < 0){
                LERROR("failed to connect\n");
            }else{
                LINFO("connected");
            }
        }
    }
}

HttpGet::~HttpGet(){
    CLOSESOCK(sockfd_);
    CLOSEFP(tmpfile_);
}

static string get_hostname(const string& url){
    size_t start = 0, end, n = string::npos;
    if(url.compare(0, 7, "http://") == 0){
        start = 7;
    }
    end = url.find('/', start);
    if(end != string::npos){
        n = end - start;
    }
    return url.substr(start, n);
}

static string trim_string(const char* str, int len){
    const char* newstr = str;
    int newlen = len;
    for (int i = 0; i < len; ++i) {
        if(str[i] == ' '){
            ++ newstr;
            -- newlen;
        }else{
            break;
        }
    }
    for (int i = len-1; i > 0; --i) {
        if(str[i] == ' '){
            -- newlen;
        }else{
            break;
        }
    }
    string ret(newstr, newlen);
    return ret;
}

static string key_string(const char* str, int len){
    string ret = trim_string(str, len);
    std::transform(ret.begin(), ret.end(), ret.begin(), ::tolower);
    return ret;
}

static int get_content_length(const string& str){
    return atoi(str.c_str());
}

static int hex2dec(const char* str, int len){
    assert(len < 8);
    char hex[10] = "0x";
    memcpy(hex+2, str, len);
    hex[len+2] = '\0';
    return int(strtol(hex, NULL, 0));
}

static int string_find(const char* str, int slen, const char* search, int selen){
    int i = 0, j = 0;
    while(i < slen && j < selen) {
        if(*(str+i) == *(search+j)) {
            int m=i+1, n=j+1;
            while(m < slen && n < selen) {
                if(*(str+m)==*(search+n)) {
                    m++;
                    n++;
                } else {
                    break;
                }
            }
            if(n == selen) return m - selen;
            ++i;
        } else {
            ++i;
        }
    }
    return -1;
}

static string basename(const string& url){
    size_t found = url.find_last_of("/");
    if(found != string::npos && found < url.length())
        return url.substr(found+1);
    LERROR("failed to strip basename from %s.", url.c_str());
    return "Unknown";
}

int
HttpGet::request(const string& url){
    char buffer[MAX_HEADER_LENGTH];
    int buflen;

    filename_ = basename(url);
    // start to request
    //
    snprintf(buffer, MAX_HEADER_LENGTH,
             "GET %s HTTP/1.1" CRLF
             "Accept: image/png, image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, */*" CRLF
             "Host: %s" CRLF
             "User-Agent: Mozilla/4.0 (compatible; Linux 2.6.22) NetFront/3.4 Kindle/2.5 (screen 824x1200; rotate)" CRLF
             "Proxy-Connection: Keep-Alive" CRLF
             "Accept-Encoding: deflate, gzip" CRLF
             "x-fsn: \"%s\"" "\n" // not CRLF, ken die a!
             "x-appNamespace: WEB_BROWSER" "\n"
             "x-appId: Kindle_2.5" CRLF
             CRLF,
             url.c_str(), get_hostname(url).c_str(), fsn_);
    ssize_t wlen = write(sockfd_, buffer, strlen(buffer));
    if (wlen < 0){
        LERROR("failed to write to socket");
        return -1;
    }

    // reterive HTTP header
    //
    int ptr;
    for (ptr = 0; ptr < MAX_HEADER_LENGTH; ++ptr) {
        if(read(sockfd_, buffer + ptr, 1) < 0){
            LERROR("failed to read socket, %s.\n", strerror(errno));
            return -1;
        }
        if(ptr > 4 && (0 == strncmp(buffer + ptr - 3, CRLF CRLF, 4))){
            // end header
            break;
        }
    }
    buflen = ptr + 1;

    // parse header
    //
    int start = 0, end, len;
    char rescode[4];
    bzero(rescode,4);
    strncpy(rescode, buffer + 9, 3); // eg: HTTP/1.1 200
    // TODO: check HTTP response code 200, 403 ...
    bool chunked = false;
    int contentLength = 0;
    while(1){
        // delimit by \r\n
        len = string_find(buffer + start, buflen - start, CRLF, 2);
        if(len <= 0){           // double \r\n split header and body
            break;
        }
        end = start + len;
        // delimit by :
        len = string_find(buffer + start, end - start, ":", 1);
        if(len < 0){
            // maybe the first line.
        }else{
            string key = key_string(buffer + start, len);
            string value = trim_string(buffer + start + len +1, end - start - len -1 );
            if(key == "content-length"){
                contentLength = get_content_length(value);
            }else if(key == "content-encoding"){
                gziped_ = (value == "gzip") ? true : false;
            }else if(key == "transfer-encoding"){
                chunked = (value == "chunked") ? true : false;
            }else if(key == "content-disposition"){
                // attachment, obtain its filename, eg:
                // Content-Disposition: attachment; filename=fname.ext
                //
                int aoff = start + len + 1;
                int alen = string_find(buffer + aoff,
                        end - aoff, "=", 1);
                if(alen > 0){
                    aoff += alen;
                    filename_ = trim_string(buffer + aoff, end - aoff);
                }else{
                    LERROR("Failed to fetch attachment filename.");
                    filename_ = "Unknown";
                }
            }
        }

        // update iterator
        start = end + 2;        // skip \r\n
    }

    // reterive body
    //
    if(contentLength > 0){
        int remaining = contentLength;
        // FIXME: why rerieve different content when debug mode, if more than 1b?!
        do {
            int fetchsize = (remaining > MAX_HEADER_LENGTH) ? MAX_HEADER_LENGTH : remaining;
            int fetched = read(sockfd_, buffer, fetchsize);
            if(fetched < 0){
                LERROR("failed to reterive content. %s\n", strerror(errno));
                return -1;
            }
            if(fetched == 0){
                LERROR("reach end early...something wrong?");
            }
            if(fwrite(buffer, 1, fetched, tmpfile_) != size_t(fetched)){
                LERROR("failed to write content to tmpfile. %s\n", strerror(errno));
                return -1;
            }
            remaining -= fetched;
        } while(remaining > 0);
        LINFO("fetch content length %d.", contentLength);
    }else if(chunked){
        int chunksize = 0;
        do {
            // read chunk size
            for (ptr = 0; ptr < 10; ++ptr) {
                int ret = read(sockfd_, buffer + ptr, 1);
                if(ret < 0){
                    LERROR("failed to reterive trunk size. %s\n", strerror(errno));
                    return -1;
                }
                if(ret == 0){
                    LINFO("reach end.\n");
                    break;
                }
                if(strncmp(buffer + ptr -1, CRLF, 2) == 0){
                    break;
                }
            }
            buflen = ptr + 1;
            // read chunk body
            chunksize = hex2dec(buffer, buflen - 2);
            LDEBUG("chunksize: %d\n", chunksize);
            if(chunksize > 0){
                int remaining = chunksize;
                do {
                    int fetchsize = (remaining > MAX_HEADER_LENGTH) ? MAX_HEADER_LENGTH : remaining;
                    int fetched = read(sockfd_, buffer, fetchsize);
                    if( fetched < 0){
                        LERROR("failed to reterive trunk. %s\n", strerror(errno));
                        return -1;
                    }
                    if(fetched == 0){
                        LERROR("reach end early...something wrong?");
                    }
                    if(fwrite(buffer, 1, fetched, tmpfile_) != size_t(fetched)){
                        LERROR("failed to write chunk to tmpfile. %s\n", strerror(errno));
                        return -1;
                    }
                    remaining -= fetched;
                } while(remaining > 0);

            }
            // read chunk delimiter CRLF
            read(sockfd_, buffer, 2);

        } while(chunksize == 0);
        LINFO("reach chunk's end.\n");
    }

    // restore download file
    //
#if 0
    string tmpgz = this->filename_ + ".gz";
    FILE* fp = fopen(tmpgz.c_str(), "w");
    rewind(tmpfile_);
    while(!feof(tmpfile_)){
        int tlen = fread(buffer, 1, 1024, tmpfile_);
        fwrite(buffer, 1, tlen, fp);
    }
    fclose(fp);
#endif

    return 0;
}

int
HttpGet::gunzipText(string& text){
    int ret = 0;

    if(gziped_){
        FILE* tmp = tmpfile();
        rewind(tmpfile_);
        ret = gunzip_file(tmpfile_, tmp);
        if(ret){
            LERROR("failed to ungzip text.\n");
        }
        rewind(tmp);
        char buffer[1024];
        while(!feof(tmp)){
            int rlen = fread(buffer, 1, 1024, tmp);
            text.append(buffer, rlen);
        }
        CLOSEFP(tmp);
    }else{
        rewind(tmpfile_);
        char buffer[1024];
        while(!feof(tmpfile_)){
            int rlen = fread(buffer, 1, 1024, tmpfile_);
            text.append(buffer, rlen);
        }
    }
    return ret;
}

int
HttpGet::gunzipFile(const char* dir){
    int ret = 0;

    string file(dir);
    file.append(filename_);
    FILE* fp = fopen(file.c_str(), "w");
    if(fp == NULL){
        LERROR("failed to open %s\n", file.c_str());
        return -1;
    }

    // read from temporay file
    rewind(tmpfile_);

    if(gziped_){
        ret = gunzip_file(tmpfile_, fp);
        if(ret){
            LERROR("Failed to ungzip to %s\n", file.c_str());
        }

    }else{
        char buffer[1024];
        while(!feof(tmpfile_)){
            int rlen = fread(buffer, 1, 1024, tmpfile_);
            fwrite(buffer, 1, rlen, fp);
        }
        if(ferror(tmpfile_)){
            LERROR("Failed to copy to %s\n", file.c_str());
            ret = -1;
        }
    }

    CLOSEFP(fp);
    if(ret) unlink(file.c_str());

    return ret;
}
