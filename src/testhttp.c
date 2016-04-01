#include "log.h"
#include "http.h"


int main()
{
    log_init();
    http_init();
    LOG(FATAL, "hello, dashcopy!\n");
    http_get_file("http://dash.edgesuite.net/dash264/TestCases/1a/netflix/exMPD_BIP_TC1.mpd","xxx.mpd");
    http_cleanup();
    log_cleanup();
}


