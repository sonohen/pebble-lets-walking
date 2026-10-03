#include <pebble.h>
#include "walk.h"

#define TARGET_KEY 1
static Window *s_window;
static TextLayer *s_text, *s_title, *s_clock, *s_label, *s_detail_label, *s_detail;
static Layer *s_band;
static ActionBarLayer *s_actions;
static GBitmap *s_icons[8];
static Walk s_walk;
static bool s_editing, s_available, s_result_incomplete;
static int32_t s_today, s_edit_target;
static char s_buffer[32], s_detail_buffer[64], s_clock_buffer[8];
/* Compare before touching the text currently owned by a TextLayer. Changed
 * text switches to another persistent buffer, including same-length numbers. */
typedef struct {
  char text[2][64];
  unsigned index;
  bool valid;
} DisplayText;
static DisplayText s_display_text[6];
static bool s_accent_valid;
static const char *s_main_font, *s_detail_font;
static const char *s_notice = "";
static GColor s_accent;
static int s_width;
static int s_action_icons[3] = {-2, -2, -2};
static void clicks(void *context);
enum { ICON_PLAY, ICON_PAUSE, ICON_STOP, ICON_EDIT, ICON_CHECK, ICON_PLUS, ICON_MINUS, ICON_NEW };
static const uint32_t s_icon_ids[] = {RESOURCE_ID_ICON_PLAY, RESOURCE_ID_ICON_PAUSE, RESOURCE_ID_ICON_STOP, RESOURCE_ID_ICON_EDIT, RESOURCE_ID_ICON_CHECK, RESOURCE_ID_ICON_PLUS, RESOURCE_ID_ICON_MINUS, RESOURCE_ID_ICON_NEW};

static int32_t positive(int32_t n) { return n > 0 ? n : 0; }
static void set_changed_text(TextLayer *layer, unsigned slot, const char *candidate) {
  DisplayText *display = &s_display_text[slot];
  if (display->valid && strcmp(display->text[display->index], candidate) == 0) return;
  display->index ^= 1;
  snprintf(display->text[display->index], sizeof(display->text[0]), "%s", candidate);
  display->valid = true;
  text_layer_set_text(layer, display->text[display->index]);
}
static void band_draw(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, s_accent);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}
static void render(void) {
  GColor previous_accent = s_accent;
  const char *title = "READY", *label = "DAILY GOAL", *detail_label = "";
  int up_icon = -1, select_icon = ICON_EDIT, down_icon = -1;
  int32_t number = s_walk.target;
  bool numeric_detail = false;
  s_accent = PBL_IF_COLOR_ELSE(GColorCobaltBlue, GColorBlack);
  s_detail_buffer[0] = 0;
  if (s_editing) {
    title = "SET GOAL"; number = s_edit_target;
    detail_label = "STEPS";
    snprintf(s_detail_buffer, sizeof(s_detail_buffer), "+100 / -100");
    up_icon = ICON_PLUS; select_icon = ICON_CHECK; down_icon = ICON_MINUS;
  } else if (s_walk.status == WALK_FINISHED) {
    label = "WALKED"; number = s_walk.walked; select_icon = ICON_NEW;
    if (s_result_incomplete) {
      title = "UNKNOWN"; detail_label = "NO DATA";
      s_accent = PBL_IF_COLOR_ELSE(GColorDarkCandyAppleRed, GColorBlack);
    } else {
      title = walk_achieved(&s_walk) ? "GOAL MET" : "FINISHED";
      if (walk_achieved(&s_walk)) s_accent = PBL_IF_COLOR_ELSE(GColorDarkGreen, GColorBlack);
      snprintf(s_detail_buffer, sizeof(s_detail_buffer), "LEFT %ld", (long)positive(walk_remaining(&s_walk)));
    }
  } else if (!s_available) {
    title = "NO DATA"; label = "STEPS";
    detail_label = "ENABLE HEALTH";
    snprintf(s_detail_buffer, sizeof(s_detail_buffer), "THEN RETRY");
    s_accent = PBL_IF_COLOR_ELSE(GColorDarkCandyAppleRed, GColorBlack);
    if (s_walk.status == WALK_ACTIVE || s_walk.status == WALK_PAUSED) { select_icon = -1; down_icon = ICON_STOP; }
  } else if (s_walk.status == WALK_READY) {
    if (s_notice[0]) title = "ALREADY MET";
    up_icon = ICON_PLAY;
    detail_label = "";
    snprintf(s_detail_buffer, sizeof(s_detail_buffer), "TODAY %ld\nLEFT %ld", (long)s_today, (long)positive(s_walk.target - s_today));
  } else {
    title = s_walk.status == WALK_PAUSED ? "PAUSED" : "WALKING";
    if (s_walk.status == WALK_PAUSED) s_accent = GColorBlack;
    label = walk_remaining(&s_walk) <= 0 ? "GOAL MET" : "GOAL LEFT";
    number = positive(walk_remaining(&s_walk));
    detail_label = s_walk.turned ? "TURN NOW" : "TURN IN";
    snprintf(s_detail_buffer, sizeof(s_detail_buffer), "%ld", (long)positive(walk_turn_remaining(&s_walk)));
    numeric_detail = true;
    select_icon = s_walk.status == WALK_PAUSED ? ICON_PLAY : ICON_PAUSE; down_icon = ICON_STOP;
  }
  if ((!s_available && !s_editing && s_walk.status != WALK_FINISHED) || (s_walk.status == WALK_FINISHED && s_result_incomplete)) snprintf(s_buffer, sizeof(s_buffer), "--");
  else snprintf(s_buffer, sizeof(s_buffer), "%ld", (long)number);
  time_t now = time(NULL);
  strftime(s_clock_buffer, sizeof(s_clock_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", localtime(&now));
  set_changed_text(s_clock, 0, s_clock_buffer);
  set_changed_text(s_title, 1, title);
  set_changed_text(s_label, 2, label);
  set_changed_text(s_text, 3, s_buffer);
  set_changed_text(s_detail_label, 4, detail_label);
  set_changed_text(s_detail, 5, s_detail_buffer);
  const char *main_font = strlen(s_buffer) >= 6 ? FONT_KEY_GOTHIC_28_BOLD : (s_width >= 200 ? FONT_KEY_BITHAM_42_BOLD : FONT_KEY_BITHAM_30_BLACK);
  const char *detail_font = numeric_detail ? FONT_KEY_GOTHIC_24_BOLD : FONT_KEY_GOTHIC_18_BOLD;
  if (main_font != s_main_font) { text_layer_set_font(s_text, fonts_get_system_font(main_font)); s_main_font = main_font; }
  if (detail_font != s_detail_font) { text_layer_set_font(s_detail, fonts_get_system_font(detail_font)); s_detail_font = detail_font; }
  if (!s_accent_valid || !gcolor_equal(previous_accent, s_accent)) {
    action_bar_layer_set_background_color(s_actions, s_accent);
    layer_mark_dirty(s_band);
    s_accent_valid = true;
  }
  /* Setting even an unchanged icon reapplies Pebble's click configuration,
   * cancelling a held repeating button. Only update actual state changes. */
  const int icons[] = {up_icon, select_icon, down_icon};
  const ButtonId buttons[] = {BUTTON_ID_UP, BUTTON_ID_SELECT, BUTTON_ID_DOWN};
  for (unsigned i = 0; i < 3; ++i) {
    if (icons[i] != s_action_icons[i]) {
      action_bar_layer_set_icon(s_actions, buttons[i], icons[i] < 0 ? NULL : s_icons[icons[i]]);
      s_action_icons[i] = icons[i];
    }
  }
}

static void refresh(void) {
  if (s_walk.status == WALK_FINISHED) { render(); return; }
#if defined(PBL_HEALTH)
  time_t now = time(NULL), today = time_start_of_today();
  s_available = (health_service_metric_accessible(HealthMetricStepCount, today, now) & HealthServiceAccessibilityMaskAvailable) != 0;
  if (s_available) {
    s_today = health_service_sum_today(HealthMetricStepCount);
    int32_t previous = s_walk.last_total;
    if ((s_walk.status == WALK_ACTIVE || s_walk.status == WALK_PAUSED) && s_walk.last_day != (int32_t)today) {
      previous = 0;
      time_t old_start = s_walk.last_day;
      if (old_start > today) s_available = false;
      while (s_available && old_start < today) {
        struct tm old_tm = *localtime(&old_start);
        old_tm.tm_mday++;
        old_tm.tm_isdst = -1;
        time_t old_end = mktime(&old_tm);
        if (!(health_service_metric_accessible(HealthMetricStepCount, old_start, old_end) & HealthServiceAccessibilityMaskAvailable)) {
          s_available = false;
          break;
        }
        previous += health_service_sum(HealthMetricStepCount, old_start, old_end);
        old_start = old_end;
      }
    }
    if (s_available) {
      WalkEvents events = walk_sample(&s_walk, (int32_t)today, s_today, previous);
      static const uint32_t segments[] = {200, 150, 200, 150, 200};
      static const uint32_t both_segments[] = {200, 150, 200, 150, 200, 500, 200, 150, 200, 150, 200};
      if (events == (WALK_EVENT_TURN | WALK_EVENT_GOAL)) {
        /* Two separate groups of three, independent of queueing semantics. */
        vibes_enqueue_custom_pattern((VibePattern){.durations = both_segments, .num_segments = 11});
      } else if (events != WALK_EVENT_NONE) {
        vibes_enqueue_custom_pattern((VibePattern){.durations = segments, .num_segments = 5});
      }
    }
  }
#else
  s_available = false;
#endif
  render();
}
static void up(ClickRecognizerRef recognizer, void *context) {
  (void)recognizer; (void)context;
  if (s_editing) {
    if (s_edit_target < 100000) { s_edit_target += 100; render(); }
    return;
  }
  refresh();
  if (s_available && s_walk.status == WALK_READY) {
    if (!walk_start(&s_walk, (int32_t)time_start_of_today(), s_today)) s_notice = "Goal already reached";
    else s_notice = "";
  }
  render();
}
static void down(ClickRecognizerRef recognizer, void *context) {
  (void)recognizer; (void)context;
  if (s_editing) {
    if (s_edit_target > 100) { s_edit_target -= 100; render(); }
    return;
  }
  refresh();
  if (s_walk.status == WALK_ACTIVE || s_walk.status == WALK_PAUSED) s_result_incomplete = !s_available;
  walk_finish(&s_walk);
  render();
}
static void select_button(ClickRecognizerRef recognizer, void *context) {
  (void)recognizer; (void)context;
  if (!s_editing && s_walk.status != WALK_FINISHED) refresh();
  bool was_editing = s_editing;
  if (s_editing) {
    s_editing = false; s_walk.target = s_edit_target; persist_write_int(TARGET_KEY, s_walk.target); s_notice = "";
  } else if (s_walk.status == WALK_READY) { s_edit_target = s_walk.target; s_editing = true; }
  else if (s_walk.status == WALK_FINISHED) { walk_init(&s_walk, s_walk.target); s_result_incomplete = false; }
  else if (s_available) walk_toggle(&s_walk);
  if (was_editing != s_editing) action_bar_layer_set_click_config_provider(s_actions, clicks);
  if (!s_editing && s_walk.status == WALK_READY) refresh();
  else render();
}
static void back(ClickRecognizerRef recognizer, void *context) {
  (void)recognizer; (void)context;
  if (s_editing) {
    s_editing = false;
    action_bar_layer_set_click_config_provider(s_actions, clicks);
    refresh();
  }
  else if (s_walk.status == WALK_READY || s_walk.status == WALK_FINISHED) window_stack_pop(true);
}
static void clicks(void *context) {
  (void)context;
  if (s_editing) {
    window_single_repeating_click_subscribe(BUTTON_ID_UP, 100, up);
    window_single_repeating_click_subscribe(BUTTON_ID_DOWN, 100, down);
  } else {
    window_single_click_subscribe(BUTTON_ID_UP, up);
    window_single_click_subscribe(BUTTON_ID_DOWN, down);
  }
  window_single_click_subscribe(BUTTON_ID_SELECT, select_button);
  window_single_click_subscribe(BUTTON_ID_BACK, back);
}
static void tick(struct tm *time, TimeUnits units) { (void)time; (void)units; refresh(); }
#if defined(PBL_HEALTH)
static void health(HealthEventType event, void *context) {
  (void)context;
  if (s_walk.status != WALK_FINISHED &&
      (event == HealthEventMovementUpdate || event == HealthEventSignificantUpdate)) refresh();
}
#endif
static TextLayer *make_text(GRect frame, const char *font, GColor foreground) {
  TextLayer *text = text_layer_create(frame);
  text_layer_set_font(text, fonts_get_system_font(font));
  text_layer_set_text_alignment(text, GTextAlignmentCenter);
  text_layer_set_background_color(text, GColorClear);
  text_layer_set_text_color(text, foreground);
  layer_add_child(window_get_root_layer(s_window), text_layer_get_layer(text));
  return text;
}
static void load(Window *window) {
  GRect bounds = layer_get_bounds(window_get_root_layer(window));
  s_width = bounds.size.w;
  memset(s_display_text, 0, sizeof(s_display_text));
  s_accent_valid = false;
  s_main_font = s_detail_font = NULL;
  for (unsigned i = 0; i < 3; ++i) s_action_icons[i] = -2;
  int w = bounds.size.w - ACTION_BAR_WIDTH, h = bounds.size.h;
  int scale = h > 180 ? h - 180 : 0;
  int band_y = PBL_IF_ROUND_ELSE(23, 18), label_y = PBL_IF_ROUND_ELSE(46, 42);
  s_band = layer_create(GRect(PBL_IF_ROUND_ELSE(26, 0), band_y, w - PBL_IF_ROUND_ELSE(38, 0), 23));
  layer_set_update_proc(s_band, band_draw);
  layer_add_child(window_get_root_layer(window), s_band);
  s_clock = make_text(GRect(PBL_IF_ROUND_ELSE(40, 4), 0, w - PBL_IF_ROUND_ELSE(64, 8), 22), FONT_KEY_GOTHIC_18_BOLD, GColorBlack);
  s_title = make_text(GRect(PBL_IF_ROUND_ELSE(26, 0), band_y - 2, w - PBL_IF_ROUND_ELSE(38, 0), 25), FONT_KEY_GOTHIC_18_BOLD, GColorWhite);
  s_label = make_text(GRect(4, label_y + scale / 4, w - 8, 24), FONT_KEY_GOTHIC_18_BOLD, GColorBlack);
  s_text = make_text(GRect(4, label_y + 20 + scale / 4, w - 8, 50 + scale / 4), FONT_KEY_BITHAM_30_BLACK, GColorBlack);
  s_detail_label = make_text(GRect(PBL_IF_ROUND_ELSE(22, 4), 105 + scale * 2 / 3, w - PBL_IF_ROUND_ELSE(32, 8), 24), FONT_KEY_GOTHIC_18_BOLD, GColorBlack);
  s_detail = make_text(GRect(PBL_IF_ROUND_ELSE(22, 4), 124 + scale * 2 / 3, w - PBL_IF_ROUND_ELSE(32, 8), h - 124 - scale * 2 / 3), FONT_KEY_GOTHIC_24_BOLD, GColorBlack);
  s_actions = action_bar_layer_create();
  action_bar_layer_add_to_window(s_actions, window);
  action_bar_layer_set_click_config_provider(s_actions, clicks);
  for (unsigned i = 0; i < 8; ++i) s_icons[i] = gbitmap_create_with_resource(s_icon_ids[i]);
  refresh();
}
static void unload(Window *window) {
  (void)window;
  text_layer_destroy(s_clock); text_layer_destroy(s_title); text_layer_destroy(s_label);
  text_layer_destroy(s_text); text_layer_destroy(s_detail_label); text_layer_destroy(s_detail);
  layer_destroy(s_band); action_bar_layer_destroy(s_actions);
  for (unsigned i = 0; i < 8; ++i) gbitmap_destroy(s_icons[i]);
}
int main(void) {
  walk_init(&s_walk, persist_exists(TARGET_KEY) ? persist_read_int(TARGET_KEY) : 8000);
  s_window = window_create();
  window_set_background_color(s_window, GColorWhite);
  window_set_window_handlers(s_window, (WindowHandlers){.load = load, .unload = unload});
  window_set_click_config_provider(s_window, clicks);
  window_stack_push(s_window, true);
  tick_timer_service_subscribe(MINUTE_UNIT, tick);
#if defined(PBL_HEALTH)
  if (!health_service_events_subscribe(health, NULL)) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Health events unavailable; minute/button refresh remains active");
  }
#endif
  app_event_loop();
  tick_timer_service_unsubscribe();
#if defined(PBL_HEALTH)
  health_service_events_unsubscribe();
#endif
  window_destroy(s_window);
}
