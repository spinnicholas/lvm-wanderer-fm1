/* SPDX-License-Identifier: GPL-3.0-only */
#define DINGHY_HOST 1
#define main wanderer_host_main
#include "../firmware/src/dinghy.c"
#undef main
#include <assert.h>
int main(void)
{
    const splash_mask_t *masks[]={&SPLASH_orbit,&SPLASH_signal,&SPLASH_star,&SPLASH_title,&SPLASH_brand};
    for(unsigned i=0;i<5;i++){
        const splash_mask_t *m=masks[i];
        assert(m->x+m->w<=240&&m->y+m->h<=240);
        for(unsigned y=0;y<m->h;y++){
            unsigned width=0;
            for(unsigned k=m->row[y];k<m->row[y+1];k++)width+=(m->data[k]&15u)+1u;
            assert(width==m->w);
        }
    }
    wm_state_t before=wm;
    uint64_t leds=host_leds,glow=host_glow;
    for(int theme=0;theme<2;theme++){
        wm.light=(uint8_t)theme;splash_active=1;
        screen();
        write_ppm_as(theme?"build/screens/45-splash-light.ppm":"build/screens/44-splash-dark.ppm");
        assert(cv_oy==0&&cv_h==60&&cv_w==240);
        assert(host_leds==leds&&host_glow==glow);
    }
    wm.light=before.light;assert(!memcmp(&before,&wm,sizeof wm));
    splash_started=100;splash_active=1;page_dirty=0;
    fm1_ms=1199;splash_tick();assert(splash_active&&!page_dirty);
    fm1_ms=1200;splash_tick();assert(!splash_active&&page_dirty);
    splash_started=0xffffff00u;splash_active=1;
    fm1_ms=splash_started+1100u;splash_tick();assert(!splash_active);
    splash_active=1;page_dirty=0;splash_dismiss();assert(!splash_active&&page_dirty);
    page_dirty=0;splash_dismiss();assert(!page_dirty);
    splash_active=1;midi_message(1,0x90,60,100);
    assert(!splash_active);assert(voice[0].gate);midi_message(1,0x80,60,0);
    puts("PASS: both themes, bounds, 60-row bands, state/LED preservation, timeout, wrap, dismiss, MIDI note-through");
}
