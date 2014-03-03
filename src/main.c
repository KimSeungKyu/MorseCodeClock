#include <pebble.h>

Window *my_window;
TextLayer *Time_layer, *Title_layer;

static const uint32_t morse_time[24][20] = {
    { 300, 100, 300, 100, 300, 100, 300, 100, 300, 300 },  //0
    { 100, 100, 300, 100, 300, 100, 300, 100, 300, 300 },  //1
    { 100, 100, 100, 100, 300, 100, 300, 100, 300, 300 },  //2
    { 100, 100, 100, 100, 100, 100, 300, 100, 300, 300 },  //3
    { 100, 100, 100, 100, 100, 100, 100, 100, 300, 300 },  //4
    { 100, 100, 100, 100, 100, 100, 100, 100, 100, 300 },  //5
    { 300, 100, 100, 100, 100, 100, 100, 100, 100, 300 },  //6
    { 300, 100, 300, 100, 100, 100, 100, 100, 100, 300 },  //7
    { 300, 100, 300, 100, 300, 100, 100, 100, 100, 300 },  //8
    { 300, 100, 300, 100, 300, 100, 300, 100, 100, 300 },  //9
    { 100, 100, 300, 100, 300, 100, 300, 100, 300, 300, 300, 100, 300, 100, 300, 100, 300, 100, 300, 300 },  //10
    { 100, 100, 300, 100, 300, 100, 300, 100, 300, 300, 100, 100, 300, 100, 300, 100, 300, 100, 300, 300 },  //11
    { 100, 100, 300, 100, 300, 100, 300, 100, 300, 300, 100, 100, 100, 100, 300, 100, 300, 100, 300, 300 },  //12
    { 100, 100, 300, 100, 300, 100, 300, 100, 300, 300, 100, 100, 100, 100, 100, 100, 300, 100, 300, 300 },  //13
    { 100, 100, 300, 100, 300, 100, 300, 100, 300, 300, 100, 100, 100, 100, 100, 100, 100, 100, 300, 300 },  //14
    { 100, 100, 300, 100, 300, 100, 300, 100, 300, 300, 100, 100, 100, 100, 100, 100, 100, 100, 100, 300 },  //15
    { 100, 100, 300, 100, 300, 100, 300, 100, 300, 300, 300, 100, 100, 100, 100, 100, 100, 100, 100, 300 },  //16
    { 100, 100, 300, 100, 300, 100, 300, 100, 300, 300, 300, 100, 300, 100, 100, 100, 100, 100, 100, 300 },  //17
    { 100, 100, 300, 100, 300, 100, 300, 100, 300, 300, 300, 100, 300, 100, 300, 100, 100, 100, 100, 300 },  //18
    { 100, 100, 300, 100, 300, 100, 300, 100, 300, 300, 300, 100, 300, 100, 300, 100, 300, 100, 100, 300 },  //19
    { 100, 100, 300, 100, 300, 100, 300, 100, 300, 300 ,300, 100, 300, 100, 300, 100, 300, 100, 300, 300 },  //20
    { 100, 100, 300, 100, 300, 100, 300, 100, 300, 300 ,100, 100, 300, 100, 300, 100, 300, 100, 300, 300 },  //21
    { 100, 100, 300, 100, 300, 100, 300, 100, 300, 300 ,100, 100, 100, 100, 300, 100, 300, 100, 300, 300 },  //22
    { 100, 100, 300, 100, 300, 100, 300, 100, 300, 300 ,100, 100, 100, 100, 100, 100, 300, 100, 300, 300 },  //23
};
// viberation
void vibeMorseCode(struct tm* time){
  int h = time->tm_hour;
  VibePattern pat = {
    .durations = morse_time[h], 
    .num_segments = ARRAY_LENGTH(morse_time[h])
  };
  vibes_enqueue_custom_pattern(pat);
}
static void handle_tick(struct tm* tick_time, TimeUnits units_changed){  
  if(units_changed & HOUR_UNIT){
    static char time_text[] = "00:00";
    strftime(time_text, strlen(time_text)+1, "%T", tick_time);
    text_layer_set_text(Time_layer, time_text);
    vibeMorseCode(tick_time);
    
    APP_LOG(APP_LOG_LEVEL_DEBUG, "hour .... %d", tick_time->tm_hour);
  }else if(units_changed & MINUTE_UNIT){
    static char time_text[] = "00:00";

    strftime(time_text, strlen(time_text)+1, "%T", tick_time);
    text_layer_set_text(Time_layer, time_text);
    
    APP_LOG(APP_LOG_LEVEL_DEBUG, "minute .... %s", time_text);
  }
}

// Current time from tap or shake
static void handle_tap(AccelAxisType axis, int32_t direction){
  if(axis != ACCEL_AXIS_Z) return;
  
  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);
  vibeMorseCode(current_time);
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "now time .... %d:%d", current_time->tm_hour, current_time->tm_min);
}

void handle_init(void) {
  my_window = window_create();
  
  Time_layer = text_layer_create(GRect(0, 168-40, 144, 40));
  text_layer_set_font(Time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_color(Time_layer, GColorWhite);
  text_layer_set_background_color(Time_layer, GColorBlack);
  text_layer_set_text_alignment(Time_layer, GTextAlignmentCenter);
  
  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);
  handle_tick(current_time, MINUTE_UNIT);
  tick_timer_service_subscribe(HOUR_UNIT|MINUTE_UNIT, &handle_tick);
  
  layer_add_child(window_get_root_layer(my_window), text_layer_get_layer(Time_layer));
  
  Title_layer = text_layer_create(GRect(0, 0, 144, 35));
  text_layer_set_font(Title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_color(Title_layer, GColorWhite);
  text_layer_set_background_color(Title_layer, GColorBlack);
  text_layer_set_text_alignment(Title_layer, GTextAlignmentCenter);
  text_layer_set_text(Title_layer, "Morse Code Clock");
  layer_add_child(window_get_root_layer(my_window), text_layer_get_layer(Title_layer));
  
  accel_tap_service_subscribe(&handle_tap);
  
  window_stack_push(my_window, true);
}

void handle_deinit(void) {
  tick_timer_service_unsubscribe();
  accel_tap_service_unsubscribe();
  text_layer_destroy(Time_layer);
  window_destroy(my_window);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
