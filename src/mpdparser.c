#include "mpdparser.h"
#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

void mpdparser_get_xml_prop_type(xmlNode *a_node, const char *property_name, int *property_value)
{
    xmlChar *prop_str;

    *property_value = MPD_TYPE_LIVE;   
    prop_str = xmlGetProp(a_node, (const xmlChar*)property_name);
    if (prop_str)
    {
        if (xmlStrcmp(prop_str, (xmlChar *) "OnDemand") == 0
            || xmlStrcmp(prop_str, (xmlChar *) "static"))
        {
            LOG(INFO, "MPD static"); 
        }
        else if (xmlStrcmp(prop_str, (xmlChar *) "Live") == 0
            || xmlStrcmp(prop_str, (xmlChar *) "dynamic"))
        {
            LOG(FATAL, "MPD dynamic, not support yet"); 
        } 
        else
        {
            LOG(FATAL, "failed to parser MPD type");
        }

       xmlFree(prop_str);
    }
    else
    {
        LOG(FATAL, "MPD file without type");
    }
    return;
}

int convert_to_ms(int decimals, int pos)
{
    int num = 1, den = 1;
    int i = 3 - pos;

    while (i < 0)
    {
        den *= 10;
        i++;
    }

    while (i > 0)
    {
        num *= 10;
        i--; 
    }

    return decimals * num / den;

}

/* to simplify, assume duration as just like "PT0H1M59.89S" */
int mpdparser_parse_duration(const char *str, long long *value)
{
    int len, pos, ret, read;
    int hours = -1, minutes = -1, seconds = -1, ms = -1;
    int have_ms = 0;
    long long tmp_value;

    LOG(INFO, "begind parse duration %s", str);
    len = strlen(str);
    if (strspn(str, "PT0123456789.HMS") < len)
    {
        LOG(FATAL, "invalid char"); 
        goto error;
    }

    if (str[0] != 'P')
    {
        LOG(FATAL, "P not found"); 
        goto error;
    }
    str++;
    len--;

    if (str[0] != 'T')
    {
        LOG(FATAL, "T not found"); 
        goto error;
    }
    str++;
    len--;
   
    pos = 0;
    if (pos < len)
    {
        do 
        {
            pos = strcspn(str, "HMS.");
            ret = sscanf(str, "%u", &read);
            if (ret != 1)
            {
                LOG(FATAL, "sscanf failed"); 
                goto error;
            }

            switch (str[pos])
            {
                case 'H':
                   if (hours != -1 || minutes != -1 || seconds != -1) 
                   {
                       LOG(FATAL, "hour/minute/second already set"); 
                       goto error;
                   }
                   hours = read;
                   if (hours >= 24)
                   {
                       LOG(FATAL, "hour out of range");
                       goto error;
                   }
                   break;
                case 'M':
                   if (minutes != -1 || seconds != -1) 
                   {
                       LOG(FATAL, "minute/second already set"); 
                       goto error;
                   }
                   minutes= read;
                   if (minutes >= 60)
                   {
                       LOG(FATAL, "minute out of range");
                       goto error;
                   }
                   break;
                case 'S':
                   if (have_ms)
                   {
                       ms = convert_to_ms(read, pos);
                   }
                   else
                   {
                       if (seconds != -1)
                       {
                           LOG(FATAL, "second already set");
                           goto erro;
                       }

                       seconds = read;
                   
                   }
                   break;
                case '.':
                   if (seconds != -1)
                   {
                       LOG(FATAL, "seconds already set"); 
                       goto error;
                   }
                   seconds = read;
                   have_ms = 1;
                   break;
                default:
                    LOG(FATAL, "unexpected char"); 
                    goto error;
                    break;
            }
        
        }while(len > 0);
    
    }

    if (hours == -1)
        hours = 0;
    if (minutes == -1)
        minutes = 0;
    if (seconds == -1)
        seconds = 0;
    if (ms == -1)
        ms = 0;

    tmp_value = ((((hours * 60) + minutes) * 60) + seconds) * 1000 + ms;
    return MPD_PARSE_OK;

error:
    return MPD_PARSE_ERROR;
}

void mpdparser_get_xml_prop_duration(xmlNode *a_node, const char *property_name, long long default_value, long long *property_value)
{
    xmlchar *prop_str;
    char *str;

    *property_value = default_value;
    prop_str = xmlGetProp(a_node, (const xmlChar *)property_name);
    if (prop_str)
    {
        str = (char *)prop_str;
        if (mpdparser_parse_duration(str, property_value))
        {
            xmlFree(prop_str);
            return;
        }
        LOG(INFO, "%s:%lld", property_name, *property_value);
        xmlFree(prop_str);
        return;
    }
}

int url_contains_scheme(const char *url)
{
    if (url[0] == 'h'
        || url[1] == 't'
        || url[2] == 't'
        || url[3] == 'p')
        return 1;

    return 0;
}

/* assume everything goes well */
int mpdparser_parse_baseURL_node(char **BaseURL， char **PathURI, char *ParentBaseURL, char *ParentPathURI, xmlNode *a_node)
{
    xmlChar *node_content = 0;
    char *curBaseURL = 0;
    char *tmpBaseURL = 0;
    char *tmpPathURI = 0;

    node_content = xmlNodeGetContent(a_node);
    curBaseURL = (char *)node_content;

    if (node_content)
    {
        if (url_contains_scheme(curBaseURL))
        {
            tmpBaseURL = (char *)malloc(strlen(curBaseURL) + 1);
            if (!tmpBaseURL)
            {
                LOG(FATAL, "malloc failed"); 
                goto error;
            }
            strncpy(tmpBaseURL, curBaseURL, strlen(curBaseURL) + 1);
            
        }    
        else
        {
            tmpBaseURL = (char *)malloc(strlen(curBaseURL) + strlen(ParentBaseURL) + 1);
            if (!tmpBaseURL)
            {
                LOG(FATAL, "malloc failed"); 
                goto error;
            }
            memset(tmpBaseURL, 0, strlen(curBaseURL) + strlen(ParentBaseURL) + 1);
            memcpy(tmpBaseURL, ParentBaseURL, strlen(ParentBaseURL));
            memcpy(tmpBaseURL + strlen(ParentBaseURL), curBaseURL, strlen(curBaseURL));

            tmpPathURI = (char *)malloc(strlen(curBaseURL) + strlen(ParentPathURI) + 1) 
            if (!tmpPathURI)
            {
                LOG(FATAL, "malloc failed"); 
                goto error;
            }
            memset(tmpPathURI, 0, strlen(curBaseURL) + strlen(ParentPathURI) + 1);
            memcpy(tmpPathURI, ParentPathURI, strlen(ParentPathURI));
            memcpy(tmpPathURI + strlen(ParentPathURI), curBaseURL, strlen(curBaseURL));
        }
    }
    else
    {
        LOG(FATAL, "%s have no content", a_node->name);
        goto error;
    }
    
    *BaseURL = tmpBaseURL;
    *PathURI = tmpPathURI;
    xmlFree(node_content);
    return MPD_PARSE_OK;

error:
    xmlFree(node_content);
    if (tmpBaseURL)
        free(tmpBaseURL);
    if (tmpPathURI)
        free(tmpPathURI);
    return MPD_PARSE_ERROR;
}

int construct_mpd_baseurl(char **mpdBaseURL, const char *mpdURL)
{
    char *tmpurl = 0;
    int last_slash_pos = 0;
    int i = 0;

    for (i = 0; mpdURL[i] != '\0'; i++)
    {
        if (mpdURL[i] == '/')
            last_slash_pos = i;
    }

    tmpurl = (char *)malloc(last_slash_pos + 2);
    if (!tmpurl)
    {
        LOG(FATAL, "malloc failed"); 
        return MPD_PARSE_ERROR;
    }
    memset(tmpurl, 0, last_slash_pos + 2);
    memcpy(tmpurl, mpdURL, last_slash_pos + 1);
    *mpdBaseURL = tmpurl;
    return MPD_PARSE_OK;
}

void mpdparser_free_seg_base_type(struct SegmentBaseType *segbase)
{
    if (!segbase) 
        return;

    if (segbase->indexRange)
        free(segbase->indexRange);
    if (segbase->initialization)
        free(segbase->initialization);

    free(segbase);
}

int mpdparser_parse_seg_base_type(struct SegmentBaseType **ptr, xmlNode *a_node, struct SegmentBaseType *parent)
{
    xmlNode *cur_node = 0;
    struct SegmentBaseType *new_seg_base = 0;
    char * tmp_str = 0;

    new_seg_base = (struct SegmentBaseType *)malloc(sizeof(struct SegmentBaseType));
    if (!new_seg_base)
    {
        LOG(FATAL, "malloc failed");
        goto error;
    }

    new_seg_base->timeScale = 1;
    new_seg_base->indexRange = 0;
    new_seg_base->initialization = 0;

    if (parent)
    {
        new_seg_base->timeScale = parent->timeScale; 
        if (parent->indexRange) 
        {
            new_seg_base->indexRange = (struct indexRange *)malloc(sizeof(struct indexRange));
            if (!new_seg_base->indexRange)
            {
                LOG(FATAL, "malloc failed"); 
                goto error;
            }
            memcpy(new_seg_base->indexRange, parent->indexRange, sizeof(struct indexRange));
        }

        if (parent->initialization)
        {
            new_seg_base->initialization = (char *)malloc(strlen(parent->initialization) + 1);
            if (!new_seg_base->initialization)
            {
                LOG(FATAL, "malloc failed");
                goto error;
            }
            memset(new_seg_base->initialization, 0, sstrlen(parent->initialization) + 1);
            memcpy(new_seg_base->initialization, parent->initialization, strlen(parent->initialization));
        }

    }

error:

    if (new_seg_base)
        mpdparser_free_seg_base_type(new_seg_base);
    return MPD_PARSE_ERROR;
}

int mpdparser_parse_period_node(struct PeriodNode **PeriodsHead, struct MPDNode *Parent, xmlNode *a_node)
{
    xmlNode *cur_node;
    struct PeriodNode *new_period = 0;

    new_period = (struct PeriodNode *)malloc(sizeof(struct PeriodNode));
    if (!new_period)
    {
        LOG(FATAL, "malloc failed"); 
        return MPD_PARSE_ERROR;
    }

    mpdparser_get_xml_prop_duration(a_node, "start", -1, &new_period->start);
    mpdparser_get_xml_prop_duration(a_node, "duration", -1, &new_period->start);

    for (cur_node = a_node->children; cur_node; cur_node = cur_node->next)
    {
        if (cur_node->type == XML_ELEMENT_NODE) 
        {
            if (xmlStrcmp(cur_node->name, (xmlChar *)"BaseURL") == 0 && !(new_mpd->BaseURL))       
            {
                if(mpdparser_parse_baseURL_node(&new_period->BaseURL, &new_period->PathURI, Parent->BaseURL, Parent->PathURI, cur_node))
                {
                    goto error; 
                }
            }
            else if (xmlStrcmp(cur_node->name, (xmlChar *)"SegmentBase") == 0)
            {
                if (mpdparser_parse_seg_base_type(&new_period->SegmentBase, cur_node, 0))
                {
                    goto error;
                }
            }
            else if (xmlStrcmp(cur_node->name, (xmlChar *)"SegmentList") == 0)
            {
                //if (mpdparser_parse_seg_base_type(&new_period->SegmentBase, cur_node, 0))
                {
                    goto error;
                }
            }
            else if (xmlStrcmp(cur_node->name, (xmlChar *)"SegmentTemplate") == 0)
            {
                //if (mpdparser_parse_seg_base_type(&new_period->SegmentBase, cur_node, 0))
                {
                    goto error;
                }
            }
            else if (xmlStrcmp(cur_node->name, (xmlChar *)"AdaptationSet") == 0)
            {
                //if (mpdparser_parse_seg_base_type(&new_period->SegmentBase, cur_node, 0))
                {
                    goto error;
                }
            } 
        }
    

}

int mpdparser_parse_root_node(struct MPDNode **ptr, xmlNode *a_node, char *mpdURL)
{
    xmlNode *cur_node;
    struct MPDNode *new_mpd;
    char *mpdBaseURL;

    new_mpd =(struct MPDNode *) malloc(sizeof(struct MPDNode));
    if (!new_mpd)
    {
        LOG(FATAL, "malloc failed"); 
        return MPD_PARSE_ERROR; 
    }

    memset(new_mpd, 0, sizeof(struct MPDNode));

    mpdparser_get_xml_prop_type(a_node, "type", &new_mpd->type);
    if (new_mpd->type == MPD_TYPE_LIVE)
    {
        LOG(FATAL, "live not supported yet");
        goto error;
    }

    mpdparser_get_xml_prop_duration(a_node, "mediaPresentationDuration", 0, &new_mpd->mediaPresentationDuration);


    if (construct_mpd_baseurl(&mpdBaseURL, mpdURL))
    {
        goto error;
    }

    for (cur_node = a_node->children; cur_node; cur_node = cur_node->next)
    {
        if (cur_node->type == XML_ELEMENT_NODE) 
        {
            if (xmlStrcmp(cur_node->name, (xmlChar *)"BaseURL") == 0 && !(new_mpd->BaseURL))       
            {
                if(mpdparser_parse_baseURL_node(&new_mpd->BaseURL, &new_mpd->PathURI, mpdURL, NULL, cur_node))
                {
                    goto error; 
                }
            }
            else if (xmlStrcmp(cur_node->name, (xmlChar *)"Period") == 0)
            {
                if (mpdparser_parse_period_node(&new_mpd->Periods, new_mpd,  cur_node))
                {
                    goto error;
                }
            }
        
        }
    
    }

    *ptr = new_mpd;
    return MPD_PARSE_OK;

error:
    mpdparser_free_mpd_node(new_mpd);
    return MPD_PARSE_ERROR;
}



