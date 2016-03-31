#include "log.h"
#include "http.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <inttypes.h>



char curl_errstr[CURL_ERROR_SIZE];


int http_init(void)
{
    if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK)
    {
        LOG(FATAL, "curl init failed!!!");
        return HTTP_ERROR;
    }

    return HTTP_OK;

}

void http_cleanup()
{
    curl_global_cleanup();
}

int http_get_file(const char *url, const char *filename)
{
    FILE *fileptr;
    int ret;
    char *tmpfile;
    
    tmpfile = (char *)malloc(strlen(filename)+10);
    if (!tmpfile)
    {
        LOG(FATAL, "malloc failed");
        ret = HTTP_ERROR;
        goto cleanup;
    
    }
    memset(tmpfile, 0, strlen(filename)+10);

    sprintf(tmpfile, "%s.temp", filename);
    fileptr = fopen(tmpfile, "a");
    if (!fileptr)
    {
        LOG(FATAL, "fopen failed");
        ret = HTTP_ERROR;
        goto cleanup;
    } 

    ret = http_request(url, fileptr, HTTP_REQUEST_FILE);
    fclose(fileptr);

    if (ret == HTTP_OK)
    {
        if (rename(tmpfile, filename))
        {
            LOG(FATAL, "rename failed %s", strerror(errno)); 
        }
    
    }
cleanup:
    if (tmpfile)
    {
        free(tmpfile);    
    }

    return ret;

}



int http_request(const char *url, void *result, int target)
{
    struct active_request_slot *slot;
    int ret;
    off_t posn; 
    char rangebuf[128];


    slot = get_active_slot();
    strncpy(slot->url, url, sizeof(slot->url));

    if (!slot)
    {
        LOG(FATAL, "get_active_slot failed");    
        return HTTP_ERROR;
    }
    
    curl_easy_setopt(slot->curl, CURLOPT_HTTPGET, 1);
    curl_easy_setopt(slot->curl, CURLOPT_URL, url);

    if (target == HTTP_REQUEST_FILE)
    {
        curl_easy_setopt(slot->curl, CURLOPT_FILE, result); 
        curl_easy_setopt(slot->curl, CURLOPT_WRITEFUNCTION, fwrite); 
        
        posn = ftello(result);
        if (posn > 0)
        {
            snprintf(rangebuf, 128,  "%"PRIuMAX"-",(uintmax_t)posn);
            LOG(FATAL, "range %s", rangebuf);
            curl_easy_setopt(slot->curl, CURLOPT_RANGE, rangebuf); 
        }
    }

    ret = run_one_slot(slot);

    free_active_slot(slot);

    return ret;
}


struct active_request_slot *get_active_slot()
{
    struct active_request_slot *newslot;
    
    newslot = malloc(sizeof(*newslot));

    if (!newslot)
    {
        LOG(FATAL, "malloc failed");
        return 0;
    }
    
    newslot->curl = curl_easy_init();
    newslot->curl_result = 0;
    newslot->http_code = 0;
    memset(newslot->url, 0, sizeof(newslot->url));

    curl_easy_setopt(newslot->curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(newslot->curl, CURLOPT_SSL_VERIFYHOST, 0);
    curl_easy_setopt(newslot->curl, CURLOPT_LOW_SPEED_LIMIT, 100);
    curl_easy_setopt(newslot->curl, CURLOPT_LOW_SPEED_TIME, 60);
    curl_easy_setopt(newslot->curl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(newslot->curl, CURLOPT_TCP_KEEPALIVE, 1);
    curl_easy_setopt(newslot->curl, CURLOPT_ERRORBUFFER, curl_errstr);

    if (getenv("http_proxy"))
    {
        LOG(INFO, "using http_proxy");    
        curl_easy_setopt(newslot->curl, CURLOPT_PROXY, getenv("http_proxy"));
    }

    return newslot;
}


void free_active_slot(struct active_request_slot *slot)
{
    curl_easy_cleanup(slot->curl);
    
    free(slot);
}

int run_one_slot(struct active_request_slot *slot)
{
    slot->curl_result = curl_easy_perform(slot->curl);
    curl_easy_getinfo(slot->curl, CURLINFO_HTTP_CODE, &slot->http_code);
    
    if (slot->curl_result == CURLE_OK)
    {
        LOG(FATAL, "download succeed[%s] httpcode[%ld] ", slot->url, slot->http_code);
        return HTTP_OK;
    }
    else
    {
        LOG(FATAL, "download failed[%s] curlcode[%d] httpcode[%ld] curl_error_str[%s]", slot->url, slot->curl_result, slot->http_code, curl_errstr);
        return HTTP_ERROR;
    }
}
