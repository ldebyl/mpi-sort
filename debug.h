/* Debug output functions - don't leave your home process without them */
#define DEBUG 0
#if DEBUG
    #define PRINT_DEBUG(fmt, args...) fprintf(stderr, "%s:  %s:%d\n"fmt, __FILE__, __FUNCTION__, __LINE__, args)
#else
    #define PRINT_DEBUG(fmt, args...) {}
#endif
