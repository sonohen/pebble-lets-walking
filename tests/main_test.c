#include <assert.h>
#define main pebble_app_main
#include "../src/c/main.c"
#undef main
static void reset(void){walk_init(&s_walk,8000);s_editing=false;s_result_incomplete=false;fake_available=true;fake_history_available=true;fake_today=100000;fake_total=1000;fake_vibes=0;fake_motor_pulses=0;fake_pop=0;refresh();up(NULL,NULL);assert(s_walk.status==WALK_ACTIVE);}
static void test_goal_notifications(void){
  reset();fake_total=4500;health(HealthEventMovementUpdate,NULL);
  assert(fake_vibes==1 && fake_motor_pulses==3 && s_walk.turned);
  refresh();health(HealthEventSignificantUpdate,NULL);assert(fake_vibes==1);
  fake_total=8000;health(HealthEventMovementUpdate,NULL);
  assert(fake_vibes==2 && fake_motor_pulses==6 && s_walk.goal_notified && s_walk.status==WALK_ACTIVE);
  refresh();health(HealthEventSignificantUpdate,NULL);down(NULL,NULL);assert(fake_vibes==2);
  select_button(NULL,NULL);assert(!s_walk.turned && !s_walk.goal_notified);
  reset();fake_total=9000;refresh();assert(fake_vibes==1 && fake_motor_pulses==6 && fake_segment_count==11);
  const uint32_t combined[]={200,150,200,150,200,500,200,150,200,150,200};
  for(unsigned i=0;i<11;i++)assert(fake_segments[i]==combined[i]);
  refresh();down(NULL,NULL);assert(fake_vibes==1);
  reset();select_button(NULL,NULL);fake_total=9000;refresh();
  assert(fake_vibes==0 && !s_walk.turned && !s_walk.goal_notified && s_walk.walked==0);
  select_button(NULL,NULL);fake_total=12500;refresh();assert(fake_vibes==1);
  fake_total=16000;refresh();assert(fake_vibes==2 && fake_motor_pulses==6);
  walk_init(&s_walk,100);fake_total=99;fake_vibes=0;fake_motor_pulses=0;refresh();up(NULL,NULL);
  assert(s_walk.initial_remaining==1 && !s_walk.turned && !s_walk.goal_notified);
  fake_total=100;refresh();assert(fake_vibes==1 && fake_motor_pulses==6 && fake_segment_count==11);
  refresh();assert(fake_vibes==1);
  puts("separate turnaround and goal notification tests passed");
}
/* Reconfiguring SDK click recognizers invalidates an in-progress hold.
 * A repeated callback is delivered only while that registration survives. */
static int hold_button(int button,int repeats){
  unsigned generation=fake_click_generation;
  int delivered=0;
  for(int i=0;i<repeats;i++){
    if(i && (fake_repeat[button]==0 || generation!=fake_click_generation))break;
    fake_button_callbacks[button](NULL,NULL);
    delivered++;
    /* Updates arriving during a held button must preserve its recognizer. */
    refresh();tick(NULL,MINUTE_UNIT);health(0,NULL);
  }
  return delivered;
}
static void test_goal_holds(void){
  walk_init(&s_walk,8000);s_editing=false;s_result_incomplete=false;fake_available=true;fake_total=1000;refresh();
  select_button(NULL,NULL);
  unsigned generation=fake_click_generation;
  assert(fake_repeat[BUTTON_ID_UP]==100 && fake_repeat[BUTTON_ID_DOWN]==100);
  assert(hold_button(BUTTON_ID_UP,20)==20);assert(s_edit_target==10000);
  assert(fake_click_generation==generation);
  render();refresh();tick(NULL,MINUTE_UNIT);health(0,NULL);
  assert(fake_click_generation==generation);
  assert(hold_button(BUTTON_ID_DOWN,20)==20);assert(s_edit_target==8000);
  /* Release: service updates alone cannot change the goal. */
  refresh();tick(NULL,MINUTE_UNIT);health(0,NULL);assert(s_edit_target==8000);
  s_edit_target=99900;hold_button(BUTTON_ID_UP,20);assert(s_edit_target==100000);
  s_edit_target=200;hold_button(BUTTON_ID_DOWN,20);assert(s_edit_target==100);
  back(NULL,NULL);assert(!s_editing && s_walk.target==8000);
  assert(fake_repeat[BUTTON_ID_UP]==0 && fake_repeat[BUTTON_ID_DOWN]==0);
  select_button(NULL,NULL);hold_button(BUTTON_ID_UP,20);select_button(NULL,NULL);
  assert(s_walk.target==10000 && fake_saved_target==10000);
  up(NULL,NULL);assert(s_walk.status==WALK_ACTIVE);
  generation=fake_click_generation;
  assert(hold_button(BUTTON_ID_DOWN,20)==1);assert(s_walk.status==WALK_FINISHED);
  assert(fake_repeat[BUTTON_ID_DOWN]==0);
  (void)generation;
}
static void test_power_efficiency(void){
  walk_init(&s_walk,8000);s_editing=false;s_result_incomplete=false;s_notice="";
  fake_available=true;fake_today=100000;fake_total=1000;refresh();
  fake_reset_counters();render();refresh();
  assert(fake_total_text_sets()==0 && fake_total_font_sets()==0 && fake_dirty_calls==0 && fake_action_background_calls==0);
  fake_reset_counters();fake_now+=60;tick(NULL,MINUTE_UNIT);
  assert(s_clock->text_sets==1 && fake_total_text_sets()==1 && fake_dirty_calls==0);
  fake_reset_counters();health(HealthEventSleepUpdate,NULL);health(HealthEventHeartRateUpdate,NULL);health(HealthEventMetricAlert,NULL);
  assert(fake_access_calls==0 && fake_today_calls==0 && fake_history_calls==0 && fake_total_text_sets()==0);
  fake_reset_counters();health(HealthEventMovementUpdate,NULL);health(HealthEventSignificantUpdate,NULL);
  assert(fake_access_calls==2 && fake_today_calls==2 && fake_total_text_sets()==0);
  /* The minute fallback still recovers unavailable data without a Health event. */
  fake_available=false;tick(NULL,MINUTE_UNIT);assert(!s_available);
  fake_available=true;fake_reset_counters();tick(NULL,MINUTE_UNIT);
  assert(s_available && fake_today_calls==1 && !strcmp(s_title->text,"READY"));
  select_button(NULL,NULL);fake_reset_counters();
  for(int i=0;i<20;i++)up(NULL,NULL);
  assert(s_edit_target==10000 && fake_access_calls==0 && fake_today_calls==0 && fake_history_calls==0);
  assert(s_text->text_sets==20 && fake_total_text_sets()==20 && fake_dirty_calls==0 && fake_persist_calls==0);
  fake_reset_counters();for(int i=0;i<20;i++)down(NULL,NULL);
  assert(s_edit_target==8000 && fake_access_calls==0 && fake_today_calls==0 && fake_total_text_sets()==20);
  s_edit_target=100000;render();fake_reset_counters();up(NULL,NULL);
  assert(fake_total_text_sets()==0 && fake_dirty_calls==0 && fake_access_calls==0);
  s_edit_target=100;render();fake_reset_counters();down(NULL,NULL);
  assert(fake_total_text_sets()==0 && fake_dirty_calls==0 && fake_access_calls==0);
  s_edit_target=8100;fake_reset_counters();select_button(NULL,NULL);
  assert(fake_persist_calls==1 && fake_saved_target==8100);
  select_button(NULL,NULL);up(NULL,NULL);fake_reset_counters();back(NULL,NULL);assert(fake_persist_calls==0 && s_walk.target==8100);
  up(NULL,NULL);
  const char *previous_main_pointer=s_text->last_text_pointer,*previous_detail_pointer=s_detail->last_text_pointer;
  fake_total+=1;fake_reset_counters();health(HealthEventMovementUpdate,NULL);
  assert(!strcmp(s_text->text,"7099") && !strcmp(s_detail->text,"3549"));
  assert(s_text->text_sets==1 && s_detail->text_sets==1 && fake_total_text_sets()==2 && fake_total_font_sets()==0 && fake_dirty_calls==0);
  assert(previous_main_pointer!=s_text->last_text_pointer && previous_detail_pointer!=s_detail->last_text_pointer);
  assert(!strcmp(previous_main_pointer,"7100") && !strcmp(previous_detail_pointer,"3550"));
  fake_reset_counters();select_button(NULL,NULL);assert(fake_today_calls==1 && s_walk.status==WALK_PAUSED);
  fake_reset_counters();select_button(NULL,NULL);assert(fake_today_calls==1 && s_walk.status==WALK_ACTIVE);
  fake_reset_counters();down(NULL,NULL);assert(fake_today_calls==1 && s_walk.status==WALK_FINISHED);
  fake_reset_counters();refresh();health(HealthEventMovementUpdate,NULL);health(HealthEventSignificantUpdate,NULL);fake_now+=60;tick(NULL,MINUTE_UNIT);
  assert(fake_access_calls==0 && fake_today_calls==0 && fake_history_calls==0 && s_clock->text_sets==1 && fake_total_text_sets()==1);
  fake_total=3333;fake_reset_counters();select_button(NULL,NULL);
  assert(s_walk.status==WALK_READY && s_today==3333 && fake_today_calls==1);
  puts("power operation-count tests passed");
}
int main(void){
#ifdef TEST_WIDTH
  fake_width=TEST_WIDTH;fake_height=TEST_HEIGHT;
#else
#ifdef PBL_ROUND
  fake_width=180;fake_height=180;
#endif
#endif
  load(NULL);
  for(int i=0;i<fake_text_count;i++){
    GRect r=fake_text_layers[i].layer.frame;
    assert(r.origin.x>=0 && r.origin.y>=0 && r.origin.x+r.size.w<=fake_width-ACTION_BAR_WIDTH && r.origin.y+r.size.h<=fake_height);
  }
  clicks(NULL);
  assert(fake_repeat[BUTTON_ID_DOWN]==0);
  test_goal_holds();fake_saved_target=0;
  walk_init(&s_walk,8000);s_editing=false;refresh();
  assert(!strcmp(s_title->text,"READY") && !strcmp(s_buffer,"8000"));
  assert(fake_action.icons[BUTTON_ID_UP]->id==RESOURCE_ID_ICON_PLAY);
  assert(fake_action.icons[BUTTON_ID_DOWN]==NULL);
  select_button(NULL,NULL);up(NULL,NULL);assert(s_edit_target==8100 && s_walk.target==8000);
  assert(fake_repeat[BUTTON_ID_DOWN]==100);
  back(NULL,NULL);assert(!s_editing && s_walk.target==8000 && fake_saved_target==0);
  assert(fake_repeat[BUTTON_ID_DOWN]==0);
  select_button(NULL,NULL);s_edit_target=100;down(NULL,NULL);assert(s_edit_target==100);
  s_edit_target=100000;render();up(NULL,NULL);assert(s_edit_target==100000);
  assert(!strcmp(s_text->font,FONT_KEY_GOTHIC_28_BOLD));
  select_button(NULL,NULL);assert(s_walk.target==100000 && fake_saved_target==100000);
  assert(persist_read_int(TARGET_KEY)==100000);
  fake_available=false;refresh();assert(!strcmp(s_title->text,"NO DATA") && !strcmp(s_buffer,"--"));
  up(NULL,NULL);assert(s_walk.status==WALK_READY);
  select_button(NULL,NULL);assert(s_editing && !strcmp(s_title->text,"SET GOAL"));back(NULL,NULL);
  reset();
  assert(!strcmp(s_title->text,"WALKING") && !strcmp(s_label->text,"GOAL LEFT"));
#ifdef PBL_COLOR
  assert(s_accent==GColorCobaltBlue);
#else
  assert(s_accent==GColorBlack);
#endif
  assert(s_text->foreground==GColorBlack);
  fake_available=false;
  select_button(NULL,NULL);
  assert(s_walk.status==WALK_ACTIVE);
  fake_available=true;fake_total=1200;select_button(NULL,NULL);
  assert(s_walk.status==WALK_PAUSED && s_walk.walked==200);
  assert(!strcmp(s_title->text,"PAUSED") && s_accent==GColorBlack);
  assert(fake_action.icons[BUTTON_ID_SELECT]->id==RESOURCE_ID_ICON_PLAY);
  fake_available=false;fake_total=1500;select_button(NULL,NULL);
  assert(s_walk.status==WALK_PAUSED);
  fake_available=true;select_button(NULL,NULL);
  assert(s_walk.status==WALK_ACTIVE && s_walk.walked==200);
  fake_total=1600;refresh();assert(s_walk.walked==300);
  fake_available=false;down(NULL,NULL);
  assert(s_walk.status==WALK_FINISHED);
  assert(s_result_incomplete);
  assert(!strcmp(s_title->text,"UNKNOWN") && !strcmp(s_detail_label->text,"NO DATA"));
  assert(!strcmp(s_buffer,"--"));
#ifdef PBL_COLOR
  assert(s_accent==GColorDarkCandyAppleRed);
#endif
  back(NULL,NULL);assert(fake_pop==1);
  reset();fake_total=4501;refresh();assert(fake_vibes==1);refresh();assert(fake_vibes==1);
  assert(!strcmp(s_detail_label->text,"TURN NOW") && !strcmp(s_detail->text,"0"));
  assert(fake_segment_count==5);
  assert(fake_segments[0]>0 && fake_segments[1]>0 && fake_segments[2]>0 && fake_segments[3]>0 && fake_segments[4]>0);
  reset();back(NULL,NULL);assert(fake_pop==0 && s_walk.status==WALK_ACTIVE);
  walk_toggle(&s_walk);back(NULL,NULL);assert(fake_pop==0 && s_walk.status==WALK_PAUSED);
  reset();
  /* Advance to the next local midnight; the production adapter calculates it. */
  time_t old=fake_today;struct tm date=*localtime(&old);date.tm_mday++;date.tm_isdst=-1;fake_today=(int32_t)mktime(&date);
  fake_total=100;fake_previous=1200;fake_history_available=false;refresh();assert(s_walk.walked==0);
  fake_history_available=true;refresh();assert(s_walk.walked==300);
  reset();fake_today-=86400;refresh();assert(!s_available && s_walk.walked==0);
  down(NULL,NULL);assert(s_walk.status==WALK_FINISHED && s_result_incomplete);
  reset();old=fake_today;date=*localtime(&old);date.tm_mday+=3;date.tm_isdst=-1;fake_today=(int32_t)mktime(&date);fake_previous=1200;fake_total=100;refresh();assert(s_walk.walked==2700);
  reset();fake_total=8000;refresh();assert(s_walk.status==WALK_ACTIVE && !strcmp(s_label->text,"GOAL MET"));
  down(NULL,NULL);assert(!strcmp(s_title->text,"GOAL MET"));fake_available=false;refresh();assert(!strcmp(s_title->text,"GOAL MET"));
#ifdef PBL_COLOR
  assert(s_accent==GColorDarkGreen);
#endif
  select_button(NULL,NULL);assert(s_walk.status==WALK_READY);
  fake_available=true;fake_total=8000;up(NULL,NULL);assert(s_walk.status==WALK_READY && !strcmp(s_title->text,"ALREADY MET"));
  test_power_efficiency();
  test_goal_notifications();
  puts("main boundary tests passed");
}
