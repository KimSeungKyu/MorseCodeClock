#include <pebble.h>

Window *my_window;
TextLayer *Time_layer, *Title_layer;

static const uint32_t const morse_time[24][20] = {
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

/*
from : http://forums.getpebble.com/discussion/8368
*/
static int myAtol(const char *s) {
    const char *p = s, *q;
    long n = 0;
    int sign = 1, k = 1;
  
    if (p != NULL) {
        if (*p != '\0') {
            if ((*p == '+') || (*p == '-')) {
                if (*p++ == '-') sign = -1;
            }
            for (q = p; (*p != '\0'); p++);
            for (--p; p >= q; --p, k *= 10) n += (*p - '0') * k;
        }
    }
    return n * sign;
}

static void handle_minute_tick(struct tm* tick_time, TimeUnits units_changed){
  
   static char time_text[] = "00:00";

  strftime(time_text, sizeof(time_text), "%T", tick_time);
  text_layer_set_text(Time_layer, time_text);
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "minute .... %s", time_text);
}

// Viberate per hour
static void handle_hour_tick(struct tm* tick_time, TimeUnits units_changed){
  
  static char time_text[] = "00:00";
  strftime(time_text, sizeof(time_text), "%T", tick_time);
  text_layer_set_text(Time_layer, time_text);
  
  static char morse_number[] = "00"; 
  strftime(morse_number, sizeof(morse_number), "%T", tick_time);
  
  int h = myAtol(morse_number);
  VibePattern pat = {
    .durations = morse_time[h], 
    .num_segments = ARRAY_LENGTH(morse_time[h])
  };
  vibes_enqueue_custom_pattern(pat);
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "hour .... %d", h);
}

static void handle_tick(struct tm* tick_time, TimeUnits units_changed){  
  if(units_changed & HOUR_UNIT){
    handle_hour_tick(tick_time, HOUR_UNIT);
  }else if(units_changed & MINUTE_UNIT){
    handle_minute_tick(tick_time, MINUTE_UNIT);
  }
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
    
    window_stack_push(my_window, true);
}

void handle_deinit(void) {
	  text_layer_destroy(Time_layer);
	  window_destroy(my_window);
}

int main(void) {
	  handle_init();
	  app_event_loop();
	  handle_deinit();
}
