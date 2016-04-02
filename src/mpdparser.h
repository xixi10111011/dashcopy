#ifndef _MPDPARSER_H_
#define _MPDPARSER_H_

#define MPD_TYPE_STATIC 0
#define MPD_TYPE_LIVE   1


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

struct PeriodNode
{
    long long start;
    long long duration;
    
    struct SegmentBaseType *SegmentBase;
    struct SegmentListNode *SegmentList;
    struct SegmentTemplateNode *SegmentTemplate;

    struct AdaptationSetNode *AdaptationSets;

    char *BaseURL;

    struct PeriodNode *next;
};

struct MPDNode
{
    int type;
    long long mediaPresentationDuration;
    struct PeriodNode Periods;
};






#endif /* _MPDPARSER_H_ */
