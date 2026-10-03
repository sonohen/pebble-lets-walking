#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef enum { WALK_READY, WALK_ACTIVE, WALK_PAUSED, WALK_FINISHED } WalkStatus;
typedef enum {
  WALK_EVENT_NONE = 0,
  WALK_EVENT_TURN = 1 << 0,
  WALK_EVENT_GOAL = 1 << 1
} WalkEvents;
typedef struct {
  WalkStatus status;
  int32_t target, initial_remaining, walked;
  int32_t last_day, last_total;
  bool turned, goal_notified;
} Walk;
void walk_init(Walk *w, int32_t target);
bool walk_start(Walk *w, int32_t day, int32_t total);
/* previous_total is the finalized total for last_day on a day change. */
WalkEvents walk_sample(Walk *w, int32_t day, int32_t total, int32_t previous_total);
void walk_toggle(Walk *w);
void walk_finish(Walk *w);
int32_t walk_remaining(const Walk *w);
int32_t walk_turn_remaining(const Walk *w);
bool walk_achieved(const Walk *w);
