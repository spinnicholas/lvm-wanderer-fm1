/* SPDX-License-Identifier: GPL-3.0-only
 * Wanderer 1.1 RC2: licensed source icons, calm spacing and explicit state.
 * Only Home uses a word. All editing views devote the header to context/status.
 */
static char *number(uint32_t n,char *s)
{
    char t[12];uint32_t i=0,j=0;
    if(!n){s[0]='0';s[1]=0;return s;}
    while(n&&i<11u){t[i++]=(char)('0'+n%10u);n/=10u;}
    while(i){s[j++]=t[--i];}
    s[j]=0;return s;
}
static void symbol_blit(int32_t x,int32_t y,uint32_t id,uint32_t size,const icon_atlas_t *base,uint16_t fg)
{
    for(uint32_t gy=0;gy<size;gy++){
      if(y+(int32_t)gy+cv_oy<0||y+(int32_t)gy+cv_oy>=(int32_t)cv_h)continue;
      const uint8_t *data=base->data+base->row[id*size+gy];
      uint32_t run=0,alpha=0;
      for(uint32_t gx=0;gx<size;gx++){
        if(!run){uint32_t token=*data++;alpha=token>>4;run=(token&15u)+1u;}run--;
        int32_t px=x+(int32_t)gx,py=y+(int32_t)gy+cv_oy;
        if(alpha&&(uint32_t)px<cv_w&&(uint32_t)py<cv_h){
            uint16_t old=swap16(cv_px[(uint32_t)py*cv_w+(uint32_t)px]);
            uint32_t r=(((old>>11)*(15u-alpha)+(fg>>11)*alpha)+7u)/15u;
            uint32_t g=(((((old>>5)&63u)*(15u-alpha)+((fg>>5)&63u)*alpha)+7u)/15u);
            uint32_t b=(((old&31u)*(15u-alpha)+(fg&31u)*alpha)+7u)/15u;
            cv_px[(uint32_t)py*cv_w+(uint32_t)px]=swap16((r<<11)|(g<<5)|b);
        }
    }
    }
}
static void icon_blit(int32_t x,int32_t y,uint32_t id,uint16_t fg){symbol_blit(x,y,id,64,ICON_ALPHA,fg);}
static void zoom_blit(int32_t x,int32_t y,uint32_t id,uint16_t fg){symbol_blit(x,y,id,96,ICON_ZOOM_ALPHA,fg);}
static void tiny(int32_t x,int32_t y,uint32_t id,uint16_t fg){symbol_blit(x,y,id,24,ICON_TINY_ALPHA,fg);}
static void mini(int32_t x,int32_t y,uint32_t id,uint16_t fg){symbol_blit(x,y,id,32,ICON_MINI_ALPHA,fg);}
static void mid(int32_t x,int32_t y,uint32_t id,uint16_t fg){symbol_blit(x,y,id,48,ICON_MID_ALPHA,fg);}
static void disk(int32_t cx,int32_t cy,int32_t r,uint16_t col)
{
    for(int32_t y=-r;y<=r;y++)for(int32_t x=-r;x<=r;x++)if(x*x+y*y<=r*r)cv_pset(cx+x,cy+y,col);
}
static void panel(int32_t x,int32_t y,int32_t w,int32_t h,uint16_t fill)
{
    int32_t r=h/2<7?h/2:7;
    cv_rect(x+r,y,w-2*r,h,fill);cv_rect(x,y+r,w,h-2*r,fill);
    disk(x+r,y+r,r,fill);disk(x+w-r-1,y+r,r,fill);
    disk(x+r,y+h-r-1,r,fill);disk(x+w-r-1,y+h-r-1,r,fill);
}
static void card(int32_t x,int32_t y,int32_t w,int32_t h,uint16_t fill,uint16_t edge,uint8_t selected)
{
    uint32_t inset=selected?3u:1u;panel(x,y,w,h,edge);
    panel(x+(int32_t)inset,y+(int32_t)inset,w-2*(int32_t)inset,h-2*(int32_t)inset,fill);
}
static void slash(int32_t x,int32_t y,int32_t w,uint16_t col)
{for(int32_t i=-1;i<=1;i++)cv_line(x+i,y+w,x+w+i,y,col);}
static void switch_draw(int32_t x,int32_t y,int32_t w,int32_t h,uint8_t on,uint16_t fg,uint16_t guide,uint16_t bg)
{
    int32_t r=h/2;
    cv_rect(x+r,y,w-2*r,h,on?fg:guide);disk(x+r,y+r,r,on?fg:guide);disk(x+w-r-1,y+r,r,on?fg:guide);
    if(!on){cv_rect(x+r,y+2,w-2*r,h-4,bg);disk(x+r,y+r,r-2,bg);disk(x+w-r-1,y+r,r-2,bg);}
    disk(x+(on?w-r-1:r),y+r,r-4,on?bg:fg);
}
static void transport_symbol(int32_t x,int32_t y,uint16_t col)
{
    if(!running){cv_rect(x+3,y+3,15,15,col);return;}
    for(int32_t i=0;i<18;i++)cv_line(x+2,y+2+i,x+2+(i<9?i:17-i)*2,y+2+i,col);
}
static void status_header(uint32_t context,uint16_t fg,uint16_t guide,uint8_t home)
{
    if(home)aa_text(12,4,&FONT_N,"WANDERER",fg);else mini(8,0,context,fg);
    transport_symbol(209,6,fg);
    if(recording)disk(193,16,6,fg);
    cv_rect(12,33,216,2,guide);
}
static void gauge(int32_t x,int32_t y,int32_t w,uint32_t percent,uint16_t fg,uint16_t guide)
{
    if(percent>100u)percent=100u;
    panel(x,y,w,10,guide);
    if(percent)cv_rect(x,y+2,(int32_t)(percent*(uint32_t)w/100u),6,fg);
}
static uint32_t percent(uint32_t v){return (v*100u+63u)/127u;}
static void percent_sign(int32_t x,int32_t y,uint16_t fg)
{
    disk(x+3,y+3,2,fg);disk(x+12,y+15,2,fg);
    for(int32_t i=0;i<2;i++)cv_line(x+2+i,y+17,x+13+i,y+1,fg);
}
static void large_value(uint32_t value,int32_t y,uint8_t percentage,uint16_t fg)
{
    const font_t *f=wm.extra_large?&FONT_X:&FONT_B;char b[12];number(value,b);
    int32_t w=width(f,b)+(percentage?20:0),x=(240-w)/2;
    aa_text(x,y,f,b,fg);if(percentage)percent_sign(x+width(f,b)+3,y+f->h-20,fg);
}
static void steps(const wm_phrase_t *p,int32_t x,int32_t y,int32_t spacing,int32_t group,uint16_t col,uint16_t guide,uint8_t audible)
{
    /* Two bars occupy two rows, keeping each step as readable as the 16-step view. */
    for(uint32_t i=0;i<wm.loop_steps;i++){
        uint32_t pos=i%16u,row=i/16u;
        int32_t px=x+(int32_t)pos*spacing+(int32_t)(pos/4u)*group;
        int32_t py=y+(wm.loop_steps==32u?(int32_t)row*8-2:4);
        uint8_t note=wm_step_notes(p,i)!=0;
        cv_rect(px,py,spacing-2,note?(wm.loop_steps==32u?4:8):1,note?col:guide);
        /* Held gates join cells so duration is visible instead of another onset dot. */
        for(uint32_t v=0;v<WM_POLY;v++){
            wm_event_t event=wm_get(p,i,v);
            if(!event.velocity||event.gate<=256u)continue;
            uint32_t count=(event.gate-1u)/256u;
            if(count>wm.loop_steps-1u)count=wm.loop_steps-1u;
            for(uint32_t k=1;k<=count;k++){
                uint32_t t=(i+k)%wm.loop_steps;
                int32_t tx=x+(int32_t)(t%16u)*spacing+(int32_t)((t%16u)/4u)*group;
                int32_t ty=y+(wm.loop_steps==32u?(int32_t)(t/16u)*8:6);
                cv_rect(tx,ty,spacing-2,2,col);
            }
        }
        if(running&&audible&&i==(step_index+wm.loop_steps-1u)%wm.loop_steps){
            cv_rect(px-1,py-3,spacing,2,col);
        }
    }
}
static uint32_t focus_icon(void)
{
    switch(focus){case 1:return ICO_MELT;case 2:return ICO_DRIFT;case 3:return wm.legacy?ICO_DECAY:ICO_REPEAT;
    case 4:return ICO_TEMPO;case 5:return ICO_LEVEL;case 7:return ICO_SOUND;default:return ICO_SIZE;}
}
static uint32_t notice_icon(void)
{
    switch(notice){
    case N_AUDIO_LOCK:case N_ORIGINAL_SAFE:case N_ANCHOR_SAFE:return ICO_LOCK;
    case N_ANCHORED:case N_CATCH_FIRST:return ICO_ANCHOR;
    case N_BRANCHED:return ICO_BRANCH;
    case N_UNDO:case N_NO_UNDO:return ICO_UNDO;
    case N_NO_PHRASE:case N_NO_VALID_SAVE:return ICO_EMPTY;
    case N_SAVE_FAILED:case N_PREF_FAILED:return ICO_BROKEN;
    case N_STILL:case N_STOP_FIRST:return ICO_STOP;case N_LIVE:return ICO_PLAY;
    case N_LAYER_CLEARED:return ICO_LAYER;case N_NEW_SESSION:return ICO_NEW;
    case N_SAVED:return ICO_SAVE;case N_LOADED:return ICO_LOAD;
    default:return ICO_MEMORY;}
}
static uint8_t notice_error(void)
{
    return notice==N_NO_PHRASE||notice==N_CATCH_FIRST||notice==N_NO_UNDO||notice==N_SAVE_FAILED||
        notice==N_NO_VALID_SAVE||notice==N_STOP_FIRST||notice==N_PREF_FAILED||notice==N_AUDIO_LOCK;
}
static uint8_t notice_success(void)
{return notice==N_SAVED||notice==N_LOADED;}
static uint8_t notice_sticky(void)
{return notice_error()||notice_success();}
static void project_footer(uint16_t fg,uint16_t guide)
{
    uint32_t icon=project_dirty?ICO_EDITED:project_origin_slot<NSLOT?ICO_STORED:ICO_FRESH;
    tiny(8,214,icon,project_dirty||project_origin_slot<NSLOT?fg:guide);
    if(project_origin_slot<NSLOT){char b[12];number(project_origin_slot+1u,b);aa_text(38,216,&FONT_N,b,fg);}
    if(wm.arp_on)aa_text(70,217,&FONT_T,"ARP",fg);
    if(!vol)tiny(wm.arp_on?126:108,214,ICO_MUTE,fg);
    tiny(159,214,ICO_LENGTH,guide);
    char b[12];number(wm.loop_steps,b);aa_text(188,216,&FONT_N,b,fg);
}
static void screen_home(uint16_t bg,uint16_t fg,uint16_t pa,uint16_t pm,uint16_t guide,uint16_t a,uint16_t m,uint16_t l)
{
    status_header(ICO_MEMORY,fg,guide,1);
    card(8,42,108,118,pa,anchor_view?fg:guide,anchor_view);
    card(124,42,108,118,pm,!anchor_view?fg:guide,!anchor_view);
    icon_blit(30,70,ICO_ANCHOR,a);icon_blit(146,70,ICO_MEMORY,m);
    if(wm.has_anchor||wp_has_anchor)mini(12,45,ICO_LOCK,fg);else mini(12,45,ICO_NEW,guide);
    if(anchor_view)mini(80,45,ICO_LISTEN,fg);else mini(196,45,ICO_LISTEN,fg);
    if(recording&&!wm.layer_enabled)disk(211,119,5,fg);
    steps(&wm.anchor,12,140,6,2,a,guide,anchor_view);
    steps(&wm.memory,128,140,6,2,m,guide,!anchor_view);
    if(wp_active||wp_writing||wp_armed){
        card(8,165,224,47,pa,guide,0);mini(16,172,wp_writing?ICO_RECORD:ICO_MEMORY,l);
        aa_text(58,171,&FONT_T,wp_writing?"PRINTING":wp_armed?"PRINT ARMED":"PRINT / RAM",fg);
        if(wp_has_anchor)tiny(198,180,ICO_LOCK,fg);
        project_footer(fg,guide);return;
    }
    card(8,165,224,47,pa,guide,0);mid(14,164,ICO_LAYER,l);
    if(!wm.layer_enabled)slash(20,170,30,fg);
    switch_draw(168,168,54,25,wm.layer_enabled,fg,guide,pa);
    if(recording&&wm.layer_enabled)disk(85,181,5,fg);
    steps(&wm.layer,78,198,8,3,l,guide,!anchor_view&&wm.layer_enabled);
    project_footer(fg,guide);
    (void)bg;
}
static void screen_zoom(uint16_t bg,uint16_t fg,uint16_t pa,uint16_t pm,uint16_t guide,uint16_t a,uint16_t m,uint16_t l)
{
    uint8_t original=anchor_view&&(wm.has_anchor||wp_has_anchor);uint16_t col=original?a:m;
    const wm_phrase_t *p=original?&wm.anchor:&wm.memory;
    status_header(original?ICO_ANCHOR:ICO_MEMORY,fg,guide,1);
    card(8,42,224,136,original?pa:pm,fg,1);
    zoom_blit(72,52,original?ICO_ANCHOR:ICO_MEMORY,col);
    if(original)mini(16,47,wm.has_anchor||wp_has_anchor?ICO_LOCK:ICO_EMPTY,fg);
    if(recording&&!wm.layer_enabled)disk(210,62,7,fg);
    steps(p,16,152,12,4,col,guide,1);
    card(8,185,224,27,pa,guide,0);tiny(18,187,ICO_LAYER,l);
    if(!wm.layer_enabled)slash(20,188,20,fg);
    switch_draw(172,187,48,23,wm.layer_enabled,fg,guide,pa);
    if(recording&&wm.layer_enabled)disk(62,198,5,fg);
    steps(&wm.layer,80,196,5,2,l,guide,!anchor_view&&wm.layer_enabled);
    if(wp_active||wp_writing||wp_armed){panel(8,185,224,27,pa);aa_text(18,188,&FONT_T,wp_writing?"PRINTING / RAM":"PRINT / RAM",fg);}
    project_footer(fg,guide);
    (void)bg;
}
static uint32_t setting_icon(void)
{
    const uint8_t ids[4][3]={{ICO_SUN,ICO_SIZE,ICO_STILL},{ICO_CHORUS,ICO_DELAY,ICO_REVERB},{ICO_LEVEL,ICO_BELL,ICO_SAVE},{ICO_ROOT,ICO_SCALE,ICO_LENGTH}};
    return ids[settings_page][settings_index];
}
static uint8_t setting_on(uint32_t i)
{
    if(!settings_page)return i==0?wm.light:i==1?wm.extra_large:wm.reduced_motion;
    if(settings_page==1)return (wm.fx_on>>i)&1u;
    return i==1?cues_enabled:1u;
}
static void row_locator(uint16_t fg,uint16_t guide)
{
    for(uint32_t i=0;i<3u;i++){
        int32_t x=79+(int32_t)i*30;
        cv_rect(x,231,22,4,i==settings_index?fg:guide);
        if(i==settings_index)cv_rect(x,229,22,2,fg);
    }
}
static void diagram_arrow(int32_t x,int32_t y,int32_t w,uint16_t fg)
{
    for(int32_t t=-1;t<=1;t++){
        cv_line(x,y+t,x+w,y+t,fg);
        cv_line(x+w-7,y-7+t,x+w,y+t,fg);
        cv_line(x+w-7,y+7+t,x+w,y+t,fg);
    }
}
static void settings_header(uint16_t fg,uint16_t guide)
{
    const uint8_t ids[4]={ICO_SUN,ICO_CHORUS,ICO_CONTROLS,ICO_SCALE};
    transport_symbol(209,6,fg);if(recording)disk(193,16,6,fg);cv_rect(12,34,216,1,guide);
    for(uint32_t i=0;i<4;i++){
        if(i==settings_page)panel(6+(int32_t)i*46,0,36,30,wm.light?RGB(211,226,229):RGB(22,43,55));
        mini(8+(int32_t)i*46,0,ids[i],i==settings_page?fg:guide);
        if(i==settings_page)cv_rect(12+(int32_t)i*46,30,24,3,fg);
    }
}
static void screen_setting_detail(uint16_t bg,uint16_t fg,uint16_t guide)
{
    uint32_t icon=setting_icon(),value=0;uint8_t toggle=0,percentage=0,timing=0;
    if(settings_page==3u){
        const char *notes[12]={"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
        settings_header(fg,guide);mid(96,45,icon,fg);
        if(settings_index==0){
            const font_t *f=wm.extra_large?&FONT_X:&FONT_B;
            aa_text((240-width(f,notes[wm.root]))/2,111,f,notes[wm.root],fg);
        }else large_value(settings_index==1?wm.scale:wm.loop_steps,settings_index==1?101:111,0,fg);
        if(settings_index==1){
            for(uint32_t n=0;n<12u;n++){
                uint8_t on=(WM_SCALES[wm.scale]>>n)&1u;
                int32_t x=13+(int32_t)n*18;
                cv_rect(x,201,14,19,on?fg:guide);
                if(!on)cv_rect(x+2,203,10,15,bg);
            }
        }else if(settings_index==2){
            for(uint32_t bar=0;bar<2;bar++)for(uint32_t beat=0;beat<4;beat++){
                int32_t x=20+(int32_t)bar*108+(int32_t)beat*23;
                uint8_t present=!bar||wm.loop_steps==32u;
                cv_rect(x,204,19,14,present?fg:guide);
                if(!present)cv_rect(x+2,206,15,10,bg);
            }
        }
        if(settings_index==1){const char *names[8]={"FREE","MAJOR","MINOR","DORIAN","MIN PENT","MAJ PENT","MIXOLYD","HARM MIN"};
            aa_text((240-width(&FONT_T,names[wm.scale]))/2,178,&FONT_T,names[wm.scale],fg);}
        row_locator(fg,guide);
        return;
    }
    if(settings_page==0)toggle=1;
    else if(settings_page==2){if(settings_index==1)toggle=1;else value=settings_index==0?percent(vol):project_slot+1u,percentage=settings_index==0;}
    else{
        if(settings_detail_kind==1){icon=ICO_CLOCK;timing=1;}
        else if(settings_detail_kind==2){icon=ICO_FEEDBACK;value=(wm.delay_feedback*67u+63u)/127u;percentage=1;}
        else if(settings_detail_kind==3){icon=ICO_ROOM;value=percent(wm.reverb_size);percentage=1;}
        else if(settings_detail_kind==4){icon=ICO_DAMP;value=percent(wm.reverb_damp);percentage=1;}
        else{value=percent(settings_index==0?wm.chorus_mix:settings_index==1?wm.delay_mix:wm.reverb_mix);percentage=1;}
    }
    settings_header(fg,guide);
    if(toggle){
        zoom_blit(72,47,icon,fg);
        switch_draw(64,165,112,43,setting_on(settings_index),fg,guide,bg);
    }else{
        mid(96,47,icon,fg);
        if(timing){
            const char *times[4]={"1/16","1/8","3/16","1/4"};const font_t *f=wm.extra_large?&FONT_X:&FONT_B;
            aa_text((240-width(f,times[wm.delay_div]))/2,119,f,times[wm.delay_div],fg);
        }else{large_value(value,117,percentage,fg);if(percentage)gauge(16,210,208,value,fg,guide);}
        if(settings_page==1){mini(174,54,setting_icon(),fg);if(!setting_on(settings_index))slash(178,58,24,fg);}
    }
    row_locator(fg,guide);
}
static void screen_settings(uint16_t bg,uint16_t fg,uint16_t pa,uint16_t pm,uint16_t guide,uint16_t a,uint16_t m,uint16_t l)
{
    screen_parameters(fg,pa,guide);
    (void)bg;(void)pm;(void)a;(void)m;(void)l;
}
#include "wanderer_edit_screen.c"
static void screen_focus(uint16_t fg,uint16_t guide,uint16_t accent)
{
    if(focus==7){screen_preset_name(fg,guide);return;}
    uint32_t value=focus==4?wm.bpm:focus==7?wm.sound+1u:percent(focus_value);
    status_header(focus_icon(),fg,guide,0);if(focus==3&&!wm.legacy)aa_text(48,7,&FONT_T,"SCATTER",fg);icon_blit(88,44,focus_icon(),fg);
    large_value(value,122,focus!=4&&focus!=7,fg);
    if(focus!=7)gauge(16,215,208,focus==4?(wm.bpm-60u)*100u/120u:value,fg,guide);
    else for(uint32_t b=0;b<10;b++){
        int32_t x=17+(int32_t)b*21;cv_rect(x,219,16,9,b==wm.sound/10u?fg:guide);
    }
    (void)accent;
}
static void screen_message(uint16_t fg,uint16_t guide,uint16_t accent)
{
    status_header(confirm==2?ICO_SAVE:confirm==3?ICO_LOAD:confirm==1||confirm==4?ICO_ERASE:confirm==5?ICO_ANCHOR:notice_icon(),fg,guide,0);
    if(confirm){
        if(confirm==2||confirm==3){
            card(12,47,216,119,wm.light?RGB(255,255,255):RGB(16,32,43),guide,0);
            icon_blit(25,73,confirm==2?ICO_SAVE:ICO_LOAD,fg);
            char b[12];number(project_slot+1u,b);const font_t *f=wm.extra_large?&FONT_X:&FONT_B;
            aa_text(196-width(f,b),77,f,b,fg);
            if(wp_has_anchor)aa_text(15,48,&FONT_T,"NOTES + PATCH",fg);
            tiny(26,139,slot_presence[project_slot]==2u?ICO_STORED:slot_presence[project_slot]==1u?ICO_EMPTY:ICO_BROKEN,fg);
            if(project_dirty)tiny(96,139,ICO_EDITED,fg);
        }else if(confirm==4){
            if(project_dirty)tiny(202,37,ICO_EDITED,fg);
            mini(40,54,ICO_ANCHOR,fg);mini(104,54,ICO_MEMORY,fg);mini(168,54,ICO_LAYER,fg);
            icon_blit(88,100,ICO_ERASE,fg);
        }else if(confirm==5){
            icon_blit(20,73,ICO_ANCHOR,fg);icon_blit(158,73,ICO_MEMORY,fg);
            diagram_arrow(98,104,42,fg);
        }else{zoom_blit(42,51,ICO_LAYER,fg);mid(145,108,ICO_ERASE,fg);}
        card(12,177,100,57,wm.light?RGB(255,255,255):RGB(16,32,43),guide,0);
        card(128,177,100,57,wm.light?RGB(255,255,255):RGB(16,32,43),guide,0);
        mid(38,181,ICO_CLOSE,fg);mid(154,181,ICO_CHECK,fg);
    }else if(notice==N_PREF_FAILED){
        uint16_t fill=wm.light?RGB(255,255,255):RGB(16,32,43);
        card(12,47,216,76,fill,guide,0);mid(28,60,ICO_SAVE,fg);mid(158,60,ICO_CHECK,fg);
        char b[12];number(project_slot+1u,b);aa_text(103,67,&FONT_W,b,fg);
        card(12,135,216,76,fill,guide,0);mid(28,148,ICO_CONTROLS,fg);mid(158,148,ICO_CLOSE,fg);
    }else if(notice==N_AUDIO_LOCK){
        icon_blit(88,60,ICO_LOCK,fg);text_center(137,&FONT_P,"RAM LOOP",fg);text_center(184,&FONT_T,"LOAD / NEW",fg);
    }else if(notice_success()){
        card(12,47,216,119,wm.light?RGB(255,255,255):RGB(16,32,43),guide,0);
        icon_blit(25,73,notice==N_SAVED?ICO_SAVE:ICO_LOAD,fg);
        char b[12];number(project_slot+1u,b);const font_t *f=wm.extra_large?&FONT_X:&FONT_B;
        aa_text(196-width(f,b),77,f,b,fg);mid(96,177,ICO_CHECK,fg);
    }else{
        if(notice==N_STOP_FIRST){
            icon_blit(22,67,ICO_PLAY,fg);icon_blit(154,67,ICO_STOP,fg);
            diagram_arrow(97,99,44,fg);
        }else zoom_blit(72,62,notice_icon(),fg);
        if(notice==N_SAVED||notice==N_LOADED){
            char b[12];number(project_slot+1u,b);aa_text(190-width(&FONT_H,b),126,&FONT_H,b,fg);
        }
        mid(96,177,notice_error()?ICO_CLOSE:ICO_CHECK,fg);
    }
    (void)accent;
}
static void screen_scope(uint16_t bg,uint16_t fg,uint16_t guide)
{
    status_header(ICO_SCOPE,fg,guide,0);
    if(scope_hold)mini(60,0,ICO_PAUSE,fg);
    card(10,43,220,145,bg,guide,0);
    for(int32_t x=16;x<224;x+=8)cv_rect(x,114,3,1,guide);
    cv_rect(120,49,1,133,guide);
    int32_t old=115;
    for(uint32_t x=0;x<208u;x++){
        uint32_t at=x*(SCOPE_N-1u)/207u;
        int32_t y=115-((int32_t)scope_trace[at]*(1<<scope_gain)*62)/32768;
        if(y<49)y=49;
        if(y>180)y=180;
        if(x)for(int32_t t=0;t<3;t++)cv_line(15+(int32_t)x,old+t-1,16+(int32_t)x,y+t-1,fg);
        old=y;
    }
    char b[12];aa_text(12,196,&FONT_T,"1 GAIN",guide);aa_text(84,196,&FONT_T,number(1u<<scope_gain,b),fg);
    aa_text(124,196,&FONT_T,"2 TIME",guide);aa_text(204,196,&FONT_T,number(scope_time+1u,b),fg);
    aa_text(12,219,&FONT_T,"3 HOLD",guide);aa_text(92,219,&FONT_T,scope_hold?"1":"0",fg);
    aa_text(124,219,&FONT_T,"4 LVL",guide);aa_text(190,219,&FONT_T,number(vol,b),fg);
}
#include "wanderer_splash.c"
static void screen(void)
{
    if(splash_active){splash_screen();return;}
    uint16_t bg=wm.light?RGB(238,246,246):RGB(8,17,25),fg=wm.light?INK:RGB(240,249,245);
    uint16_t pa=wm.light?RGB(255,255,255):RGB(16,32,43),pm=wm.light?RGB(247,247,255):RGB(23,31,49);
    uint16_t guide=wm.light?RGB(70,100,110):RGB(127,161,173);
    uint16_t a=wm.light?RGB(8,89,90):CYAN,m=wm.light?RGB(49,64,139):BLUE,l=wm.light?RGB(139,72,8):GOLD;
    if(scope_open)scope_snapshot();
    for(int32_t band=0;band<4;band++){
        cv_begin(240,60,bg);cv_oy=-band*60;
        if(scope_open)screen_scope(bg,fg,guide);
        else if(notice&&notice_error())screen_message(fg,guide,fg);
        else if(confirm)screen_message(fg,guide,l);
        else if(notice&&ui_time_before(notice_until))screen_message(fg,guide,fg);
        else if(editor_open)screen_editor(bg,fg,pa,guide);
        else if(settings_open)screen_settings(bg,fg,pa,pm,guide,a,m,l);
        else if(focus&&ui_time_before(focus_until))screen_focus(fg,guide,fg);
        else if(wm.extra_large)screen_zoom(bg,fg,pa,pm,guide,a,m,l);
        else screen_home(bg,fg,pa,pm,guide,a,m,l);
        cv_blit(0,(uint32_t)band*60u);
    }
    cv_oy=0;
}
