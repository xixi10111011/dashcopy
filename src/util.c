#include <stdlib.h>
#include "log.h"
#include "util.h"




inline void *xmalloc(int size)
{
   void *ptr = malloc(size);

   if(!ptr)
   {
       LOG(FATAL, "malloc [%d] failed", size);
       exit(0);
   }

   memset(ptr, 0, size);
   return ptr;
}
