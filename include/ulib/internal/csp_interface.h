/* csp_interface.h */

#include <stdbool.h>

#include <ulib/base/base.h>
#include <ulib/internal/chttp.h>

#ifdef USE_LIBV8
char* runv8(const char* jssrc); /* compiles and executes javascript and returns the script return value as string */
#endif

char*        get_reply(void);
unsigned int get_reply_capacity(void);
void         set_reply_capacity(unsigned int n);
