#include "walk.h"
#include <limits.h>
#include <string.h>

void walk_init(Walk *w, int32_t target) {
  memset(w, 0, sizeof(*w));
  w->target = target >= 100 && target <= 100000 && target % 100 == 0 ? target : 8000;
}
bool walk_start(Walk *w, int32_t day, int32_t total) {
  if (w->status != WALK_READY || total < 0 || total >= w->target) return false;
  w->initial_remaining = w->target - total;
  w->last_day = day;
  w->last_total = total;
  w->status = WALK_ACTIVE;
  return true;
}
WalkEvents walk_sample(Walk *w, int32_t day, int32_t total, int32_t previous_total) {
  if ((w->status != WALK_ACTIVE && w->status != WALK_PAUSED) || total < 0) return WALK_EVENT_NONE;
  WalkEvents events = WALK_EVENT_NONE;
  int64_t delta = 0;
  if (day == w->last_day) {
    /* A downward revision must not count recovery as fresh steps. */
    if (total > w->last_total) delta = total - w->last_total;
  } else {
    if (previous_total > w->last_total) delta = previous_total - w->last_total;
    delta += total;
  }
  if (day != w->last_day || total > w->last_total) w->last_total = total;
  w->last_day = day;
  if (w->status == WALK_ACTIVE) {
    int64_t sum = (int64_t)w->walked + delta;
    w->walked = sum > INT32_MAX ? INT32_MAX : (int32_t)sum;
    if (!w->turned && walk_turn_remaining(w) <= 0) {
      w->turned = true;
      events |= WALK_EVENT_TURN;
    }
    if (!w->goal_notified && walk_remaining(w) <= 0) {
      w->goal_notified = true;
      events |= WALK_EVENT_GOAL;
    }
  }
  return events;
}
void walk_toggle(Walk *w) {
  if (w->status == WALK_ACTIVE) w->status = WALK_PAUSED;
  else if (w->status == WALK_PAUSED) w->status = WALK_ACTIVE;
}
void walk_finish(Walk *w) {
  if (w->status == WALK_ACTIVE || w->status == WALK_PAUSED) w->status = WALK_FINISHED;
}
int32_t walk_remaining(const Walk *w) { return w->initial_remaining - w->walked; }
int32_t walk_turn_remaining(const Walk *w) { return (w->initial_remaining + 1) / 2 - w->walked; }
bool walk_achieved(const Walk *w) { return w->status == WALK_FINISHED && walk_remaining(w) <= 0; }
