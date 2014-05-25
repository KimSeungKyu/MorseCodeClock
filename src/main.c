#include <pebble.h>

#define KEY_INVERT 0
  
Window *my_window;
TextLayer *Time_layer, *Morse_layer1, *Morse_layer2, *Morse_layer3, *Morse_layer4;

static const uint32_t morse_time[10][10] = {
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
};
static const char *morse_code[10] = {
  "- - - - -",  //0
  "· - - - -",  //1
  "· · - - -",  //2
  "· · · - -",  //3
  "· · · · -",  //4
  "· · · · ·",  //5
  "- · · · ·",  //6
  "- - · · ·",  //7
  "- - - · ·",  //8
  "- - - - ·",  //9
};

// viberation
static void vibeMorseCode(struct tm* time){
  short hour = time->tm_hour;
  short min = time->tm_min;
  
  uint32_t now_time[40] = {};
  short t = 0;
  for(short i = 0; i < 40; i++){
    if(i < 10){
      now_time[i] = morse_time[hour/10][t++];
    }else if(i < 20){
      now_time[i] = morse_time[hour%10][t++];
    }else if(i < 30){
      now_time[i] = morse_time[min/10][t++];
    }else{
      now_time[i] = morse_time[min%10][t++];
    }
    if(t==10)t=0;
  }
  
  VibePattern pat = {
    .durations = now_time, 
    .num_segments = ARRAY_LENGTH(now_time)
  };
  vibes_enqueue_custom_pattern(pat);
}
static void UpdateTime(struct tm* tick_time){
  static char time_text[] = "000 00 000 00:00";
  strftime(time_text, strlen(time_text)+1, "%b %d %a %T", tick_time);
  text_layer_set_text(Time_layer, time_text);
  
  text_layer_set_text(Morse_layer1, morse_code[tick_time->tm_hour / 10]);
  text_layer_set_text(Morse_layer2, morse_code[tick_time->tm_hour % 10]);
  text_layer_set_text(Morse_layer3, morse_code[tick_time->tm_min / 10]);
  text_layer_set_text(Morse_layer4, morse_code[tick_time->tm_min % 10]);
}
static void handle_tick(struct tm* tick_time, TimeUnits units_changed){  
  
  UpdateTime(tick_time);

  if(tick_time->tm_min == 0){
    vibeMorseCode(tick_time);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "MCC: O'clock!");
  }
}

// Current time from tap or shake
static void handle_tap(AccelAxisType axis, int32_t direction){
  if(axis != ACCEL_AXIS_Z) return;
  
  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);
  vibeMorseCode(current_time);
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "MCC: Shake!");
}

static void in_recv_handler(DictionaryIterator *iterator, void *context){
  //Get Tuple
  Tuple *t = dict_read_first(iterator);
  if(t)
  {
    switch(t->key)
    {
    case KEY_INVERT:
      //It's the KEY_INVERT key
      if(strcmp(t->value->cstring, "black") == 0)
      {
        //Set and save as inverted
        text_layer_set_background_color(Morse_layer1, GColorBlack);
        text_layer_set_background_color(Morse_layer2, GColorBlack);
        text_layer_set_background_color(Morse_layer3, GColorBlack);
        text_layer_set_background_color(Morse_layer4, GColorBlack);
        text_layer_set_background_color(Time_layer, GColorBlack);
        text_layer_set_text_color(Morse_layer1, GColorWhite);
        text_layer_set_text_color(Morse_layer2, GColorWhite);
        text_layer_set_text_color(Morse_layer3, GColorWhite);
        text_layer_set_text_color(Morse_layer4, GColorWhite);
        text_layer_set_text_color(Time_layer, GColorWhite);
        
        persist_write_bool(KEY_INVERT, true);
      }
      else if(strcmp(t->value->cstring, "white") == 0)
      {
        //Set and save as not inverted
        text_layer_set_background_color(Morse_layer1, GColorWhite);
        text_layer_set_background_color(Morse_layer2, GColorWhite);
        text_layer_set_background_color(Morse_layer3, GColorWhite);
        text_layer_set_background_color(Morse_layer4, GColorWhite);
        text_layer_set_background_color(Time_layer, GColorWhite);
        text_layer_set_text_color(Morse_layer1, GColorBlack);
        text_layer_set_text_color(Morse_layer2, GColorBlack);
        text_layer_set_text_color(Morse_layer3, GColorBlack);
        text_layer_set_text_color(Morse_layer4, GColorBlack);
        text_layer_set_text_color(Time_layer, GColorBlack);
        
        persist_write_bool(KEY_INVERT, false);
      }
      break;
    }
  }
}

void handle_init(void) {
  
  // 설정을 바꾼 것을 실시간으로 반영
  app_message_register_inbox_received((AppMessageInboxReceived) in_recv_handler);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
  bool inverted = persist_read_bool(KEY_INVERT);
  
  my_window = window_create();

  Morse_layer1 = text_layer_create(GRect(0, 0, 144, 140));
  text_layer_set_font(Morse_layer1, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(Morse_layer1, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(my_window), text_layer_get_layer(Morse_layer1));
  
  Morse_layer2 = text_layer_create(GRect(0, 30, 144, 140));
  text_layer_set_font(Morse_layer2, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(Morse_layer2, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(my_window), text_layer_get_layer(Morse_layer2));
  
  Morse_layer3 = text_layer_create(GRect(0, 60, 144, 140));
  text_layer_set_font(Morse_layer3, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(Morse_layer3, GTextAlignmentCenter);
  
  layer_add_child(window_get_root_layer(my_window), text_layer_get_layer(Morse_layer3));
  
  Morse_layer4 = text_layer_create(GRect(0, 90, 144, 140));
  text_layer_set_font(Morse_layer4, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(Morse_layer4, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(my_window), text_layer_get_layer(Morse_layer4));

  Time_layer = text_layer_create(GRect(0, 140, 144, 168));
  text_layer_set_font(Time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(Time_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(my_window), text_layer_get_layer(Time_layer));
  
  if(inverted == false){
    text_layer_set_background_color(Morse_layer1, GColorWhite);
    text_layer_set_background_color(Morse_layer2, GColorWhite);
    text_layer_set_background_color(Morse_layer3, GColorWhite);
    text_layer_set_background_color(Morse_layer4, GColorWhite);
    text_layer_set_background_color(Time_layer, GColorWhite);
    text_layer_set_text_color(Morse_layer1, GColorBlack);
    text_layer_set_text_color(Morse_layer2, GColorBlack);
    text_layer_set_text_color(Morse_layer3, GColorBlack);
    text_layer_set_text_color(Morse_layer4, GColorBlack);
    text_layer_set_text_color(Time_layer, GColorBlack);
  }else{
    text_layer_set_background_color(Morse_layer1, GColorBlack);
    text_layer_set_background_color(Morse_layer2, GColorBlack);
    text_layer_set_background_color(Morse_layer3, GColorBlack);
    text_layer_set_background_color(Morse_layer4, GColorBlack);
    text_layer_set_background_color(Time_layer, GColorBlack);
    text_layer_set_text_color(Morse_layer1, GColorWhite);
    text_layer_set_text_color(Morse_layer2, GColorWhite);
    text_layer_set_text_color(Morse_layer3, GColorWhite);
    text_layer_set_text_color(Morse_layer4, GColorWhite);
    text_layer_set_text_color(Time_layer, GColorWhite);
  }
  
  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);
  UpdateTime(current_time);
  
  tick_timer_service_subscribe(MINUTE_UNIT, &handle_tick);
  accel_tap_service_subscribe(&handle_tap);
  
  window_stack_push(my_window, true);
}

void handle_deinit(void) {
  accel_tap_service_unsubscribe();
  tick_timer_service_unsubscribe();
  text_layer_destroy(Time_layer);
  window_destroy(my_window);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
