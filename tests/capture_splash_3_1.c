/* 3.1 animated splash: frames to build/splash/NNNN.ppm (host only). */
#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
int main(void)
{
 fm1_ms=1000;splash_active=1;splash_drawn=splash_skip=0;splash_started=fm1_ms;fresh_left=FRESH_SECTORS;
 for(uint32_t t=0;t<=2800;t+=40){
  fm1_ms=1000+t;splash_tick();if(!splash_active)break;
  screen();char p[64];snprintf(p,sizeof p,"build/splash/%04u.ppm",t);write_ppm_as(p);
 }
 printf("fresh_left %u active %u\n",fresh_left,splash_active);
 return 0;
}
