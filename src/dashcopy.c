#include "log.h"
#include "http.h"
#include "dashsrc.h"


int main(int argc, char **argv)
{
    log_init();
    http_init();

    LOG(FATAL, "dashcopy: an utility that can copy MPEG DASH content from original server \n");
    LOG(FATAL, "usage: ./dashcopy mpd_url");

    if (argc != 2)
    {
        LOG(FATAL, "invalid input");
        LOG(FATAL, "usage: ./dashcopy mpd_url");
        goto error;
    }

    if (dashsrc_get_mpdfile(argv[1]))
    {
        LOG(FATAL, "get mpd file failed");
        goto error;
    }
    
error:
    http_cleanup();
    log_cleanup();
    return 0;
}


