#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include "log.h"
#include "http.h"
#include "dashsrc.h"
#include "mpdparser.h"


int main(int argc, char **argv)
{
    FILE *mpdfile = 0;
    int mpdfile_len = 0;
    char *mpd_buffer = 0;
    struct MPDNode *mpd = 0;

    log_init();
    http_init();

    LOG(FATAL, "dashcopy: an utility that can copy MPEG DASH content from original server \n");
    LOG(FATAL, "usage: ./dashcopy the_mpd_url");

    if (argc != 2)
    {
        LOG(FATAL, "invalid input");
        LOG(FATAL, "usage: ./dashcopy the_mpd_url");
        goto exit;
    }

    if (dashsrc_get_mpdfile(argv[1]))
    {
        LOG(FATAL, "get mpd file failed");
        goto exit;
    }

    LOG(FATAL, "mpd file writen to %s", DASHCOPY_MPD_FILE_NAME);

    mpdfile = fopen(DASHCOPY_MPD_FILE_NAME, "rb");
    if (!mpdfile)
    {
        LOG(FATAL, "mpd file fopen failed");
        goto exit;
    }
    fseek(mpdfile, 0, SEEK_END);
    mpdfile_len = ftell(mpdfile);
    fseek(mpdfile, 0, SEEK_SET);

    mpd_buffer = (char *)malloc(mpdfile_len + 1);
    memset(mpd_buffer, 0, mpdfile_len + 1);
    if (!mpd_buffer)
    {
        LOG(FATAL, "malloc failed");
        goto exit;
    }
    fread(mpd_buffer, mpdfile_len, 1, mpdfile);

    LOG(FATAL, "read in mpd file to buffer");

    if (mpdparser_parse_mpd_buffer(&mpd, mpd_buffer, mpdfile_len, argv[1]))
    {
        LOG(FATAL, "parse mpd failed");
        goto exit;
    }

    dashsrc_copy_all(mpd);
    
exit:
    http_cleanup();
    log_cleanup();

    if (mpdfile)
        fclose(mpdfile);

    if (mpd)
        mpdparser_free_mpd_node(mpd);

    return 0;
}


