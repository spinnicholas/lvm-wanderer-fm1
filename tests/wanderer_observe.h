/* SPDX-License-Identifier: GPL-3.0-only
 * Assertions at the end of the actual application control pass, after LEDs update.
 * WANDERER_EXPECT: lines "milliseconds field integer" (hex allowed).
 */
static void host_verify_app(void)
{
    static FILE *f;static int initialised,have;
    static uint32_t at;static char field[40],value[40];
    if(!initialised){
        const char *path=getenv("WANDERER_EXPECT");initialised=1;
        if(path){f=fopen(path,"r");if(!f){perror(path);exit(4);}}
    }
    if(!f)return;
    for(;;){
        if(!have){
            if(fscanf(f,"%u %39s %39s",&at,field,value)!=3){fclose(f);f=NULL;return;}
            if(at>=end_ms){fprintf(stderr,"expectation beyond script end\n");exit(4);}
            have=1;
        }
        if(fm1_ms<at)return;
        uint64_t got=0,want=strtoull(value,NULL,0);
        if(!strcmp(field,"leds"))got=host_leds;
        else if(!strcmp(field,"glow"))got=host_glow;
        else if(!strcmp(field,"running"))got=running;
        else if(!strcmp(field,"recording"))got=recording;
        else if(!strcmp(field,"settings"))got=settings_open;
        else if(!strcmp(field,"page"))got=settings_page;
        else if(!strcmp(field,"confirm"))got=confirm;
        else if(!strcmp(field,"anchor"))got=wm.has_anchor;
        else if(!strcmp(field,"anchorview"))got=anchor_view;
        else if(!strcmp(field,"layer"))got=wm.layer_enabled;
        else if(!strcmp(field,"slot"))got=project_slot;
        else if(!strcmp(field,"sound"))got=wm.sound;
        else if(!strcmp(field,"volume"))got=vol;
        else if(!strcmp(field,"cues"))got=cues_enabled;
        else if(!strcmp(field,"focus"))got=focus;
        else if(!strcmp(field,"displayhold"))got=display_hold;
        else if(!strcmp(field,"steady"))got=zoom_steady;
        else if(!strcmp(field,"cuelevel"))got=cue_level;
        else if(!strcmp(field,"splash"))got=splash_active;
        else if(!strcmp(field,"bpm"))got=wm.bpm;
        else if(!strcmp(field,"scope"))got=scope_open;
        else if(!strcmp(field,"hold"))got=scope_hold;
        else if(!strcmp(field,"timebase"))got=scope_time;
        else if(!strcmp(field,"gain"))got=scope_gain;
        else if(!strcmp(field,"notice"))got=notice;
        else if(!strcmp(field,"cueleft"))got=cue_left;
        else if(!strcmp(field,"gates")){for(uint32_t k=0;k<NV;k++)got+=voice[k].gate!=0;}
        else if(!strcmp(field,"length"))got=wm.loop_steps;
        else if(!strcmp(field,"scale"))got=wm.scale;
        else if(!strcmp(field,"root"))got=wm.root;
        else if(!strcmp(field,"editor"))got=editor_open;
        else if(!strcmp(field,"editpage"))got=editor_page;
        else if(!strcmp(field,"printarmed"))got=wp_armed;
        else if(!strcmp(field,"printing"))got=wp_writing;
        else if(!strcmp(field,"printed"))got=wp_active;
        else if(!strcmp(field,"generation"))got=wp_generation;
        else if(!strcmp(field,"capture"))got=capture_count()!=0;
        else if(!strcmp(field,"captures"))got=capture_count();
        else if(!strcmp(field,"chord0"))got=wm_step_notes(&wm.memory,0);
        else if(!strcmp(field,"dirty"))got=project_dirty;
        else if(!strcmp(field,"origin"))got=project_origin_slot;
        else if(!strcmp(field,"slotvalid"))got=slot_presence[project_slot];
        else if(!strcmp(field,"savepending"))got=save_pending;
        else if(!strcmp(field,"heldlong"))got=wm.memory.e[0].gate>1000u;
        else if(!strcmp(field,"memory0"))got=wm.memory.e[0].note;
        else {fprintf(stderr,"unknown expectation %s\n",field);exit(4);}
        if(got!=want){fprintf(stderr,"at %u %s: got %llu expected %llu\n",fm1_ms,field,(unsigned long long)got,(unsigned long long)want);exit(4);}
        printf("%6u ms  assertion %s = %llu passed\n",fm1_ms,field,(unsigned long long)got);
        have=0;
    }
}
