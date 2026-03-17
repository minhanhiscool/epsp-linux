#pragma once

auto init_gui() -> int;
void gui_loop(std::atomic<bool> *thread_closed);
void cleanup_gui();
