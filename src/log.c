#include "log.h"

FILE *dbgstream;
int debug_level;

int log_init()
{
    dbgstream = fopen("test.log","w+");

    if (!dbgstream)
        return -1;

    debug_level = DBG;
    return 0;
}


void log_cleanup()
{
    fclose(dbgstream);
}


