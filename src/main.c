#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer;

static int s_interval = 0;
static int s_step = 1;

static void update_time(int value) {
  static char buffer[] = "00:00";

  int minutes = value / 60;
  int seconds = value - 60 * minutes;

  snprintf(buffer, sizeof(buffer), "%02d:%02d", minutes, seconds);

  text_layer_set_text(s_time_layer, buffer);
}

int calculate_step(uint8_t x) {
  if (x > 15) return 15;
  if (x > 10) return 10;
  if (x > 5) return 5;

  return 1;
}

int calculate_interval(int interval, int step) {
  if (step == 1) {
    return interval;
  }

  return interval / step * step;
}

void change_time_click_handler(ClickRecognizerRef recognizer, int flag) {
  s_step = calculate_step(click_number_of_clicks_counted(recognizer));
  s_interval = calculate_interval(s_interval, s_step) + flag * s_step;

  if (s_interval > 59*60 + 59) {
    s_interval = 0;
  } else if (s_interval < 0) {
    s_interval = 59*60 + 59;
  }

  update_time(s_interval);
}

void down_single_click_handler(ClickRecognizerRef recognizer, void *c) {
  change_time_click_handler(recognizer, -1);
}

void up_single_click_handler(ClickRecognizerRef recognizer, void *c) {
  change_time_click_handler(recognizer, 1);
}

void select_single_click_handler(ClickRecognizerRef recognizer, void *c) {
  static const uint32_t const segments[] = { 500, 300, 500, 300, 500 };
  VibePattern pat = {
    .durations = segments,
    .num_segments = ARRAY_LENGTH(segments),
  };
  vibes_enqueue_custom_pattern(pat);
}

void config_provider(Window *window) {
  window_single_repeating_click_subscribe(
    BUTTON_ID_DOWN, 
    300, 
    down_single_click_handler
  );

  window_single_repeating_click_subscribe(
    BUTTON_ID_UP, 
    300, 
    up_single_click_handler
  );
  
  window_single_click_subscribe(
    BUTTON_ID_SELECT, 
    select_single_click_handler
  );
}

static void main_window_load(Window *window) {
  s_time_layer = text_layer_create(GRect(0, 46, 144, 50));

  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);

  text_layer_set_font(
    s_time_layer, 
    fonts_get_system_font(FONT_KEY_BITHAM_42_MEDIUM_NUMBERS)
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

  window_set_click_config_provider(
    s_main_window, 
    (ClickConfigProvider) config_provider
  );

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