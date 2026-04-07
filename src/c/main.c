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
  bool ShowBattery;
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
  settings.DateSize = 3;
  settings.DateThickness = 3;
  settings.BatteryLength = 4;
  settings.BatteryWidth = 5; // 3 * 2 - 1
  settings.ShowDate = true;
  settings.ShowBattery = true;
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


static void window_update_proc(Layer *layer, GContext *ctx) {
  
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  GRect bounds = layer_get_bounds(s_window_layer);
  
  window_set_background_color(s_main_window, settings.BackgroundColor);
  
  int hour = tick_time->tm_hour % 12;
  int minute = tick_time->tm_min;
  int second = tick_time->tm_sec;
  
  int hour_angle = DEG_TO_TRIGANGLE (/*30*hour + 0.5**/6*minute);
  int minute_angle = DEG_TO_TRIGANGLE (6/**minute + 0.5*/*second);
  
  GPoint center = GPoint((bounds.size.w-1)/2, (bounds.size.h-1)/2);
  int max_len = center.x>center.y? center.y : center.x;
  
  // Draw Hour Hand
  graphics_context_set_stroke_color(ctx, settings.HourColor);
  graphics_context_set_stroke_width(ctx, settings.HourWidth);
  int hour_len = (max_len*settings.HourLength/14) - ((settings.HourWidth-1)/2);
  GRect hour_rect = GRect ( center.x - hour_len, center.y - hour_len, 2*hour_len+1, 2*hour_len+1);
  GPoint hour_end = gpoint_from_polar(hour_rect, GOvalScaleModeFitCircle, hour_angle);
  graphics_draw_line(ctx, center, hour_end);
  
  // Draw Minute Hand
  graphics_context_set_stroke_color(ctx, settings.MinuteColor);
  graphics_context_set_stroke_width(ctx, settings.MinuteWidth);
  int minute_len = (max_len*settings.MinuteLength/14) - ((settings.MinuteWidth-1)/2);
  GRect minute_rect = GRect ( center.x - minute_len, center.y - minute_len, 2*minute_len+1, 2*minute_len+1);
  GPoint minute_end = gpoint_from_polar(minute_rect, GOvalScaleModeFitCircle, minute_angle);
  graphics_draw_line(ctx, center, minute_end);
  
  //z Draw the Battery Bar
  graphics_context_set_stroke_color(ctx, settings.BatteryColor);
  graphics_context_set_stroke_width(ctx, settings.BatteryWidth+4);
  int battery_radius = (max_len*2/3);
  GRect battery_rect = GRect ( center.x - battery_radius, center.y - battery_radius, 2*battery_radius+1, 2*battery_radius+1);
  int battery_max_angle = 8 * settings.BatteryLength + 5;
  int battery_angle = battery_max_angle * s_battery_level / 100;
  graphics_draw_arc(ctx, battery_rect, GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE ( 270 - battery_max_angle ), DEG_TO_TRIGANGLE ( 270 + battery_max_angle ));
  graphics_context_set_stroke_color(ctx, settings.BackgroundColor);
  graphics_context_set_stroke_width(ctx, settings.BatteryWidth);
  graphics_draw_arc(ctx, battery_rect, GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE ( 270 - battery_max_angle ), DEG_TO_TRIGANGLE ( 270 + battery_max_angle ));
  graphics_context_set_stroke_color(ctx, settings.BatteryColor);
  graphics_context_set_stroke_width(ctx, settings.BatteryWidth+2);
  graphics_draw_arc(ctx, battery_rect, GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE ( 270 - battery_angle ), DEG_TO_TRIGANGLE ( 270 + battery_angle ));
  

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
  }
  
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
    settings.DateThickness = date_thick_t->value->int32;
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

  Tuple *show_batt_t = dict_find(iterator, MESSAGE_KEY_ShowBattery);
  if (show_batt_t) {
    settings.ShowBattery = show_batt_t->value->int32 == 1;
  }

  // Save and apply if any settings were changed
  if (bg_color_t || hour_color_t || min_color_t || date_color_t || batt_color_t || \
      hour_length_t || hour_width_t || min_length_t || min_width_t || date_size_t || \
      date_thick_t || batt_length_t || batt_width_t || show_date_t || show_batt_t) {
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

  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);

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
