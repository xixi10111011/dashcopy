#ifndef _HTTP_H_
#define _HTTP_H_

#include <curl/curl.h>
#include <curl/easy.h>

/* external */

#define HTTP_OK 0
#define HTTP_ERROR 1

extern int http_init(void);
extern void http_cleanup(void);
extern int http_get_file(const char *url, const char *filename);



/* internal */

struct active_request_slot {
    CURL *curl;
    CURLcode curl_result;
    long http_code;
    char url[256];
};
struct active_request_slot *get_active_slot(void);
void free_active_slot(struct active_request_slot *slot);
int run_one_slot(struct active_request_slot *slot);

#define HTTP_REQUEST_FILE    0
#define HTTP_REQUEST_BUFFER  1


int http_request(const char *url, void *result, int target);










#endif /* _HTTP_H_ */
