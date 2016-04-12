#ifndef _MPDPARSER_H_
#define _MPDPARSER_H_

#define MPD_TYPE_STATIC 0
#define MPD_TYPE_LIVE   1

#define MPD_PARSE_OK        0
#define MPD_PARSE_ERROR     1

struct Range
{
    long long firstBytePos;
    long long lastBytePos;
};

struct SegmentURLNode
{
    char *media;
    
    struct SegmentURLNode *next;
};

struct SegmentBaseType
{
    unsigned int timeScale;
    struct Range *indexRange;
    char *initialization;
};

struct SNode
{
    long long t;
    long long d;
    int r;

    struct SNode *next;
};


struct SegmentTimelineNode 
{
    struct SNode *SNodes;
};

struct MultSegmentBaseType
{
    unsigned int duration;
    unsigned int startNumber;

    struct SegmentBaseType SegBaseType;
    struct SegmentTimelineNode *SegmentTimeline;
};


struct SegmentListNode
{
    struct MultSegmentBaseType *MultSegBaseType;
    struct SegmentURLNode  *SegURLs;
};

struct SegmentTemplateNode
{
    struct MultSegmentBaseType *MultSegBaseType;
    char *media;
    char *initialization;
};

struct RepresentationBaseType
{

};

struct RepresentationNode
{
    char *id;
    unsigned int bandwidth;
    struct RepresentationBaseType *RepresentationBase;
    struct SegmentBaseType        *SegmentBase;
    struct SegmentListNode        *SegmentList;
    struct SegmentTemplateNode    *SegmentTemplate;
    char *BaseURL;
    char *PathURI;
    struct RepresentationNode *next;
};

struct AdaptationSetNode 
{
    struct RepresentationBaseType *RepresentationBase;
    struct SegmentBaseType        *SegmentBase;
    struct SegmentListNode        *SegmentList;
    struct SegmentTemplateNode    *SegmentTemplate;
    char *BaseURL;
    char *PathURI;
    struct RepresentationNode     *Representations;
    struct AdaptationSetNode *next;
};

struct PeriodNode
{
    long long start;
    long long duration;
    
    struct SegmentBaseType *SegmentBase;
    struct SegmentListNode *SegmentList;
    struct SegmentTemplateNode *SegmentTemplate;

    struct AdaptationSetNode *AdaptationSets;

    char *BaseURL;
    char *PathURI;

    struct PeriodNode *next;
};

struct MPDNode
{
    int type;
    long long mediaPresentationDuration;
    char *BaseURL;
    char *PathURI;
    struct PeriodNode *Periods;
};


int mpdparser_parse_mpd_buffer(struct MPDNode **ptr, const char *buffer, int len, const char *mpdurl);
void mpdparser_free_mpd_node(struct MPDNode *ptr);




#endif /* _MPDPARSER_H_ */
