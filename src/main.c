#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer;

static int s_interval = 42;

static void update_time(int value) {
  static char buffer[] = "00:00";

  int minutes = value / 60;
  int seconds = value - 60 * minutes;

  snprintf(buffer, sizeof(buffer), "%02d:%02d", minutes, seconds);

  text_layer_set_text(s_time_layer, buffer);
}

static void main_window_load(Window *window) {
  s_time_layer = text_layer_create(GRect(0, 46, 144, 50));

  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);

  text_layer_set_font(
    s_time_layer, 
    fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD)
  );

  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  layer_add_child(
    window_get_root_layer(window), 
    text_layer_get_layer(s_time_layer)
  );

  update_time(s_interval);
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_time_layer);
}

static void init() {
  s_main_window = window_create();
  
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  window_stack_push(s_main_window, true);
}

static void destroy() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  destroy();
}