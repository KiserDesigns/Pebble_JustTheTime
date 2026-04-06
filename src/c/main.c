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

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_battery_layer;
static int s_battery_level;

// Unobstructed area
static Layer *s_window_layer;

// Hour Hand Layer
//static Layer *s_canvas_layer;
//GRect bounds = layer_get_bounds(s_window_layer);
//s_canvas_layer = layer_create(bounds);
//static void canvas_update_proc(Layer *layer, GContext *ctx) {
  // Custom drawing happens here!
//}
// Assign the custom drawing procedure
//layer_set_update_proc(s_canvas_layer, canvas_update_proc);
// Add to Window
//layer_add_child(s_window_layer, s_canvas_layer);



// Set default settings
static void prv_default_settings() {
  settings.BackgroundColor = GColorBlack;
  settings.HourColor = GColorWhite;
  settings.MinuteColor = GColorWhite;
  settings.DateColor = GColorWhite;
  settings.BatteryColor = GColorWhite;
  settings.HourLength = 7;
  settings.HourWidth = 11;
  settings.MinuteLength = 13;
  settings.MinuteWidth = 7;
  settings.DateSize = 3;
  settings.DateThickness = 3;
  settings.BatteryLength = 8;
  settings.BatteryWidth = 3;
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

// Apply settings to UI elements
static void prv_update_display() {
  // Set background color
  window_set_background_color(s_main_window, settings.BackgroundColor);

  // Set text colors

  // Show/hide date based on setting
  //layer_set_hidden(text_layer_get_layer(s_date_layer), !settings.ShowDate);

  // Mark battery layer for redraw (color may have changed)
  layer_mark_dirty(s_window_layer);
}

static void update_time() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();

}

static void battery_callback(BatteryChargeState state) {
  s_battery_level = state.charge_percent;
  //layer_mark_dirty(s_battery_layer);
}

static void battery_update_proc(Layer *layer, GContext *ctx) {
//  GRect bounds = layer_get_bounds(layer);

  // Find the width of the bar (inside the border)
//  int bar_width = ((s_battery_level * (bounds.size.w - 4)) / 100);

  // Draw the border using the text color
//  graphics_context_set_stroke_color(ctx, settings.BatteryColor);
//  graphics_draw_round_rect(ctx, bounds, 2);

  // Choose color based on battery level
//  GColor bar_color;
//  if (s_battery_level <= 20) {
//    bar_color = PBL_IF_COLOR_ELSE(GColorRed, settings.BatteryColor);
//  } else if (s_battery_level <= 40) {
//    bar_color = settings.BatteryColor;
//  } else {
//    bar_color = settings.BatteryColor;
//  }

  // Draw the filled bar inside the border
//  graphics_context_set_fill_color(ctx, bar_color);
//  graphics_fill_rect(ctx, GRect(2, 2, bar_width, bounds.size.h - 4), 1, GCornerNone);
}

static void bluetooth_callback(bool connected) {
  // Show icon if disconnected
  // layer_set_hidden(bitmap_layer_get_layer(s_bt_icon_layer), connected);

  if (!connected) {
    vibes_double_pulse();
  }
}
             
// AppMessage received handler
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {

  int settings_changed = false;
  
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
    settings.HourWidth = hour_width_t->value->int32;
  }
  
  Tuple *min_length_t = dict_find(iterator, MESSAGE_KEY_MinuteLength);
  if (min_length_t) {
    settings.MinuteLength = min_length_t->value->int32;
  }
  
  Tuple *min_width_t = dict_find(iterator, MESSAGE_KEY_MinuteWidth);
  if (min_width_t) {
    settings.MinuteWidth = min_width_t->value->int32;
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
    settings.BatteryWidth = batt_width_t->value->int32;
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
    prv_update_display();

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
  s_window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(s_window_layer);


  // Center canvas
  int date_height = 30;
  int block_height = 56 + date_height;
  int time_y = (bounds.size.h / 2) - (block_height / 2) - 10;
  int date_y = time_y + 56;

  // Create the time TextLayer
  s_time_layer = text_layer_create(
      GRect(0, time_y, bounds.size.w, 60));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, settings.HourColor);
//  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);


  // Add layers to the Window
  layer_add_child(s_window_layer, text_layer_get_layer(s_time_layer));

  // Apply saved settings
  prv_update_display();

}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_time_layer);
//  text_layer_destroy(s_weather_layer);
//  fonts_unload_custom_font(s_time_font);
//  fonts_unload_custom_font(s_date_font);
//  layer_destroy(s_battery_layer);
//  gbitmap_destroy(s_bt_icon_bitmap);
//  bitmap_layer_destroy(s_bt_icon_layer);
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

  update_time();

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
