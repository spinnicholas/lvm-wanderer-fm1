/* SPDX-License-Identifier: GPL-3.0-only
 * Continuous 240-pixel scene rendered through four 60-pixel DMA bands.
 */
static const int8_t OX[16]={0,17,31,41,44,41,31,17,0,-17,-31,-41,-44,-41,-31,-17};
static const int8_t OY[16]={-44,-41,-31,-17,0,17,31,41,44,41,31,17,0,-17,-31,-41};
static char *number(uint32_t n,char *s)
{
    char t[12];uint32_t i=0,j=0;
    if(!n){s[0]='0';s[1]=0;return s;}
    while(n&&i<11u){t[i++]=(char)('0'+n%10u);n/=10u;}
    while(i)s[j++]=t[--i];
    s[j]=0;return s;
}
static void symbol_blit(int32_t x,int32_t y,uint32_t id,uint32_t size,
                        const uint8_t *base,uint16_t fg)
{
    const uint8_t *data=base+id*size*size/2;
    for(uint32_t gy=0;gy<size;gy++)for(uint32_t gx=0;gx<size;gx++){
        uint32_t k=gy*size+gx,alpha=k&1u?data[k/2]&15u:data[k/2]>>4;
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
static void icon_blit(int32_t x,int32_t y,uint32_t id,uint16_t fg)
{symbol_blit(x,y,id,ICON_SIZE,ICON_ALPHA,fg);}
static void zoom_blit(int32_t x,int32_t y,uint32_t id,uint16_t fg)
{symbol_blit(x,y,id,ZOOM_SIZE,ICON_ZOOM_ALPHA,fg);}
static void disk(int32_t cx,int32_t cy,int32_t r,uint16_t col)
{
    for(int32_t y=-r;y<=r;y++)for(int32_t x=-r;x<=r;x++)
        if(x*x+y*y<=r*r)cv_pset(cx+x,cy+y,col);
}
static void panel(int32_t x,int32_t y,int32_t w,int32_t h,uint16_t fill)
{
    cv_rect(x+5,y,w-10,h,fill);
    cv_rect(x,y+5,w,h-10,fill);
    cv_rect(x+2,y+2,w-4,h-4,fill);
}
static void card_edge(int32_t x,int32_t y,int32_t w,int32_t h,
                      uint16_t col,uint8_t selected)
{
    /* One continuous visual cue for the listening/editing target. */
    cv_rect(x+12,y+1,w-24,selected?3:1,col);
    if(selected)cv_rect(x+2,y+11,3,h-22,col);
}
static void transport_symbol(int32_t x,int32_t y,uint16_t col)
{
    if(recording){disk(x+10,y+10,9,col);return;}
    if(!running){cv_rect(x+2,y+2,17,17,col);return;}
    for(int32_t i=0;i<18;i++)cv_line(x+2,y+2+i,x+2+(i<9?i:17-i)*2,y+2+i,col);
}
static void switch_draw(int32_t x,int32_t y,uint8_t on,uint16_t lit,uint16_t idle,uint16_t face)
{
    uint16_t rail=on?lit:idle;
    cv_rect(x+13,y,34,27,rail);cv_rect(x+5,y+5,50,17,rail);
    disk(x+13,y+13,13,rail);disk(x+47,y+13,13,rail);
    disk(x+(on?46:14),y+13,10,face);
    disk(x+(on?46:14),y+13,4,on?lit:idle);
}
static void orbit_pixels(int32_t cx,int32_t cy,int32_t r,uint16_t col)
{
    int32_t x=r,y=0,d=1-r;
    while(y<=x){
        cv_pset(cx+x,cy+y,col);cv_pset(cx+y,cy+x,col);
        cv_pset(cx-x,cy+y,col);cv_pset(cx-y,cy+x,col);
        cv_pset(cx+x,cy-y,col);cv_pset(cx+y,cy-x,col);
        cv_pset(cx-x,cy-y,col);cv_pset(cx-y,cy-x,col);
        y++;
        if(d<0)d+=2*y+1;
        else{x--;d+=2*(y-x)+1;}
    }
}
static void orbit(int32_t cx,int32_t cy,int32_t r,uint16_t col)
{
    orbit_pixels(cx,cy,r,col);
    orbit_pixels(cx,cy,r-1,col);
}
static void phrase_orbit(const wm_phrase_t *p,int32_t cx,int32_t cy,
                         uint8_t selected,uint16_t col,uint16_t guide,uint16_t face)
{
    orbit(cx,cy,43,guide);
    for(uint32_t i=0;i<WM_STEPS;i++){
        int32_t x=cx+OX[i]*43/44,y=cy+OY[i]*43/44;
        disk(x,y,p->e[i].velocity?4:3,p->e[i].velocity?col:guide);
        if(running&&i==step_index){
            disk(x,y,7,col);disk(x,y,4,face);
            if(p->e[i].velocity)disk(x,y,2,col);
        }
    }
    if(selected){disk(cx,cy+53,3,col);cv_rect(cx-16,cy+58,32,3,col);}
}
static void layer_steps(const wm_phrase_t *p,uint32_t active,uint16_t col,uint16_t guide)
{
    cv_rect(82,213,142,2,guide);
    for(uint32_t i=0;i<WM_STEPS;i++){
        int32_t x=85+(int32_t)i*9;
        disk(x,214,p->e[i].velocity?3:1,p->e[i].velocity?col:guide);
        if(running&&i==active)cv_rect(x-2,205,5,18,col);
    }
}
static uint32_t focus_icon(void)
{
    switch(focus){
    case 1:return ICO_MELT;case 2:return ICO_DRIFT;case 3:return ICO_DECAY;
    case 4:return ICO_TEMPO;case 5:return ICO_LEVEL;case 7:return ICO_SOUND;default:return ICO_SIZE;
    }
}
static uint32_t notice_icon(void)
{
    switch(notice){
    case N_ORIGINAL_SAFE:case N_ANCHOR_SAFE:return ICO_SAFE;
    case N_ANCHORED:case N_CATCH_FIRST:return ICO_ANCHOR;
    case N_BRANCHED:return ICO_BRANCH;
    case N_UNDO:case N_NO_UNDO:return ICO_UNDO;
    case N_STILL:return ICO_STOP;case N_LIVE:return ICO_PLAY;
    case N_LAYER_CLEARED:return ICO_ERASE;
    case N_NEW_SESSION:return ICO_NEW;
    case N_SAVE_FAILED:case N_SAVED:return ICO_SAVE;
    case N_NO_VALID_SAVE:case N_LOADED:return ICO_LOAD;
    case N_STOP_FIRST:return ICO_STOP;
    default:return ICO_MEMORY;
    }
}
static uint8_t notice_error(void)
{
    return notice==N_NO_PHRASE||notice==N_CATCH_FIRST||notice==N_NO_UNDO||
      notice==N_SAVE_FAILED||notice==N_NO_VALID_SAVE||notice==N_STOP_FIRST;
}
static void screen_home(uint16_t bg,uint16_t fg,uint16_t pa,uint16_t pm,
                        uint16_t guide,uint16_t a,uint16_t m,uint16_t l)
{
    aa_text(16,5,&FONT_N,"WANDERER",fg);
    transport_symbol(206,6,recording?l:running?a:guide);
    cv_rect(16,34,208,2,guide);
    if(running)cv_rect(16+(int32_t)step_index*13,32,13,6,a);
    panel(8,43,108,117,pa);panel(124,43,108,117,pm);
    card_edge(8,43,108,117,a,anchor_view);
    card_edge(124,43,108,117,m,!anchor_view);
    phrase_orbit(&wm.anchor,62,100,anchor_view,a,guide,pa);
    phrase_orbit(&wm.memory,178,100,!anchor_view,m,guide,pm);
    icon_blit(30,68,ICO_ANCHOR,wm.has_anchor?a:guide);
    icon_blit(146,68,ICO_MEMORY,anchor_view?guide:m);
    panel(8,167,224,66,pa);
    card_edge(8,167,224,66,l,wm.layer_enabled);
    icon_blit(16,168,ICO_LAYER,l);
    switch_draw(169,174,wm.layer_enabled,l,guide,wm.light?RGB(255,255,255):fg);
    layer_steps(&wm.layer,step_index,l,guide);
}
static void screen_zoom(uint16_t bg,uint16_t fg,uint16_t pa,uint16_t pm,
                        uint16_t guide,uint16_t a,uint16_t m,uint16_t l)
{
    uint8_t original=anchor_view&&wm.has_anchor;
    const wm_phrase_t *p=original?&wm.anchor:&wm.memory;
    uint16_t col=original?a:m,face=original?pa:pm;
    aa_text(16,5,&FONT_N,"WANDERER",fg);
    transport_symbol(206,6,recording?l:running?a:guide);
    cv_rect(16,34,208,2,guide);
    panel(8,41,224,152,face);
    card_edge(8,41,224,152,col,1);
    orbit(120,115,70,guide);
    for(uint32_t i=0;i<WM_STEPS;i++){
        int32_t x=120+OX[i]*70/44,y=115+OY[i]*70/44;
        disk(x,y,p->e[i].velocity?5:3,p->e[i].velocity?col:guide);
        if(running&&i==step_index){
            disk(x,y,8,col);disk(x,y,4,face);
            if(p->e[i].velocity)disk(x,y,2,col);
        }
    }
    zoom_blit(72,67,original?ICO_ANCHOR:ICO_MEMORY,col);
    panel(8,198,224,36,pa);
    card_edge(8,198,224,36,l,wm.layer_enabled);
    symbol_blit(18,200,0,SMALL_SIZE,ICON_LAYER_SMALL_ALPHA,l);
    cv_rect(65,220,92,2,guide);
    for(uint32_t i=0;i<WM_STEPS;i++)
        if(wm.layer.e[i].velocity)disk(67+(int32_t)i*6,221,2,l);
    switch_draw(169,203,wm.layer_enabled,l,guide,wm.light?RGB(255,255,255):fg);
    (void)bg;
}
static void screen_settings(uint16_t bg,uint16_t fg,uint16_t pa,uint16_t pm,
                            uint16_t guide,uint16_t a,uint16_t m,uint16_t l)
{
    uint32_t system_icons[3]={ICO_SUN,ICO_SIZE,ICO_STILL};
    uint32_t effect_icons[3]={ICO_CHORUS,ICO_DELAY,ICO_REVERB};
    uint8_t system_on[3]={wm.light,wm.extra_large,wm.reduced_motion};
    uint8_t effect_on[3]={wm.fx_on&1u,(wm.fx_on>>1)&1u,(wm.fx_on>>2)&1u};
    uint8_t amounts[3]={wm.chorus_mix,wm.delay_mix,wm.reverb_mix};
    uint16_t colors[3]={l,a,m};
    char buf[12];
    aa_text(16,5,&FONT_N,"WANDERER",fg);
    cv_rect(16,34,208,2,guide);
    cv_rect(settings_page?122:16,34,settings_page?102:102,3,
            settings_page?m:l);
    cv_rect(212,3,6,5,guide);cv_rect(212,26,6,5,guide);
    cv_rect(201,14,6,6,guide);cv_rect(223,14,6,6,guide);
    cv_line(205,7,209,11,guide);cv_line(221,23,225,27,guide);
    cv_line(205,27,209,23,guide);cv_line(221,11,225,7,guide);
    disk(215,17,10,guide);disk(215,17,5,bg);
    for(uint32_t i=0;i<3u;i++){
        int32_t y=42+(int32_t)i*64;
        panel(8,y,224,59,i==1?pm:pa);
        card_edge(8,y,224,59,settings_index==i?colors[i]:guide,settings_index==i);
        if(settings_index==i&&!settings_page){
            cv_rect(94,y+52,55,3,colors[i]);
        }
        icon_blit(22,y-2,settings_page?effect_icons[i]:system_icons[i],colors[i]);
        if(settings_page){
            uint32_t value=amounts[i];
            if(settings_index==i&&settings_detail&&fm1_ms<settings_detail_until)
                value=settings_detail;
            number(value,buf);
            aa_text(157-width(&FONT_N,buf),y+12,&FONT_N,buf,fg);
            cv_rect(96,y+49,60,2,guide);
            cv_rect(96,y+49,(int32_t)amounts[i]*60/127,3,colors[i]);
        }
        switch_draw(166,y+16,settings_page?effect_on[i]:system_on[i],colors[i],guide,
          wm.light?RGB(255,255,255):fg);
    }
}
static void screen_focus(uint16_t fg,uint16_t guide,uint16_t accent)
{
    char buf[16];uint32_t v=focus==4?((uint32_t)wm.bpm-60u)*127u/120u:
      focus==6?(wm.extra_large?127u:0u):
      focus==7?(uint32_t)wm.sound*127u/(WM_PRESET_COUNT-1u):focus_value;
    aa_text(16,5,&FONT_N,"WANDERER",fg);
    cv_rect(16,34,208,2,guide);
    orbit(65,121,55,guide);
    for(uint32_t i=0;i<16u;i++)
        disk(65+OX[i]*55/44,121+OY[i]*55/44,3,i*127u/15u<=v?accent:guide);
    icon_blit(33,89,focus_icon(),accent);
    number(focus==4?wm.bpm:focus_value,buf);
    aa_text(231-width(&FONT_H,buf),98,&FONT_H,buf,fg);
    cv_rect(20,205,200,7,guide);
    if(v)cv_rect(20,205,(int32_t)v*200/127,7,accent);
    disk(20+(int32_t)v*200/127,208,8,accent);
}
static void screen_message(uint16_t fg,uint16_t guide,uint16_t accent)
{
    uint32_t icon=confirm?(confirm==1?ICO_ERASE:confirm==2?ICO_SAVE:
      confirm==3?ICO_LOAD:ICO_NEW):notice_icon();
    aa_text(16,5,&FONT_N,"WANDERER",fg);
    cv_rect(16,34,208,2,guide);
    orbit(120,123,70,guide);
    for(uint32_t i=0;i<16u;i++)
        disk(120+OX[i]*70/44,123+OY[i]*70/44,2,guide);
    icon_blit(88,91,icon,accent);
    if(confirm){
        icon_blit(40,166,ICO_CLOSE,guide);
        icon_blit(142,166,ICO_CHECK,accent);
        cv_rect(64,223,112,3,guide);
    }else{
        uint32_t remaining=notice_until>fm1_ms?notice_until-fm1_ms:0u;
        if(remaining>1200u)remaining=1200u;
        icon_blit(142,148,notice_error()?ICO_CLOSE:ICO_CHECK,accent);
        cv_rect(32,220,176,4,guide);
        cv_rect(32,220,(int32_t)remaining*176/1200,4,accent);
    }
}
static void screen(void)
{
    uint16_t bg=wm.light?RGB(234,246,243):RGB(5,17,23);
    uint16_t fg=wm.light?INK:RGB(240,249,245);
    uint16_t pa=wm.light?RGB(251,255,251):RGB(10,38,43);
    uint16_t pm=wm.light?RGB(241,245,252):RGB(21,32,53);
    uint16_t guide=wm.light?RGB(145,170,167):RGB(75,108,116);
    uint16_t a=wm.light?RGB(10,91,92):CYAN;
    uint16_t m=wm.light?RGB(51,64,143):BLUE;
    uint16_t l=wm.light?RGB(138,74,8):GOLD;
    for(int32_t band=0;band<4;band++){
        cv_begin(240,60,bg);
        cv_oy=-band*60;
        if(settings_open)screen_settings(bg,fg,pa,pm,guide,a,m,l);
        else if(confirm)screen_message(fg,guide,l);
        else if(notice&&fm1_ms<notice_until)
            screen_message(fg,guide,notice_error()?l:a);
        else if(focus&&fm1_ms<focus_until)
            screen_focus(fg,guide,focus==1?a:focus==2?m:l);
        else if(wm.extra_large)screen_zoom(bg,fg,pa,pm,guide,a,m,l);
        else screen_home(bg,fg,pa,pm,guide,a,m,l);
        cv_blit(0,(uint32_t)band*60u);
    }
    cv_oy=0;
}
