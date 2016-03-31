#include <stdio.h>

#define FATAL (1)
#define ERR   (2)
#define WARN  (3)
#define INFO  (4)
#define DBG   (5)

#define LOG(level, ...) do { \
    fprintf(dbgstream,"%s(%d):", __FILE__, __LINE__); \
    fprintf(dbgstream, __VA_ARGS__); \
    fprintf(dbgstream, "\n"); \
    fflush(dbgstream); \
    if (level <= debug_level) { \
        printf("%s(%d):", __FILE__, __LINE__); \
        printf( __VA_ARGS__); \
        printf("\n"); \
    } \
} while (0)
extern FILE *dbgstream;
extern int debug_level;

extern int log_init();
extern void log_cleanup();
