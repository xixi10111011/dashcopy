#include "dashsrc.h"
#include <stdlib.h>
#include "log.h"
#include "http.h"


void dashsrc_copy_period(struct PeriodNode * cur_period, long long period_duration);
void dashsrc_copy_adaptationset(struct AdaptationSetNode *cur_adpt, long long  period_duration);
void dashsrc_copy_representation(struct RepresentationNode *cur_rep, long long period_duration);

int dashsrc_get_file_with_retry(const char *url, const char *filename)
{
    for (int i = 0; i < DASHSRC_HTTP_TRY_TIMES; i++)
    {
        LOG(FATAL, "try times[%d] http request file[%s] from url[%s] ", i + 1, filename, url);
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
    struct PeriodNode *cur_period = mpd->Periods;
    long long period_duration = 0;
    long long total_duration = 0;

    while (cur_period)
    {
        if (cur_period->start == -1) 
        {
            cur_period->start = total_duration;
        
        }
        if (cur_period->duration != -1)
        {
            period_duration = cur_period->duration;
        }
        else if (cur_period->next && cur_period->next->start != -1)
        {
            period_duration = cur_period->next->start - cur_period->start;
        }
        else if (mpd->mediaPresentationDuration != -1)
        {
            period_duration = mpd->mediaPresentationDuration - cur_period->start;
        }
        else 
        {
            LOG(FATAL, "can not determine period duration");
            return;
        }
        total_duration += period_duration;

        dashsrc_copy_period(cur_period, period_duration);

        cur_period = cur_period->next;
    }

    return;
}


void dashsrc_copy_period(struct PeriodNode * cur_period, long long period_duration)
{
    struct AdaptationSetNode *cur_adpt = cur_period->AdaptationSets;

    while (cur_adpt)
    {
        dashsrc_copy_adaptationset(cur_adpt, period_duration);
    
        cur_adpt = cur_adpt->next;
    }
}


void dashsrc_copy_adaptationset(struct AdaptationSetNode *cur_adpt, long long  period_duration)
{
    struct RepresentationNode *cur_rep = cur_adpt->Representations;


    while (cur_rep)
    {
        dashsrc_copy_representation(cur_rep, period_duration);

        cur_rep = cur_rep->next;
    }

}


void dashsrc_copy_representation(struct RepresentationNode *cur_rep, long long period_duration)
{

    if (cur_rep->SegmentList == 0 && cur_rep->SegmentTemplate == 0) // single segment 
    {
        dashsrc_get_file_with_retry(cur_rep->BaseURL, cur_rep->PathURI);
    }

}
