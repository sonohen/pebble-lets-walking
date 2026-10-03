#include "../src/c/walk.h"
#include <assert.h>
#include <stdio.h>
int main(void) {
  Walk w;
  walk_init(&w, 8000);
  assert(!walk_start(&w, 1, 8000));
  assert(w.status == WALK_READY);
  assert(walk_start(&w, 1, 3001));
  assert(walk_remaining(&w) == 4999 && walk_turn_remaining(&w) == 2500);
  assert(!walk_sample(&w, 1, 5500, 0));
  assert(walk_sample(&w, 1, 5502, 0));
  assert(!walk_sample(&w, 1, 5700, 0));
  walk_toggle(&w);
  assert(!walk_sample(&w, 1, 6200, 0));
  assert(w.walked == 2699);
  walk_toggle(&w);
  assert(!walk_sample(&w, 2, 100, 6300));
  assert(w.walked == 2899);
  walk_sample(&w, 2, 2200, 0);
  assert(w.status == WALK_ACTIVE);
  walk_finish(&w);
  assert(walk_achieved(&w));
  assert(!walk_sample(&w, 2, 3000, 0));
  assert(w.walked == 4999);

  walk_init(&w, 100);
  walk_start(&w, 1, 1);
  walk_toggle(&w);
  walk_sample(&w, 2, 20, 90);
  assert(w.walked == 0 && !w.turned);
  walk_toggle(&w);
  assert(walk_sample(&w, 2, 70, 0));
  walk_finish(&w);
  assert(!walk_achieved(&w));

  walk_init(&w, -1);
  assert(w.target == 8000);
  walk_start(&w, 1, 1000);
  walk_sample(&w, 1, 900, 0);
  walk_sample(&w, 1, 1000, 0);
  assert(w.walked == 0);
  walk_sample(&w, 2, 10, 1005);
  assert(w.walked == 15);
  walk_init(&w, 8000);
  assert(walk_start(&w, 1, 3000));
  assert(walk_remaining(&w) == 5000 && walk_turn_remaining(&w) == 2500);
  assert(walk_sample(&w, 1, 5500, 0));
  assert(walk_remaining(&w) == 2500 && walk_turn_remaining(&w) == 0);
  walk_sample(&w, 1, 8000, 0);
  assert(w.status == WALK_ACTIVE && !walk_achieved(&w));
  walk_finish(&w);
  assert(walk_achieved(&w));
  walk_init(&w, 8000);
  assert(walk_start(&w, 1, 2999));
  assert(walk_turn_remaining(&w) == 2501);
  assert(!walk_sample(&w, 1, 5499, 0));
  assert(walk_sample(&w, 1, 5501, 0));
  walk_init(&w,8000);
  walk_start(&w,1,3000);
  assert(walk_sample(&w,1,5500,0)==WALK_EVENT_TURN);
  assert(walk_sample(&w,1,5500,0)==WALK_EVENT_NONE);
  assert(walk_sample(&w,1,8000,0)==WALK_EVENT_GOAL);
  assert(w.turned && w.goal_notified);
  assert(walk_sample(&w,1,9000,0)==WALK_EVENT_NONE);
  walk_finish(&w);assert(walk_sample(&w,1,10000,0)==WALK_EVENT_NONE);
  walk_init(&w,8000);assert(!w.turned && !w.goal_notified);
  walk_start(&w,1,3000);
  assert(walk_sample(&w,1,9000,0)==(WALK_EVENT_TURN|WALK_EVENT_GOAL));
  walk_init(&w,100);walk_start(&w,1,99);
  assert(walk_sample(&w,1,100,0)==(WALK_EVENT_TURN|WALK_EVENT_GOAL));
  puts("walking state tests passed");
}
