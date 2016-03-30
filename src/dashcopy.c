#include "log.h"
#include "http.h"

FILE *dbgstream;
int debug_level;

int main()
{
    dbgstream = fopen("test.log","w+");
    debug_level = DBG;
    http_init();
    LOG(FATAL, "hello, dashcopy!\n");
    fclose(dbgstream);
}


