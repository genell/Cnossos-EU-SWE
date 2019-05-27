#if defined(MSVC) || defined(__MINGW32__) || defined(WIN32)
// Windows MSVC or MingW
//#include "win.h" 
#elif __GNUC__
// Unix/GCC environment - TODO: complement with more thorough macro?
#include "unixgcc.h"
#endif
