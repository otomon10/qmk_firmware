#pragma once

#include "quantum.h"

bool is_ready_trackball(void);
void send_trackball_report(const int mouse_speed);
bool is_trackball_stop(void);
void process_trackball(const int mouse_speed);
void enable_trackball_force_move();
void disable_trackball_force_move();
void disable_trackball_force_move_with_delay();
void trackball_continue_moving();
