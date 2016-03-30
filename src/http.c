#include "log.h"
#include "http.h"
#include <stdio.h>
#include <string.h>


struct active_request_slot *defult_single_slot = 0;


int http_init(void)
{
    if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK)
    {
        LOG(FATAL, "curl init failed!!!");
        return HTTP_ERROR;
    }

    return HTTP_OK;

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
    if (!slot)
    {
        LOG(FATAL, "get_active_slot failed");    
        return HTTP_ERROR;
    }
    
    curl_easy_setopt(slot->curl, CURLOPT_HTTPGET, 1);

    if (target == HTTP_REQUEST_FILE)
    {
        curl_easy_setopt(slot->curl, CURLOPT_FILE, result); 
        curl_easy_setopt(slot->curl, CURLOPT_WRITEFUNCTION, fwrite); 
        
        posn = ftello(result);
        if (posn > 0)
        {
            vsnprintf(rangebuf, "%"PRIuMAX"-",(uintmax_t)posn);
            LOG(FATAL, "range %s", rangebuf);
            curl_easy_setopt(slot->curl, CURLOPT_RANGE, rangebuf); 
        }
    }

    ret = run_one_slot(slot);

    return ret;
}


struct active_request_slot *get_active_slot()
{
    struct active_request_slot *newslot = defult_single_slot;
    
    if (!newslot)
    {
        newslot = malloc(sizeof(*newslot));

        if (!newslot)
        {
            LOG(FATAL, "malloc failed");
            return 0;
        }
        
        newslot->curl = curl_easy 
    
    }



}
