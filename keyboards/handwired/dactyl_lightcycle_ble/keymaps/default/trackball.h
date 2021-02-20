#pragma once

#include "quantum.h"

bool is_ready_trackball(void);
bool is_trackball_stop(void);
void process_trackball(const double mouse_speed);
void enable_trackball_force_move();
void disable_trackball_force_move();
void disable_trackball_force_move_with_delay();
void trackball_continue_moving();
void init_ttp223();
void enable_touch_detect();
void disable_touch_detect();
void toggle_touch_detect();
