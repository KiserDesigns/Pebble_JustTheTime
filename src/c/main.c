#include <pebble.h>

// Persistent storage key
#define SETTINGS_KEY 1

// Define our settings struct
typedef struct ClaySettings {
  GColor BackgroundColor;
  GColor HourColor;
  GColor MinuteColor;
  GColor DateColor;
  GColor BatteryColor;
  int HourLength;
  int HourWidth;
  int MinuteLength;
  int MinuteWidth;
  int DateSize;
  int DateThickness;
  int BatteryLength;
  int BatteryWidth;
  bool ShowDate;
  bool DateFront;
  bool ShowBattery;
  bool BatteryFront;
} ClaySettings;

// An instance of the struct
static ClaySettings settings;

// Main Window
static Window *s_main_window;

// Root (Parent) Window Layer
static Layer *s_window_layer;

// Global Values:
static int s_battery_level;
static bool s_battery_charging;
static bool s_bluetooth_connected;

// Set default settings
static void prv_default_settings() {
  settings.BackgroundColor = GColorBlack;
  settings.HourColor = GColorWhite;
  settings.MinuteColor = PBL_IF_COLOR_ELSE(GColorFromHEX(0xFF5555), GColorWhite);
  settings.DateColor = GColorWhite;
  settings.BatteryColor = PBL_IF_COLOR_ELSE(GColorFromHEX(0x55FF55), GColorWhite);
  settings.HourLength = 7;
  settings.HourWidth = 11; // 6 * 2 - 1
  settings.MinuteLength = 13;
  settings.MinuteWidth = 7;  // 4 * 2 - 1
  settings.DateSize = 2;
  settings.DateThickness = 3; // 2 * 2 - 1
  settings.BatteryLength = 4;
  settings.BatteryWidth = 5; // 3 * 2 - 1
  settings.ShowDate = true;
  settings.DateFront = true;
  settings.ShowBattery = true;
  settings.BatteryFront = false;
}

// Save settings to persistent storage
static void prv_save_settings() {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
}

// Read settings from persistent storage
static void prv_load_settings() {
  // Set defaults first
  prv_default_settings();
  // Then override with any saved values
  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}

static void draw_battery(GContext *ctx, GPoint center, int max_len, GRect battery_rect, int battery_max_angle, int battery_angle) {
  graphics_context_set_stroke_color(ctx, settings.BatteryColor);
  graphics_context_set_stroke_width(ctx, settings.BatteryWidth+4);
  graphics_draw_arc(ctx, battery_rect, GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE ( 270 - battery_max_angle ), DEG_TO_TRIGANGLE ( 270 + battery_max_angle ));
  graphics_context_set_stroke_color(ctx, settings.BackgroundColor);
  graphics_context_set_stroke_width(ctx, settings.BatteryWidth);
  graphics_draw_arc(ctx, battery_rect, GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE ( 270 - battery_max_angle ), DEG_TO_TRIGANGLE ( 270 + battery_max_angle ));
  graphics_context_set_stroke_color(ctx, settings.BatteryColor);
  graphics_context_set_stroke_width(ctx, settings.BatteryWidth+2);
  graphics_draw_arc(ctx, battery_rect, GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE ( 270 - battery_angle ), DEG_TO_TRIGANGLE ( 270 + battery_angle ));
}

static int zero[] = {5,
                     -1,-2,
                    1,-2,
                    1,1,
                    0,2,
                    -1,2,
                    -1,-2};
static int one[] = {4,
                    -1,-1,
                    0,-2,
                    0,2,
                    -1,2,
                    1,2};
static int two[] = {6,
                   -1,-2,
                   1,-2,
                   1,0,
                   0,0,
                   -1,1,
                   -1,2,
                   1,2};
static int three[] = {6,
                     -1,-2,
                     1,-2,
                     1,-1,
                     0,0,
                     1,1,
                     1,2,
                     -1,2};
static int four[] = {4,
                    -1,-2,
                    -1,0,
                    1,0,
                    1,-2,
                    1,2};
static int five[] = {6,
                    1,-2,
                    -1,-2,
                    -1,0,
                    1,0,
                    1,1,
                    0,2,
                    -1,2};
static int six[] = {6,
                    1,-2,
                    0,-2,
                    -1,-1,
                    -1,2,
                    1,2,
                    1,0,
                    -1,0};
static int seven[] = {4,
                     -1,-2,
                    1,-2,
                    1,-1,
                    0,0,
                    0,2};
static int eight[] = {8,
                     -1,-2,
                    1,-2,
                    1,-1,
                    -1,1,
                    -1,2,
                    1,2,
                    1,1,
                    -1,-1,
                    -1,-2};
static int nine[] = {6, // number of strokes
                    -1,2, // point 1 of stroke 1
                    0,2, // point 2 of stroke 1, which is point 1 of stroke 2
                    1,1,
                    1,-2,
                    -1,-2,
                    -1,0,
                    1,0}; // last point of the last stroke
static int* numbers[] = {zero, one, two, three, four, five, six, seven, eight, nine};

static void draw_date(GContext *ctx, GPoint center, int max_len, int day_of_month) {
  //DateSize = 3, 1-5
  //DateThickness = 5: 1,3,5,7,9
  
  // Calculate Date Digits
  int date_radius = max_len/2;
  int segment_length = settings.DateThickness / 2 + settings.DateSize;
  int digit_width = 2 * segment_length;
  
  int digit, i;
  GPoint digit_center, start, end;
  
  // draw thin background outline
  graphics_context_set_stroke_color(ctx, settings.BackgroundColor);
  graphics_context_set_stroke_width(ctx, settings.DateThickness+4);
    // left digit background
  digit_center = GPoint(center.x + date_radius - digit_width, center.y);
  digit = day_of_month / 10;
  for (i = 1; i < 2*numbers[digit][0]; i = i + 2) {
    start = GPoint(digit_center.x + numbers[digit][i]*segment_length, digit_center.y + numbers[digit][i+1]*segment_length);
    end = GPoint(digit_center.x + numbers[digit][i+2]*segment_length, digit_center.y + numbers[digit][i+3]*segment_length);
    graphics_draw_line(ctx, start, end);
  }
    // right digit background
  digit_center = GPoint(center.x + date_radius + digit_width, center.y);
  digit = day_of_month % 10;
  for (i = 1; i < 2*numbers[digit][0]; i+=2) {
    start = GPoint(digit_center.x + numbers[digit][i]*segment_length, digit_center.y + numbers[digit][i+1]*segment_length);
    end = GPoint(digit_center.x + numbers[digit][i+2]*segment_length, digit_center.y + numbers[digit][i+3]*segment_length);
    graphics_draw_line(ctx, start, end);
  }
  
  // draw digits
  graphics_context_set_stroke_color(ctx, settings.DateColor);
  graphics_context_set_stroke_width(ctx, settings.DateThickness);
    // first digit
  digit_center = GPoint(center.x + date_radius - digit_width, center.y);
  digit = day_of_month / 10;
  for (i = 1; i < 2*numbers[digit][0]; i = i + 2) {
    start = GPoint(digit_center.x + numbers[digit][i]*segment_length, digit_center.y + numbers[digit][i+1]*segment_length);
    end = GPoint(digit_center.x + numbers[digit][i+2]*segment_length, digit_center.y + numbers[digit][i+3]*segment_length);
    graphics_draw_line(ctx, start, end);
  }
    // second digit
  digit_center = GPoint(center.x + date_radius + digit_width, center.y);
  digit = day_of_month % 10;
  for (i = 1; i < 2*numbers[digit][0]; i+=2) {
    start = GPoint(digit_center.x + numbers[digit][i]*segment_length, digit_center.y + numbers[digit][i+1]*segment_length);
    end = GPoint(digit_center.x + numbers[digit][i+2]*segment_length, digit_center.y + numbers[digit][i+3]*segment_length);
    graphics_draw_line(ctx, start, end);
  }
  
  
}

static void window_update_proc(Layer *layer, GContext *ctx) {
  
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  GRect bounds = layer_get_bounds(s_window_layer);
  
  window_set_background_color(s_main_window, settings.BackgroundColor);
  
  // Get Local Time and Day of Month
  int hour = tick_time->tm_hour % 12;
  int minute = tick_time->tm_min;
  
  int day_of_month = tick_time->tm_mday;
  
  int hour_angle = DEG_TO_TRIGANGLE (30*hour + 0.5*minute);
  int minute_angle = DEG_TO_TRIGANGLE (6*minute);
  
  // Get center of screen and max visible arm length for scaling
  GPoint center = GPoint((bounds.size.w-1)/2, (bounds.size.h-1)/2);
  int max_len = center.x>center.y? center.y : center.x;
  
  // Calculate Hour Hand
  int hour_len = (max_len*settings.HourLength/14) - ((settings.HourWidth-1)/2);
  GRect hour_rect = GRect ( center.x - hour_len, center.y - hour_len, 2*hour_len+1, 2*hour_len+1);
  GPoint hour_end = gpoint_from_polar(hour_rect, GOvalScaleModeFitCircle, hour_angle);
  
  // Calculate Minute Hand
  int minute_len = (max_len*settings.MinuteLength/14) - ((settings.MinuteWidth-1)/2);
  GRect minute_rect = GRect ( center.x - minute_len, center.y - minute_len, 2*minute_len+1, 2*minute_len+1);
  GPoint minute_end = gpoint_from_polar(minute_rect, GOvalScaleModeFitCircle, minute_angle);

  // Calculate Battery Bar
  int battery_radius = (max_len*2/3);
  GRect battery_rect = GRect ( center.x - battery_radius, center.y - battery_radius, 2*battery_radius+1, 2*battery_radius+1);
  int battery_max_angle = 8 * settings.BatteryLength + 5;
  int battery_angle = battery_max_angle * s_battery_level / 100;
  
  // Draw Battery Bar Behind
  if (settings.ShowBattery && !settings.BatteryFront) {
    draw_battery(ctx, center, max_len, battery_rect, battery_max_angle, battery_angle);
  }
  
  // Draw Date Behind
  if (settings.ShowDate && !settings.DateFront) {
    draw_date(ctx, center, max_len, day_of_month);
  }
  
  // Draw Hands
    // Thin Background Border
  graphics_context_set_stroke_color(ctx, settings.BackgroundColor);
  graphics_context_set_stroke_width(ctx, settings.HourWidth+4);
  graphics_draw_line(ctx, center, hour_end);
  graphics_context_set_stroke_width(ctx, settings.MinuteWidth+4);
  graphics_draw_line(ctx, center, minute_end);
    // Actual Hands
  graphics_context_set_stroke_color(ctx, settings.HourColor);
  graphics_context_set_stroke_width(ctx, settings.HourWidth);
  graphics_draw_line(ctx, center, hour_end);
  graphics_context_set_stroke_color(ctx, settings.MinuteColor);
  graphics_context_set_stroke_width(ctx, settings.MinuteWidth);
  graphics_draw_line(ctx, center, minute_end);
  
  
  // Draw Battery Bar in Front
  if (settings.ShowBattery && settings.BatteryFront) {
    draw_battery(ctx, center, max_len, battery_rect, battery_max_angle, battery_angle);
  }
  
  // Draw Date in Front
  if (settings.ShowDate && settings.DateFront) {
    draw_date(ctx, center, max_len, day_of_month);
  }
  

}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(s_window_layer);
}

static void battery_callback(BatteryChargeState state) {
  s_battery_level = state.charge_percent;
  s_battery_charging = state.is_charging;
  
  layer_mark_dirty(s_window_layer);
}

static void bluetooth_callback(bool connected) {
  s_bluetooth_connected = connected;
  
  if (!connected) {
    vibes_double_pulse();
  }
  
  layer_mark_dirty(s_window_layer);
}
             
// AppMessage received handler
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  
  // Check for Clay settings data
  Tuple *bg_color_t = dict_find(iterator, MESSAGE_KEY_BackgroundColor);
  if (bg_color_t) {
    settings.BackgroundColor = GColorFromHEX(bg_color_t->value->int32);
    window_set_background_color(s_main_window, settings.BackgroundColor);
  }
  
  #ifdef PBL_COLOR
  Tuple *hour_color_t = dict_find(iterator, MESSAGE_KEY_HourColor);
  if (hour_color_t) {
    settings.HourColor = GColorFromHEX(hour_color_t->value->int32);
  }
  
  Tuple *min_color_t = dict_find(iterator, MESSAGE_KEY_MinuteColor);
  if (min_color_t) {
    settings.MinuteColor = GColorFromHEX(min_color_t->value->int32);
  }
  
  Tuple *date_color_t = dict_find(iterator, MESSAGE_KEY_DateColor);
  if (date_color_t) {
    settings.DateColor = GColorFromHEX(date_color_t->value->int32);
  }
  
  Tuple *batt_color_t = dict_find(iterator, MESSAGE_KEY_BatteryColor);
  if (batt_color_t) {
    settings.BatteryColor = GColorFromHEX(batt_color_t->value->int32);
  }
  #else
  // For B/W watches, set the elements to the opposite color of the background. Element color selection is disabled for b/w watches in app config.js
  settings.HourColor = gcolor_equal(settings.BackgroundColor,GColorBlack) ? GColorWhite : GColorBlack;
  settings.MinuteColor = settings.HourColor;
  settings.DateColor = settings.HourColor;
  settings.BatteryColor = settings.HourColor;
  #endif

  Tuple *hour_length_t = dict_find(iterator, MESSAGE_KEY_HourLength);
  if (hour_length_t) {
    settings.HourLength = hour_length_t->value->int32;
  }
  
  Tuple *hour_width_t = dict_find(iterator, MESSAGE_KEY_HourWidth);
  if (hour_width_t) {
    settings.HourWidth = hour_width_t->value->int32 * 2 - 1;
  }
  
  Tuple *min_length_t = dict_find(iterator, MESSAGE_KEY_MinuteLength);
  if (min_length_t) {
    settings.MinuteLength = min_length_t->value->int32;
  }
  
  Tuple *min_width_t = dict_find(iterator, MESSAGE_KEY_MinuteWidth);
  if (min_width_t) {
    settings.MinuteWidth = min_width_t->value->int32 * 2 - 1;
  }
  
  Tuple *date_size_t = dict_find(iterator, MESSAGE_KEY_DateSize);
  if (date_size_t) {
    settings.DateSize = date_size_t->value->int32;
  }
  
  Tuple *date_thick_t = dict_find(iterator, MESSAGE_KEY_DateThickness);
  if (date_thick_t) {
    settings.DateThickness = date_thick_t->value->int32 * 2 - 1;
  }
  
  Tuple *batt_length_t = dict_find(iterator, MESSAGE_KEY_BatteryLength);
  if (batt_length_t) {
    settings.BatteryLength = batt_length_t->value->int32;
  }
  
  Tuple *batt_width_t = dict_find(iterator, MESSAGE_KEY_BatteryWidth);
  if (batt_width_t) {
    settings.BatteryWidth = batt_width_t->value->int32 * 2 - 1;
  }
  
  Tuple *show_date_t = dict_find(iterator, MESSAGE_KEY_ShowDate);
  if (show_date_t) {
    settings.ShowDate = show_date_t->value->int32 == 1;
  }
  
  Tuple *date_front_t = dict_find(iterator, MESSAGE_KEY_DateFront);
  if (date_front_t) {
    settings.DateFront = date_front_t->value->int32 == 1;
  }

  Tuple *show_batt_t = dict_find(iterator, MESSAGE_KEY_ShowBattery);
  if (show_batt_t) {
    settings.ShowBattery = show_batt_t->value->int32 == 1;
  }
  
  Tuple *batt_front_t = dict_find(iterator, MESSAGE_KEY_BatteryFront);
  if (batt_front_t) {
    settings.BatteryFront = batt_front_t->value->int32 == 1;
  }

  // Save and apply if any settings were changed
  if ( PBL_IF_COLOR_ELSE(bg_color_t || hour_color_t || min_color_t || date_color_t || batt_color_t, bg_color_t)  || \
      hour_length_t || hour_width_t || min_length_t || min_width_t || date_size_t || date_thick_t || \
      batt_length_t || batt_width_t || show_date_t || date_front_t || show_batt_t || batt_front_t) {
    prv_save_settings();
    layer_mark_dirty(s_window_layer);

  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void main_window_load(Window *window) {
  GRect bounds = layer_get_bounds(window_get_root_layer(window));

  s_window_layer = layer_create(bounds);
  
  layer_set_update_proc(s_window_layer, window_update_proc);
  
  layer_add_child(window_get_root_layer(window), s_window_layer);
}

static void main_window_unload(Window *window) {
  layer_destroy(s_window_layer);
}

static void init() {
  // Load settings before creating UI
  prv_load_settings();

  s_main_window = window_create();
  window_set_background_color(s_main_window, settings.BackgroundColor);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);

  layer_mark_dirty(s_window_layer);

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  battery_state_service_subscribe(battery_callback);
  battery_callback(battery_state_service_peek());

  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = bluetooth_callback
  });

  // Register AppMessage callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  // Open AppMessage
  const int inbox_size = 256;
  const int outbox_size = 256;
  app_message_open(inbox_size, outbox_size);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
