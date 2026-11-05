/* Host object layout cross-check only; not a native PI32 linker map. */
#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
int main(void){return WM_GRAIN_COUNT != 10;}
