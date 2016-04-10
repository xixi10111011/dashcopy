#include "dashsrc.h"
#include "mpdparser.h"


int dashsrc_get_file_with_retry(const char *url, const char *filename)
{
    for (int i = 0; i < DASHSRC_HTTP_TRY_TIMES; i++)
    {
        LOG(FATAL, "try times[%d] http request file[%s] from url[%s] ", i + 1; filename, url);
        if (!http_get_file(url, filename))
            return DASHSRC_OK; 
    }

    return DASHSRC_ERROR;
}


int dashsrc_get_mpdfile(const char *mpdurl)
{

    if (!dashsrc_get_file_with_retry(mpdurl, DASHCOPY_MPD_FILE_NAME))
        return DASHSRC_OK;
    return DASHSRC_ERROR;
}

void dashsrc_copy_all(struct MPDNode *mpd)
{
    struct PeriodNode *cur_period = 0;



}

