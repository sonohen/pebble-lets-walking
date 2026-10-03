#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#define GColorClear 5
#define ACTION_BAR_WIDTH 20
#define GCornerNone 0
static bool clock_is_24h_style(void){return true;}
#define PBL_HEALTH 1
#ifdef PBL_ROUND
#define PBL_IF_ROUND_ELSE(a,b) (a)
#else
#define PBL_IF_ROUND_ELSE(a,b) (b)
#endif
#ifdef PBL_COLOR
#define PBL_IF_COLOR_ELSE(a,b) (a)
#else
#define PBL_IF_COLOR_ELSE(a,b) (b)
#endif
#define FONT_KEY_GOTHIC_14_BOLD "font"
#define FONT_KEY_GOTHIC_18_BOLD "GOTHIC_18_BOLD"
#define FONT_KEY_GOTHIC_18 "GOTHIC_18"
#define FONT_KEY_GOTHIC_24_BOLD "GOTHIC_24_BOLD"
#define FONT_KEY_BITHAM_42_BOLD "BITHAM_42_BOLD"
#define FONT_KEY_BITHAM_30_BLACK "BITHAM_30_BLACK"
#define FONT_KEY_GOTHIC_28_BOLD "GOTHIC_28_BOLD"
#define GColorWhite 0
#define GColorBlack 1
#define GColorCobaltBlue 2
#define GColorDarkGreen 3
#define GColorDarkCandyAppleRed 4
#define GCornersAll 0
#define RESOURCE_ID_ICON_PLAY 1
#define RESOURCE_ID_ICON_PAUSE 2
#define RESOURCE_ID_ICON_STOP 3
#define RESOURCE_ID_ICON_EDIT 4
#define RESOURCE_ID_ICON_CHECK 5
#define RESOURCE_ID_ICON_PLUS 6
#define RESOURCE_ID_ICON_MINUS 7
#define RESOURCE_ID_ICON_NEW 8
#define GTextOverflowModeWordWrap 0
#define GTextOverflowModeTrailingEllipsis 1
#define GTextAlignmentCenter 0
#define BUTTON_ID_UP 0
#define BUTTON_ID_DOWN 1
#define BUTTON_ID_SELECT 2
#define BUTTON_ID_BACK 3
#define MINUTE_UNIT 1
#define APP_LOG_LEVEL_WARNING 1
#define APP_LOG(...) ((void)0)
#define HealthMetricStepCount 0
#define HealthServiceAccessibilityMaskAvailable 1
#define HealthEventSignificantUpdate 0
#define HealthEventMovementUpdate 1
#define HealthEventSleepUpdate 2
#define HealthEventHeartRateUpdate 4
#define HealthEventMetricAlert 3
typedef void Window;
typedef int GColor;
static bool gcolor_equal(GColor a,GColor b){return a==b;}
typedef int ButtonId;
typedef const char *GFont;
typedef void *ClickRecognizerRef;
typedef int TimeUnits;
typedef int HealthEventType;
typedef struct {int16_t x,y;} GPoint;
typedef struct {int16_t w,h;} GSize;
typedef struct {GPoint origin;GSize size;} GRect;
#define GRect(x,y,w,h) ((GRect){.origin={(x),(y)},.size={(w),(h)}})
typedef struct {GRect frame;void (*update)(void *,void *);} Layer;
typedef struct {Layer layer;char text[160];GFont font;GColor background,foreground;unsigned text_sets,font_sets;const char *last_text_pointer;} TextLayer;
typedef struct {int id;} GBitmap;
typedef struct {GBitmap *icons[3];GColor background;void(*provider)(void *);} ActionBarLayer;
typedef void GContext;
typedef struct {const uint32_t *durations; uint32_t num_segments;} VibePattern;
typedef struct {void (*load)(Window *); void (*unload)(Window *);} WindowHandlers;
static int32_t fake_today=100000, fake_total=1000, fake_previous=1200;
static bool fake_available=true, fake_history_available=true;
static int fake_vibes, fake_pop;
static unsigned fake_motor_pulses;
static uint32_t fake_segments[16], fake_segment_count;
static int fake_saved_target;
static int fake_width=144,fake_height=168;
static TextLayer fake_text_layers[12];static int fake_text_count;
static Layer fake_layer;
static ActionBarLayer fake_action;
static GBitmap fake_bitmaps[8];
static int fake_repeat[4];
static unsigned fake_click_generation;
static void (*fake_button_callbacks[4])(ClickRecognizerRef,void *);
static unsigned fake_access_calls,fake_today_calls,fake_history_calls,fake_persist_calls,fake_dirty_calls,fake_action_background_calls;
static time_t fake_now=1800000000;
static time_t fake_time(time_t *p){if(p)*p=fake_now;return fake_now;}
#define time fake_time
static void fake_reset_counters(void){
  fake_access_calls=fake_today_calls=fake_history_calls=fake_persist_calls=fake_dirty_calls=fake_action_background_calls=0;
  for(int i=0;i<fake_text_count;i++)fake_text_layers[i].text_sets=fake_text_layers[i].font_sets=0;
}
static unsigned fake_total_text_sets(void){unsigned count=0;for(int i=0;i<fake_text_count;i++)count+=fake_text_layers[i].text_sets;return count;}
static unsigned fake_total_font_sets(void){unsigned count=0;for(int i=0;i<fake_text_count;i++)count+=fake_text_layers[i].font_sets;return count;}
static time_t time_start_of_today(void) {return fake_today;}
static int health_service_metric_accessible(int metric,time_t start,time_t end) {(void)metric;(void)end;fake_access_calls++;return start==fake_today?fake_available:fake_history_available;}
static int32_t health_service_sum_today(int metric){(void)metric;fake_today_calls++;return fake_total;}
static int32_t health_service_sum(int metric,time_t start,time_t end){(void)metric;(void)start;(void)end;fake_history_calls++;return fake_previous;}
static void vibes_enqueue_custom_pattern(VibePattern p){fake_vibes++;fake_segment_count=p.num_segments;for(uint32_t i=0;i<p.num_segments;i++){if(i<16)fake_segments[i]=p.durations[i];if(i%2==0 && p.durations[i]>0)fake_motor_pulses++;}}
static void text_layer_set_text(TextLayer *p,const char *s){if(p){p->text_sets++;p->last_text_pointer=s;snprintf(p->text,sizeof(p->text),"%s",s);}}
static int persist_write_int(int k,int v){(void)k;fake_persist_calls++;fake_saved_target=v;return v;}
static int persist_read_int(int k){(void)k;return fake_saved_target;}
static bool persist_exists(int k){(void)k;return fake_saved_target!=0;}
static void window_stack_pop(bool a){(void)a;fake_pop++;}
static void window_single_repeating_click_subscribe(int b,int interval,void(*f)(ClickRecognizerRef,void*)){fake_button_callbacks[b]=f;fake_repeat[b]=interval;}
static void window_single_click_subscribe(int b,void(*f)(ClickRecognizerRef,void*)){fake_button_callbacks[b]=f;fake_repeat[b]=0;}
static Layer *window_get_root_layer(Window *w){(void)w;return &fake_layer;}
static GRect layer_get_bounds(Layer *w){(void)w;return GRect(0,0,fake_width,fake_height);}
static TextLayer *text_layer_create(GRect r){TextLayer *p=&fake_text_layers[fake_text_count++];p->layer.frame=r;return p;}
static void text_layer_set_font(TextLayer *p,GFont f){p->font=f;p->font_sets++;}
static GFont fonts_get_system_font(const char *f){return f;}
#define text_layer_set_text_alignment(...) ((void)0)
static void text_layer_set_background_color(TextLayer *p,GColor c){p->background=c;}
static void text_layer_set_text_color(TextLayer *p,GColor c){p->foreground=c;}
#define text_layer_set_overflow_mode(...) ((void)0)
static Layer *text_layer_get_layer(TextLayer *p){return &p->layer;}
static void layer_set_frame(Layer *p,GRect r){p->frame=r;}
static Layer *layer_create(GRect r){fake_layer.frame=r;return &fake_layer;}
#define layer_set_update_proc(...) ((void)0)
static void layer_mark_dirty(Layer *p){(void)p;fake_dirty_calls++;}
#define layer_destroy(...) ((void)0)
#define graphics_context_set_fill_color(...) ((void)0)
#define graphics_fill_rect(...) ((void)0)
static ActionBarLayer *action_bar_layer_create(void){return &fake_action;}
#define action_bar_layer_add_to_window(...) ((void)0)
static void action_bar_layer_set_background_color(ActionBarLayer *p,GColor c){p->background=c;fake_action_background_calls++;}
static void action_bar_layer_set_icon(ActionBarLayer *p,int b,GBitmap *i){p->icons[b]=i;if(p->provider){fake_click_generation++;p->provider(NULL);}}
static void action_bar_layer_clear_icon(ActionBarLayer *p,int b){p->icons[b]=NULL;}
static void action_bar_layer_set_click_config_provider(ActionBarLayer *p,void(*f)(void *)){p->provider=f;fake_click_generation++;f(NULL);}
#define action_bar_layer_destroy(...) ((void)0)
static GBitmap *gbitmap_create_with_resource(int i){fake_bitmaps[i-1].id=i;return &fake_bitmaps[i-1];}
#define gbitmap_destroy(...) ((void)0)
#define layer_add_child(...) ((void)0)
#define text_layer_destroy(...) ((void)0)
static Window *window_create(void){return NULL;}
#define window_set_background_color(...) ((void)0)
#define window_set_window_handlers(...) ((void)0)
#define window_set_click_config_provider(...) ((void)0)
#define window_stack_push(...) ((void)0)
#define tick_timer_service_subscribe(...) ((void)0)
#define health_service_events_subscribe(...) (true)
#define app_event_loop(...) ((void)0)
#define tick_timer_service_unsubscribe(...) ((void)0)
#define health_service_events_unsubscribe(...) ((void)0)
#define window_destroy(...) ((void)0)
