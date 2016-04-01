#ifndef _MPDPARSER_H_
#define _MPDPARSER_H_



struct Range
{
    long long firstBytePos;
    long long lastBytePos;
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

    SegmentBaseType SegBaseType;
    SegmentTimelineNode *SegmentTimeline;
};

struct PeriodNode
{
    SegmentBaseType *SegmentBase;
    SegmentListNode *SegmentList;
    SegmentTemplateNode *SegmentTemplate;

    struct AdaptationSetNode *AdaptationSets;

    char *BaseURL;

    struct PeriodNode *next;
};

struct MPDNode
{
    int type;

    struct PeriodNode Periods;
};






#endif /* _MPDPARSER_H_ */
