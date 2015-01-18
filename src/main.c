#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer;

static int s_interval = 0;
static int s_step = 1;

static bool is_countdown_started = false; 
static int s_timer = 0;

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

void vibe_end() {
  static const uint32_t const segments[] = { 500, 300, 500, 300, 500 };
    VibePattern pattern = {
      .durations = segments,
      .num_segments = ARRAY_LENGTH(segments),
    };
    vibes_enqueue_custom_pattern(pattern);
}

void stop_timer() {
  is_countdown_started = false;
  tick_timer_service_unsubscribe();
  update_time(s_interval);
}

void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  s_timer -= 1;

  if (s_timer) {
    update_time(s_timer);
  } else {
    vibe_end();
    stop_timer();
  }
}

void start_timer() {
  is_countdown_started = true;
  s_timer = s_interval;
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
}

void select_single_click_handler(ClickRecognizerRef recognizer, void *c) {
  if (is_countdown_started) {
    vibes_short_pulse();
    stop_timer();
  } else {
    if (s_interval != 0) start_timer();
  }
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