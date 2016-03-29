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











#endif /* _HTTP_H_ */
