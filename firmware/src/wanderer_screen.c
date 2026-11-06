/* SPDX-License-Identifier: GPL-3.0-only; icon-led Home and consistent control rows. */
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
    int32_t top=y+cv_oy;
    uint32_t first=top<0?(uint32_t)-top:0u;
    for(uint32_t gy=first;gy<size&&top+(int32_t)gy<(int32_t)cv_h;gy++){
        const uint8_t *data=base->data+base->row[id*size+gy];
        for(uint32_t gx=0;gx<size;){
            uint32_t token=*data++,alpha=token>>4,count=(token&15u)+1u;
            if(alpha)alpha_span(x+(int32_t)gx,top+(int32_t)gy,count,alpha,fg);
            gx+=count;
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
    if(cy+r+cv_oy<0||cy-r+cv_oy>=(int32_t)cv_h)return;
    int32_t span=r;
    for(int32_t y=0;y<=r;y++){
        while(span*span+y*y>r*r)span--;
        cv_rect(cx-span,cy+y,2*span+1,1,col);
        if(y)cv_rect(cx-span,cy-y,2*span+1,1,col);
    }
}
static void panel(int32_t x,int32_t y,int32_t w,int32_t h,uint16_t fill)
{
    int32_t r=h/2<7?h/2:7;
    cv_rect(x+r,y,w-2*r,h,fill);cv_rect(x,y+r,w,h-2*r,fill);
    disk(x+r,y+r,r,fill);disk(x+w-r-1,y+r,r,fill);
    disk(x+r,y+h-r-1,r,fill);disk(x+w-r-1,y+h-r-1,r,fill);
}
static uint16_t mix565(uint16_t a,uint16_t b,uint32_t t)
{
    uint32_t u=256u-t;
    return (uint16_t)(((((a>>11)*u+(b>>11)*t)>>8)<<11)|(((((a>>5)&63u)*u+((b>>5)&63u)*t)>>8)<<5)|(((a&31u)*u+(b&31u)*t)>>8));
}
/* Rounded panel with a soft vertical gradient; only rows in the current LCD band are drawn. */
static void soft_panel(int32_t x,int32_t y,int32_t w,int32_t h,uint16_t top,uint16_t bottom)
{
    int32_t r=h/2<7?h/2:7;
    for(int32_t row=0;row<h;row++){
        int32_t py=y+row+cv_oy;if(py<0||py>=(int32_t)cv_h)continue;
        int32_t dy=row<r?r-row:row>=h-r?row-(h-1-r):0,span=r;
        while(span>0&&span*span+dy*dy>r*r)span--;
        int32_t inset=dy?r-span:0;
        cv_rect(x+inset,y+row,w-2*inset,1,mix565(top,bottom,(uint32_t)(row*256/(h>1?h-1:1))));
    }
}
static void card(int32_t x,int32_t y,int32_t w,int32_t h,uint16_t fill,uint16_t edge,uint8_t selected)
{
    uint32_t inset=selected?3u:1u;panel(x,y,w,h,edge);
    /* A light from above: the top of every card leans a little toward its edge colour. */
    soft_panel(x+(int32_t)inset,y+(int32_t)inset,w-2*(int32_t)inset,h-2*(int32_t)inset,mix565(fill,edge,selected?72u:40u),fill);
}
static void transport_symbol(int32_t x,int32_t y,uint16_t col)
{
    if(!running){cv_rect(x+3,y+3,15,15,col);return;}
    for(int32_t i=0;i<18;i++)cv_line(x+2,y+2+i,x+2+(i<9?i:17-i)*2,y+2+i,col);
}
static void section_rule(uint16_t accent,uint16_t guide)
{
    cv_rect(12,33,216,1,guide);
    cv_rect(12,31,34,4,accent);
}
/* One switch for the edit and record target, visible in both Home layouts. */
static void home_layer_header(uint16_t fg,uint16_t guide)
{
    const wm_theme_t *theme=theme_palette();
    aa_text(12,7,&FONT_T,"LAYER",fg);
    card(101,3,79,27,theme->bg,fg,0);
    for(uint32_t lane=0;lane<2;lane++){
        int32_t x=103+(int32_t)lane*38;
        const char *label=lane?"2":"1";
        uint8_t selected=lane==layer_focus;
        if(selected)panel(x,5,37,23,fg);
        aa_text(x+(37-width(&FONT_T,label))/2,7,&FONT_T,label,selected?theme->bg:fg);
    }
    if(recording)disk(193,16,6,fg);
    transport_symbol(209,6,fg);
    section_rule(anchor_view?theme->anchor:layer_focus?theme->layer:theme->evolve,guide);
}
static void status_header(uint32_t context,const char *title,uint16_t fg,uint16_t guide)
{
    mini(8,0,context,fg);
    if(title)aa_text(48,7,&FONT_T,title,fg);
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
/* Sixteen chunky segments: readable at a glance, brighter as they fill. */
static void meter(int32_t x,int32_t y,int32_t w,int32_t h,uint32_t pct,uint16_t fg,uint16_t guide,uint16_t accent)
{
    if(pct>100u)pct=100u;
    uint32_t lit=(pct*16u+99u)/100u;int32_t seg=w/16;
    for(uint32_t i=0;i<16u;i++){
        int32_t sx=x+(int32_t)i*seg;
        if(i<lit)panel(sx,y,seg-4,h,mix565(accent,fg,i*12u));
        else panel(sx+(seg-4)/2-2,y+h/2-2,5,5,guide);
    }
}
/* A miniature piano roll: each note a rounded bar at its pitch and length.
 * ghost (thin) shows the Anchor beneath Memory, so the wander is visible. */
static uint8_t roll_event(const wm_phrase_t *p,uint32_t s,uint8_t heard,wm_event_t *out)
{
    uint8_t found=0;
    for(uint32_t v=0;v<WM_POLY;v++){
        wm_event_t e=wm_get(p,s,v);if(!e.velocity)continue;
        if(heard)e=ww_apply(e,s,v);
        if(e.velocity&&(!found||e.note>out->note)){*out=e;found=1;}
    }
    return found;
}
static void piano_roll(int32_t x,int32_t y,int32_t w,int32_t h,const wm_phrase_t *main,const wm_phrase_t *ghost,
    uint8_t heard,uint16_t col,uint16_t ghost_col,uint16_t guide,uint8_t playing)
{
    if(y+h+cv_oy<0||y+cv_oy>=(int32_t)cv_h)return;
    uint32_t L=wm.loop_steps;int32_t lo=127,hi=0;wm_event_t e;
    for(uint32_t s=0;s<L;s++){
        if(roll_event(main,s,heard,&e)){if(e.note<lo)lo=e.note;if(e.note>hi)hi=e.note;}
        if(ghost&&roll_event(ghost,s,0,&e)){if(e.note<lo)lo=e.note;if(e.note>hi)hi=e.note;}
    }
    for(int32_t gx=x;gx<x+w;gx+=6)cv_rect(gx,y+h-1,3,1,guide);
    if(lo>hi)return;
    int32_t span=hi-lo<12?12:hi-lo,mid=(hi+lo)/2;lo=mid-span/2;
    for(uint32_t pass=ghost?0u:1u;pass<2u;pass++){
        const wm_phrase_t *p=pass?main:ghost;int32_t px=-1,py=0;
        for(uint32_t s=0;s<L;s++){
            if(!roll_event(p,s,(uint8_t)(pass&&heard),&e))continue;
            int32_t x0=x+(int32_t)(s*(uint32_t)w/L),len=(int32_t)(e.gate*(uint32_t)w/(L*256u));
            if(len<5)len=5;if(x0+len>x+w)len=x+w-x0;
            int32_t yy=y+h-6-(e.note-lo)*(h-12)/span;
            if(pass){
                if(px>=0){cv_line(px,py,x0,yy,col);cv_line(px,py+1,x0,yy+1,col);}
                panel(x0,yy-3,len,7,col);px=x0+len-1;py=yy;
            }else cv_rect(x0,yy,len,2,ghost_col);
        }
        if(pass&&heard)for(uint32_t s=0;s<L;s++){
            wm_event_t d=ww_drip_event(s);if(!d.velocity)continue;
            disk(x+(int32_t)(s*(uint32_t)w/L)+3,y+h-6-((int32_t)d.note-lo)*(h-12)/span,3,col);
        }
    }
    if(playing){int32_t at=x+(int32_t)(((step_index+L-1u)%L)*(uint32_t)w/L);cv_rect(at,y,2,h,col);}
}
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
    if(y+13+cv_oy<0||y-5+cv_oy>=(int32_t)cv_h)return;
    /* Union held gates once per onset, then paint each occupied cell once. */
    uint32_t length=wm.loop_steps,wrap=length-1u,onsets=0,holds=0;
    for(uint32_t i=0;i<length;i++){
        uint32_t gate=0;
        for(uint32_t v=0;v<WM_POLY;v++){
            wm_event_t event=wm_get(p,i,v);
            if(!event.velocity)continue;
            onsets|=1u<<i;
            if(event.gate>gate)gate=event.gate;
        }
        if(gate>256u){
            uint32_t count=(gate-1u)>>8;
            if(count>wrap)count=wrap;
            uint32_t start=(i+1u)&wrap,bits=(1u<<count)-1u;
            holds|=bits<<start;
            if(start)holds|=bits>>(length-start);
        }
    }
    uint32_t playhead=(step_index-1u)&wrap;
    for(uint32_t i=0;i<length;i++){
        uint32_t pos=i&15u,bit=1u<<i;
        int32_t px=x+(int32_t)pos*spacing+(int32_t)(pos/4u)*group;
        int32_t py=y+(length==32u?(int32_t)(i/16u)*8-2:4);
        cv_rect(px,py,spacing-2,(onsets&bit)?(length==32u?4:8):1,(onsets&bit)?col:guide);
        if(holds&bit)cv_rect(px,py+2,spacing-2,2,col);
        if(running&&audible&&i==playhead)cv_rect(px-1,py-3,spacing,2,col);
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
    case N_RETURNED:return ICO_ANCHOR;case N_NOTHING_TO_KEEP:return ICO_EMPTY;case N_THEME:return ICO_SUN;
    case N_SAVED:return ICO_SAVE;case N_LOADED:return ICO_LOAD;
    default:return ICO_MEMORY;}
}
/* Every message pairs its symbol with a word. */
static const char *notice_word(void)
{
    switch(notice){
    case N_ORIGINAL_SAFE:return "RESTORED";case N_ANCHORED:return "ANCHOR SET";
    case N_ANCHOR_SAFE:return "ANCHOR IS SAFE";case N_NO_PHRASE:return "NO NOTES";
    case N_BRANCHED:return "KEPT";case N_UNDO:return "UNDONE";case N_NO_UNDO:return "NOTHING TO UNDO";
    case N_STILL:return "STILL";case N_LIVE:return "LIVE";case N_LAYER_CLEARED:return "LAYER 2 CLEAR";
    case N_NEW_SESSION:return "NEW SESSION";case N_STOP_FIRST:return "STOP FIRST";
    case N_RETURNED:return "BACK HOME";case N_NOTHING_TO_KEEP:return "NOT WANDERED";
    case N_THEME:return theme_palette()->name;case N_SAVE_FAILED:return "SAVE FAILED";
    case N_NO_VALID_SAVE:return "EMPTY SLOT";default:return 0;}
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
static void screen_home(uint16_t fg,uint16_t guide)
{
    const wm_theme_t *theme=theme_palette();
    uint8_t anchored=(uint8_t)(wm.has_anchor||wp_has_anchor);
    const wm_phrase_t *memory=layer_focus?&wm.layer:&wm.memory;
    home_layer_header(fg,guide);
    card(5,42,112,166,theme->panel,anchor_view?theme->anchor:guide,anchor_view);
    card(123,42,112,166,theme->memory,anchor_view?guide:theme->evolve,!anchor_view);

    tiny(16,49,anchored?ICO_LOCK:ICO_NEW,fg);
    if(anchor_view)tiny(84,49,ICO_LISTEN,fg);
    if(wp_active||wp_armed||wp_writing){
        const char *state=wp_writing?"WRITE":wp_armed?"ARM":"RAM";
        aa_text(133,51,&FONT_T,state,fg);
    }else if(layer_focus&&!wm.layer_enabled){
        tiny(132,49,ICO_MUTE,fg);
    }
    /* Wander: how far Memory has travelled from its source, and for how many loops. */
    uint8_t wander=(uint8_t)(!layer_focus&&ww_active()&&!wp_active&&wm_memory_playable());
    if(recording)disk(216,60,5,fg);
    else if(wander&&ww.loops&&!wp_armed&&!wp_writing){
        char b[12];number(ww.loops>99u?99u:ww.loops,b);
        aa_text(226-width(&FONT_T,b),51,&FONT_T,b,fg);
    }
    if(wm.extra_large){
        zoom_blit(13,50,ICO_ANCHOR,theme->anchor);
        zoom_blit(131,50,ICO_MEMORY,theme->evolve);
    }else{
        mid(37,70,ICO_ANCHOR,theme->anchor);
        mid(155,70,ICO_MEMORY,theme->evolve);
    }
    int32_t label_y=wm.extra_large?148:121,bar_y=label_y+21,roll_y=bar_y+11,roll_h=204-roll_y;
    aa_text(61-width(&FONT_T,"ANCHOR")/2,label_y,&FONT_T,"ANCHOR",fg);
    aa_text(179-width(&FONT_T,"MEMORY")/2,label_y,&FONT_T,"MEMORY",fg);
    if(wander){
        int32_t fill=ww.distance*92/100;
        panel(133,bar_y,92,7,guide);panel(134,bar_y+1,90,5,theme->memory);
        if(fill>3)panel(133,bar_y,fill,7,theme->evolve);
    }
    /* Melody pictures: the Anchor as recorded; Memory as you hear it now,
     * over a faint line of the Anchor so its journey can be seen. */
    piano_roll(14,roll_y,94,roll_h,&wm.anchor,0,0,theme->anchor,guide,guide,(uint8_t)(running&&anchor_view&&!wp_active));
    piano_roll(132,roll_y,94,roll_h,memory,!layer_focus&&anchored?&wm.anchor:0,wander,theme->evolve,guide,guide,
        (uint8_t)(running&&!anchor_view&&!wp_active&&(!layer_focus||wm.layer_enabled)));
    project_footer(fg,guide);
}
static void diagram_arrow(int32_t x,int32_t y,int32_t w,uint16_t fg)
{
    for(int32_t t=-1;t<=1;t++){
        cv_line(x,y+t,x+w,y+t,fg);
        cv_line(x+w-7,y-7+t,x+w,y+t,fg);
        cv_line(x+w-7,y+7+t,x+w,y+t,fg);
    }
}
#include "wanderer_edit_screen.c"
static void screen_focus(uint16_t fg,uint16_t guide)
{
    if(focus==7){screen_preset_name(fg,guide);return;}
    const char *titles[]={"","MELT","DRIFT","SCATTER","TEMPO","LEVEL",""};
    uint32_t value=focus==4?wm.bpm:percent(focus_value);
    status_header(focus_icon(),focus==3&&wm.legacy?"DECAY":titles[focus],fg,guide);
    icon_blit(88,44,focus_icon(),fg);
    large_value(value,122,focus!=4,fg);
    /* A plain word for where the knob sits, in Normal size. */
    if(!wm.extra_large&&focus>=1&&focus<=3&&!(focus==3&&wm.legacy)){
        static const char *const words[3][4]={{"DRY","SOFT","MELTING","LIQUID"},
            {"STILL","GENTLE","WANDERING","FAR"},{"CLEAN","SPARSE","BUSY","WILD"}};
        uint32_t band=focus_value?1u+(focus_value>=48u)+(focus_value>=96u):0u;
        text_center(188,&FONT_T,words[focus-1u][band],fg);
    }
    meter(16,212,208,18,focus==4?(wm.bpm-60u)*100u/120u:value,fg,guide,theme_palette()->evolve);
}
static void screen_message(uint16_t fg,uint16_t guide)
{
    static const char *const actions[]={"","CLEAR 2","SAVE","LOAD","NEW SESSION","RESTORE"};
    status_header(confirm==2?ICO_SAVE:confirm==3?ICO_LOAD:confirm==1||confirm==4?ICO_ERASE:confirm==5?ICO_ANCHOR:notice_icon(),
        confirm?actions[confirm]:0,fg,guide);
    if(confirm){
        if(confirm==2||confirm==3){
            card(12,47,216,119,theme_palette()->panel,guide,0);
            icon_blit(25,73,confirm==2?ICO_SAVE:ICO_LOAD,fg);
            char b[12];number(project_slot+1u,b);const font_t *f=wm.extra_large?&FONT_X:&FONT_B;
            aa_text(196-width(f,b),77,f,b,fg);
            const char *state=slot_presence[project_slot]==2u?(confirm==2?"OVERWRITE":"SAVED SLOT"):
                slot_presence[project_slot]==1u?"EMPTY SLOT":"CHECK SLOT";
            text_center(50,&FONT_T,state,fg);
            if(confirm==2&&wp_has_anchor)text_center(146,&FONT_T,"NOTES + PATCH",fg);
            else if(project_dirty&&confirm==3)text_center(146,&FONT_T,"UNSAVED EDITS",fg);
        }else if(confirm==4){
            if(project_dirty)tiny(202,37,ICO_EDITED,fg);
            mini(40,54,ICO_ANCHOR,fg);mini(104,54,ICO_MEMORY,fg);mini(168,54,ICO_LAYER,fg);
            icon_blit(88,100,ICO_ERASE,fg);
        }else if(confirm==5){
            icon_blit(20,73,ICO_ANCHOR,fg);icon_blit(158,73,ICO_MEMORY,fg);
            diagram_arrow(98,104,42,fg);
        }else{zoom_blit(42,51,ICO_LAYER,fg);mid(145,108,ICO_ERASE,fg);}
        card(12,177,100,57,theme_palette()->panel,guide,0);
        card(128,177,100,57,theme_palette()->panel,guide,0);
        mini(46,178,ICO_CLOSE,fg);mini(162,178,ICO_CHECK,fg);
        aa_text(62-width(&FONT_T,"HOME")/2,210,&FONT_T,"HOME",fg);
        aa_text(178-width(&FONT_T,"SELECT")/2,210,&FONT_T,"SELECT",fg);
    }else if(notice==N_PREF_FAILED){
        uint16_t fill=theme_palette()->panel;
        card(12,47,216,76,fill,guide,0);mid(28,60,ICO_SAVE,fg);mid(158,60,ICO_CHECK,fg);
        char b[12];number(project_slot+1u,b);aa_text(103,67,&FONT_W,b,fg);
        card(12,135,216,76,fill,guide,0);mid(28,148,ICO_CONTROLS,fg);mid(158,148,ICO_CLOSE,fg);
    }else if(notice==N_AUDIO_LOCK){
        icon_blit(88,60,ICO_LOCK,fg);text_center(137,&FONT_P,"RAM LOOP",fg);text_center(184,&FONT_T,"LOAD / NEW",fg);
    }else if(notice==N_CATCH_FIRST){
        icon_blit(88,53,ICO_ANCHOR,fg);
        text_center(133,&FONT_P,"NO ANCHOR",fg);
        const char *next=wm_memory_playable()?(recording?"TAP REC TO FINISH":"TAP LFO TO LISTEN"):
            "RECORD LAYER 1";
        text_center(174,&FONT_T,next,fg);
    }else if(notice==N_ANCHOR_LISTEN){
        zoom_blit(72,54,ICO_LOCK,fg);
        text_center(157,&FONT_T,"ANCHOR LISTEN",fg);
        text_center(188,&FONT_T,"TAP LFO TO EXIT",fg);
    }else if(notice_success()){
        card(12,47,216,119,theme_palette()->panel,guide,0);
        icon_blit(25,73,notice==N_SAVED?ICO_SAVE:ICO_LOAD,fg);
        char b[12];number(project_slot+1u,b);const font_t *f=wm.extra_large?&FONT_X:&FONT_B;
        aa_text(196-width(f,b),77,f,b,fg);mid(96,177,ICO_CHECK,fg);
    }else{
        const char *word=notice_word();
        if(notice==N_STOP_FIRST){
            icon_blit(22,60,ICO_PLAY,fg);icon_blit(154,60,ICO_STOP,fg);
            diagram_arrow(97,92,44,fg);
        }else if(word)zoom_blit(72,44,notice_icon(),fg);
        else zoom_blit(72,62,notice_icon(),fg);
        if(word)zoom_text(8,140,224,40,34,word,0,fg);
        mid(96,186,notice_error()?ICO_CLOSE:ICO_CHECK,fg);
    }
}
static void scope_control(int32_t x,int32_t y,const char *label,const char *value,uint16_t fg,uint16_t guide)
{
    card(x,y,108,44,theme_palette()->panel,guide,0);
    aa_text(x+8,y+4,&FONT_T,label,fg);
    aa_text(x+99-width(&FONT_T,value),y+23,&FONT_T,value,fg);
}
static void screen_scope(uint16_t bg,uint16_t fg,uint16_t guide)
{
    status_header(ICO_SCOPE,scope_hold?"HOLD":"SCOPE",fg,guide);
    card(8,42,224,98,bg,guide,0);
    for(int32_t x=16;x<224;x+=8)cv_rect(x,90,3,1,guide);
    cv_rect(120,48,1,85,guide);
    int32_t old=90;
    for(uint32_t x=0;x<208u;x++){
        uint32_t at=x*(SCOPE_N-1u)/207u;
        int32_t y=90-((int32_t)scope_trace[at]*(1<<scope_gain)*41)/32768;
        y=ws_clamp(y,48,132);
        if(x)for(int32_t t=0;t<3;t++)cv_line(15+(int32_t)x,old+t-1,16+(int32_t)x,y+t-1,fg);
        old=y;
    }
    char b[12];
    scope_control(8,145,"1 GAIN",number(1u<<scope_gain,b),fg,guide);
    scope_control(124,145,"2 TIME",number(scope_time+1u,b),fg,guide);
    scope_control(8,193,"3 HOLD",scope_hold?"ON":"OFF",fg,guide);
    scope_control(124,193,"4 LEVEL",number(vol,b),fg,guide);
}
#include "wanderer_splash.c"
static void screen(void)
{
    if(splash_active){splash_screen();return;}
    const wm_theme_t *theme=theme_palette();
    uint16_t bg=theme->bg,fg=theme->text,pa=theme->panel,guide=theme->label;
    if(scope_open)scope_snapshot();
    for(int32_t band=0;band<4;band++){
        app_idle();
        cv_begin(240,60,bg);cv_oy=-band*60;
        if(scope_open)screen_scope(bg,fg,guide);
        else if(notice&&notice_error())screen_message(fg,guide);
        else if(confirm)screen_message(fg,guide);
        else if(notice&&ui_time_before(notice_until))screen_message(fg,guide);
        else if(editor_open)screen_editor(bg,fg,pa,guide);
        else if(settings_open)screen_parameters(fg,pa,guide);
        else if(focus&&ui_time_before(focus_until))screen_focus(fg,guide);
        else screen_home(fg,guide);
        cv_blit(0,(uint32_t)band*60u);
    }
    cv_oy=0;
}
