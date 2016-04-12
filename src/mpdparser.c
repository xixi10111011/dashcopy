#include "mpdparser.h"
#include "log.h"
#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

void mpdparser_free_segmentlist(struct SegmentListNode *ptr);
void mpdparser_free_segmenttemplate(struct SegmentTemplateNode *ptr);
void mpdparser_free_representation(struct RepresentationNode *ptr);

void mpdparser_get_xml_prop_type(xmlNode *a_node, const char *property_name, int *property_value)
{
    xmlChar *prop_str;

    *property_value = MPD_TYPE_LIVE;   
    prop_str = xmlGetProp(a_node, (const xmlChar*)property_name);
    if (prop_str)
    {
        if (xmlStrcmp(prop_str, (xmlChar *) "OnDemand") == 0
            || xmlStrcmp(prop_str, (xmlChar *) "static") == 0)
        {
            LOG(INFO, "MPD static"); 
            *property_value = MPD_TYPE_STATIC;   

        }
        else if (xmlStrcmp(prop_str, (xmlChar *) "Live") == 0
            || xmlStrcmp(prop_str, (xmlChar *) "dynamic") == 0)
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

    LOG(INFO, "begin to parse duration %s", str);
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
                           goto error;
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
            len -= pos + 1;
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
    *value = tmp_value;
    return MPD_PARSE_OK;

error:
    return MPD_PARSE_ERROR;
}

void mpdparser_get_xml_prop_duration(xmlNode *a_node, const char *property_name, long long default_value, long long *property_value)
{
    xmlChar *prop_str;
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
int mpdparser_parse_baseURL_node(char **BaseURL, char **PathURI, char *ParentBaseURL, char *ParentPathURI, xmlNode *a_node)
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

            tmpPathURI = (char *)malloc(strlen(curBaseURL) + strlen(ParentPathURI) + 1); 
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
#if 0
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
#endif


void mpdparser_free_segmenttemplate(struct SegmentTemplateNode *ptr)
{
}

void mpdparser_free_segmentlist(struct SegmentListNode *ptr)
{

}


int mpdparser_get_xml_prop_string(xmlNode *a_node, const char *property_name, char **property_value)
{
    xmlChar *prop_str;
    char *tmp_str;


    prop_str = xmlGetProp(a_node, (const xmlChar *) property_name);
    if (prop_str)
    {
        tmp_str = (char *)malloc(strlen((char *)prop_str)+1);
        if (!tmp_str)
        {
            LOG(FATAL, "malloc failed");
            goto error;
        }

        memcpy(tmp_str, prop_str, strlen((char *)prop_str) + 1);
        *property_value = tmp_str;
        xmlFree(prop_str);
    }
    return MPD_PARSE_OK;

error:
    if (prop_str)
        xmlFree(prop_str);
    return MPD_PARSE_ERROR;
}

int mpdparser_get_xml_prop_unsigned_integer(xmlNode *a_node, const char *property_name, unsigned int default_value, unsigned int *property_value)
{
    xmlChar *prop_str;

    *property_value = default_value;

    prop_str = xmlGetProp(a_node, (const xmlChar *)property_name);
    if (prop_str)
    {
        sscanf((char *)prop_str, "%u", property_value);
    }

    return MPD_PARSE_OK;
}

void mpdparser_free_representation(struct RepresentationNode *ptr)
{
    if (ptr->id)
        free(ptr->id);

    if (ptr->BaseURL)
        free(ptr->BaseURL);

    if (ptr->PathURI)
        free(ptr->PathURI);

    if (ptr->SegmentBase)
        mpdparser_free_seg_base_type(ptr->SegmentBase);

    if (ptr->SegmentList)
        mpdparser_free_segmentlist(ptr->SegmentList);

    if (ptr->SegmentTemplate)
        mpdparser_free_segmenttemplate(ptr->SegmentTemplate);

    free(ptr);

    return;
}

int mpdparser_parse_representation_node(struct RepresentationNode **ListHead, xmlNode *a_node,  struct AdaptationSetNode *parent)
{

    xmlNode *cur_node = 0;
    struct RepresentationNode *new_representation = 0, *node = 0;

    new_representation = (struct RepresentationNode *)malloc(sizeof(struct RepresentationNode));
    if (!new_representation)
    {
        LOG(FATAL, "malloc failed");
        return MPD_PARSE_ERROR;
    }
    memset(new_representation, 0, sizeof(struct RepresentationNode));

    if (mpdparser_get_xml_prop_string(a_node, "id", &new_representation->id))
    {
        LOG(FATAL, "parse id failed");
        goto error;
    }
    if (mpdparser_get_xml_prop_unsigned_integer(a_node, "bandwidth", 0, &new_representation->bandwidth))
    {
        goto error; 
    }

    for (cur_node = a_node->children; cur_node; cur_node = cur_node->next)
    {
        if (cur_node->type == XML_ELEMENT_NODE) 
        {
            if (xmlStrcmp(cur_node->name, (xmlChar *)"BaseURL") == 0 && !(new_representation->BaseURL))       
            {
                if(mpdparser_parse_baseURL_node(&new_representation->BaseURL, &new_representation->PathURI, parent->BaseURL, parent->PathURI, cur_node))
                {
                    goto error; 
                }
            }
#if 0
            else if (xmlStrcmp(cur_node->name, (xmlChar *)"SegmentBase") == 0)
            {
                if (mpdparser_parse_seg_base_type(&new_period->SegmentBase, cur_node, 0))
                {
                    goto error;
                }
            }
#endif
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
     
        }
    }

    if (!(*ListHead))
    {
        *ListHead = new_representation;
    }
    else
    {
        node = *ListHead;
        while (node->next)
        {
            node = node->next;
        }

        node->next = new_representation;
    }
    
    return MPD_PARSE_OK;


error:
    mpdparser_free_representation(new_representation);
    return MPD_PARSE_ERROR;
}

void mpdparser_free_adaptationset(struct AdaptationSetNode *ptr)
{
    struct RepresentationNode *node = 0, *next = 0;

    if (ptr->BaseURL)
        free(ptr->BaseURL);

    if (ptr->PathURI)
        free(ptr->PathURI);

    if (ptr->SegmentBase)
        mpdparser_free_seg_base_type(ptr->SegmentBase);

    if (ptr->SegmentList)
        mpdparser_free_segmentlist(ptr->SegmentList);

    if (ptr->SegmentTemplate)
        mpdparser_free_segmenttemplate(ptr->SegmentTemplate);

    node = ptr->Representations;

    while (node)
    {
        next = node->next;
        mpdparser_free_representation(node);
        node = next; 
    }

    free(ptr);

    return;
}

int mpdparser_parse_adaptationset_node(struct AdaptationSetNode **ListHead, xmlNode *a_node, struct PeriodNode *parent)
{
    xmlNode *cur_node = 0;
    struct AdaptationSetNode *new_adpset = 0, *node = 0;

    new_adpset = (struct AdaptationSetNode *)malloc(sizeof(struct AdaptationSetNode)); 
    if (!new_adpset)
    {
        LOG(FATAL, "malloc failed");
        return MPD_PARSE_ERROR;
    }
    memset(new_adpset, 0, sizeof(struct AdaptationSetNode));

    for (cur_node = a_node->children; cur_node; cur_node = cur_node->next)
    {
        if (cur_node->type == XML_ELEMENT_NODE) 
        {
            if (xmlStrcmp(cur_node->name, (xmlChar *)"BaseURL") == 0 && !(new_adpset->BaseURL))       
            {
                if(mpdparser_parse_baseURL_node(&new_adpset->BaseURL, &new_adpset->PathURI, parent->BaseURL, parent->PathURI, cur_node))
                {
                    goto error; 
                }
            }
#if 0
            else if (xmlStrcmp(cur_node->name, (xmlChar *)"SegmentBase") == 0)
            {
                if (mpdparser_parse_seg_base_type(&new_period->SegmentBase, cur_node, 0))
                {
                    goto error;
                }
            }
#endif
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
            else if (xmlStrcmp(cur_node->name, (xmlChar *)"Representation") == 0)
            {
                if (mpdparser_parse_representation_node(&new_adpset->Representations, cur_node, new_adpset))
                {
                    goto error;
                }
            } 
        }
 
    }

    if (!(*ListHead))
    {
        *ListHead = new_adpset;
    }
    else
    {
        node = *ListHead;
        while (node->next)
        {
            node = node->next;
        }

        node->next = new_adpset;
    }
    
    return MPD_PARSE_OK;


error:
    mpdparser_free_adaptationset(new_adpset);
    return MPD_PARSE_ERROR;
}

void mpdparser_free_period_node(struct PeriodNode *ptr)
{
    struct AdaptationSetNode *node = 0, *next = 0;

    if (ptr->BaseURL)
        free(ptr->BaseURL);

    if (ptr->PathURI)
        free(ptr->PathURI);

    if (ptr->SegmentBase)
        mpdparser_free_seg_base_type(ptr->SegmentBase);

    if (ptr->SegmentList)
        mpdparser_free_segmentlist(ptr->SegmentList);

    if (ptr->SegmentTemplate)
        mpdparser_free_segmenttemplate(ptr->SegmentTemplate);

    node = ptr->AdaptationSets;

    while (node)
    {
        next = node->next;
        mpdparser_free_adaptationset(node);
        node = next; 
    }

    free(ptr);

    return;
}

int mpdparser_parse_period_node(struct PeriodNode **PeriodsHead, struct MPDNode *parent, xmlNode *a_node)
{
    xmlNode *cur_node;
    struct PeriodNode *new_period = 0;
    struct PeriodNode *node = 0;

    new_period = (struct PeriodNode *)malloc(sizeof(struct PeriodNode));
    if (!new_period)
    {
        LOG(FATAL, "malloc failed"); 
        return MPD_PARSE_ERROR;
    }

    memset(new_period, 0, sizeof(struct PeriodNode));

    mpdparser_get_xml_prop_duration(a_node, "start", -1, &new_period->start);
    mpdparser_get_xml_prop_duration(a_node, "duration", -1, &new_period->start);

    for (cur_node = a_node->children; cur_node; cur_node = cur_node->next)
    {
        if (cur_node->type == XML_ELEMENT_NODE) 
        {
            if (xmlStrcmp(cur_node->name, (xmlChar *)"BaseURL") == 0 && !(new_period->BaseURL))       
            {
                if(mpdparser_parse_baseURL_node(&new_period->BaseURL, &new_period->PathURI, parent->BaseURL, parent->PathURI, cur_node))
                {
                    goto error; 
                }
            }
#if 0
            else if (xmlStrcmp(cur_node->name, (xmlChar *)"SegmentBase") == 0)
            {
                if (mpdparser_parse_seg_base_type(&new_period->SegmentBase, cur_node, 0))
                {
                    goto error;
                }
            }
#endif
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
                if (mpdparser_parse_adaptationset_node(&new_period->AdaptationSets, cur_node, new_period))
                {
                    goto error;
                }
            } 
        }
    }

    if (!(*PeriodsHead))
    {
        *PeriodsHead = new_period;
    }
    else
    {
        node = *PeriodsHead;
        while (node->next)
        {
            node = node->next;
        }

        node->next = new_period;
    }
    
    return MPD_PARSE_OK;

error:
    mpdparser_free_period_node(new_period);
    return MPD_PARSE_ERROR;
}

void mpdparser_free_mpd_node(struct MPDNode *ptr)
{
    struct PeriodNode *node = ptr->Periods;
    struct PeriodNode *next = 0;

    while (node)
    {
        next = node->next;
        mpdparser_free_period_node(node);
        node = next;
    }

    if (ptr->BaseURL)
        free(ptr->BaseURL);

    if (ptr->PathURI)
        free(ptr->PathURI);

    free(ptr);

    return; 
}

int mpdparser_parse_root_node(struct MPDNode **ptr, xmlNode *a_node, const char *mpdURL)
{
    xmlNode *cur_node;
    struct MPDNode *new_mpd;
    char *mpd_baseurl = 0;

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


    if (construct_mpd_baseurl(&mpd_baseurl, mpdURL))
    {
        goto error;
    }

    for (cur_node = a_node->children; cur_node; cur_node = cur_node->next)
    {
        if (cur_node->type == XML_ELEMENT_NODE) 
        {
            if (xmlStrcmp(cur_node->name, (xmlChar *)"BaseURL") == 0 && !(new_mpd->BaseURL))       
            {
                if(mpdparser_parse_baseURL_node(&new_mpd->BaseURL, &new_mpd->PathURI, mpd_baseurl, NULL, cur_node))
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

int mpdparser_parse_mpd_buffer(struct MPDNode **ptr, const char *buffer, int len, const char *mpdurl)
{
    xmlDocPtr doc;
    xmlNode *root_node = 0;
    int ret = MPD_PARSE_OK;

    LIBXML_TEST_VERSION;

    doc = xmlReadMemory(buffer, len, "noname.xml", NULL, XML_PARSE_NONET);
    if (!doc)
    {
        LOG(FATAL, "xml read failed");
        ret = MPD_PARSE_ERROR;
    }
    else
    {
        root_node = xmlDocGetRootElement(doc);

        if (root_node->type != XML_ELEMENT_NODE
            || xmlStrcmp(root_node->name, (xmlChar *)"MPD") != 0)
        {
            LOG(FATAL, "can not find MPD node");
            ret = MPD_PARSE_ERROR;
        }
        else
        {
            ret = mpdparser_parse_root_node(ptr, root_node, mpdurl);
        }
        xmlFreeDoc(doc); 
    }

    return ret;
}

