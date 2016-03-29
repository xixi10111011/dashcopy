#include "log.h"
#include "http.h"

int http_init(void)
{
    if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK)
    {
        LOG(FATAL, "curl init failed!!!");
        return HTTP_ERROR;
    }

}
