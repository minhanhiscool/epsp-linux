#pragma once
#include "imgui.h"

auto get_font_sans() -> ImFont *;

auto init_gui() -> int;
void gui_loop();
void cleanup_gui();
