// TAKE II

#include <pebble.h>

#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1

static Window      *s_main_window;        // main window
static TextLayer   *s_temperature_layer;  // text layer displaying temperature
static TextLayer   *s_hour_layer;         // text layer displaying hours
static TextLayer   *s_min_layer;          // text layer displaying minutes
static TextLayer   *s_colon_layer;        // text layer displaying colon
static TextLayer   *s_date_layer;         // text layer
static TextLayer   *s_battery_layer;      // text layer displaying battery
static GFont        s_time_font;          // time display font
static GFont        s_temperature_font;   // temperature display font
static GFont        s_battery_font;       // font used to display battery %
static GFont        s_date_font;          // date display font
static GFont        s_colon_font;         // time colon display font
static GBitmap     *s_background_image;   // background image
static BitmapLayer *s_background_layer;   // background image container
static GBitmap     *s_battery_image;       // battery image
static BitmapLayer *s_battery_img_layer;  // battery image container

//static char temperature_buffer[8];        // text buffer to hold temperature

// static bool s_configTempF = false;
// static bool s_configShowBattery = true;
static bool s_configForce24H = true;

// define all the fonts used
static void load_fonts() {
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_NHL_VAN_64));
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_NHL_VAN_28));
  s_temperature_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_NHL_VAN_22));
  s_battery_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_NHL_VAN_22));
  s_colon_font = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);
}

// build up the pieces that make up background layer
static void create_background_layer() {
  s_background_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
  s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(s_background_layer, s_background_image);

}

// build up the pieces that display the weather information
static void create_weather_layer() {
  s_temperature_layer = text_layer_create(GRect(18, 126, 108, 50));
  text_layer_set_background_color(s_temperature_layer, GColorClear);
  text_layer_set_text_color(s_temperature_layer, GColorWhite);
  text_layer_set_text_alignment(s_temperature_layer, GTextAlignmentRight);
  text_layer_set_font(s_temperature_layer, s_temperature_font);
  text_layer_set_text(s_temperature_layer, "    ");
}

static void create_battery_layer() {
  s_battery_layer = text_layer_create(GRect(22, 126, 50, 50)); //GRect: x,y,w,h
  text_layer_set_background_color(s_battery_layer, GColorClear);
  text_layer_set_text_color(s_battery_layer, GColorWhite);
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentLeft);
  text_layer_set_font(s_battery_layer, s_temperature_font);
  text_layer_set_text(s_battery_layer, "    ");

  s_battery_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY);
  s_battery_img_layer = bitmap_layer_create(GRect(22, 128, 24, 24));
  bitmap_layer_set_background_color(s_battery_img_layer, GColorClear);
  bitmap_layer_set_bitmap(s_battery_img_layer, s_battery_image);
}

// build up the pieces that display the time information
static void create_time_layer() {
  s_hour_layer = text_layer_create(GRect(0, 10, 68, 70)); //GRect: x,y,w,h
  s_min_layer = text_layer_create(GRect(76, 10, 68, 70));
  text_layer_set_background_color(s_hour_layer, GColorClear);
  text_layer_set_background_color(s_min_layer, GColorClear);
  text_layer_set_text_color(s_hour_layer, GColorWhite);
  text_layer_set_text_color(s_min_layer, GColorWhite);
  text_layer_set_text(s_hour_layer, "23");
  text_layer_set_text(s_min_layer, "59");
  text_layer_set_font(s_hour_layer, s_time_font);
  text_layer_set_font(s_min_layer, s_time_font);
  text_layer_set_text_alignment(s_hour_layer, GTextAlignmentRight);
  text_layer_set_text_alignment(s_min_layer, GTextAlignmentLeft);

  s_colon_layer = text_layer_create(GRect(68, 30, 6, 70));
  text_layer_set_background_color(s_colon_layer, GColorClear);
  text_layer_set_text_color(s_colon_layer, GColorWhite);
  text_layer_set_font(s_colon_layer, s_colon_font);
  text_layer_set_text(s_colon_layer, ":");
  text_layer_set_text_alignment(s_colon_layer, GTextAlignmentCenter);

}

// build up the pieces that display the date information
static void create_date_layer() {
  s_date_layer = text_layer_create(GRect(0, 88, 144, 50));
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_text(s_date_layer, "... ... ..");
  text_layer_set_font(s_date_layer, s_date_font);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
}

// update the date text
static void update_date() {
  APP_LOG(APP_LOG_LEVEL_INFO, "update_date()");
  // build and populate the time structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char date_buffer[] = "WED JUN 30"; //using this as a placeholder example date

  //update the date's string buffer with formatted date
  strftime(date_buffer, sizeof("WED JUN 30"), "%a %b %d", tick_time);
  text_layer_set_text(s_date_layer, date_buffer);
}


static void update_time() {
  APP_LOG(APP_LOG_LEVEL_INFO, "update_time()");
  // build and populate the time structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Create 2 long-lived buffers for the hour and min
  static char hour_buffer[] = "23";
  static char min_buffer[] = "59";

  //update the hour and min buffers with formatted values from the time strucure
  if (!clock_is_24h_style() && !s_configForce24H) {
    strftime(hour_buffer, sizeof("00"), "%I", tick_time);
  } else {
    strftime(hour_buffer, sizeof("00"), "%H", tick_time);
  }
  strftime(min_buffer, sizeof("00"), "%M", tick_time);

  //update the display's hour and min layers with the formatted values
  text_layer_set_text(s_hour_layer, hour_buffer);
  text_layer_set_text(s_min_layer, min_buffer);
}


// Load up the main window
static void main_window_load(Window *window) {
  load_fonts();
  create_time_layer();
  create_background_layer();
  create_weather_layer();
  create_date_layer();
  create_battery_layer();

  // Add each layer as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_hour_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_min_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_colon_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_temperature_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_battery_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_battery_layer));
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_battery_img_layer));
}

// Cleanup time data structures
static void unload_time_layer() {
  text_layer_destroy(s_hour_layer);
  text_layer_destroy(s_min_layer);
  text_layer_destroy(s_colon_layer);
  fonts_unload_custom_font(s_time_font);
}

static void unload_battery_layer() {
  bitmap_layer_destroy(s_battery_img_layer);
  gbitmap_destroy(s_battery_image);
  text_layer_destroy(s_battery_layer);
  fonts_unload_custom_font(s_battery_font);
}

// Cleanup background data structures
static void unload_background_layer() {
  bitmap_layer_destroy(s_background_layer);
  gbitmap_destroy(s_background_image);
}

// Cleanup weather data structures
static void unload_weather_layer() {
  text_layer_destroy(s_temperature_layer);
  fonts_unload_custom_font(s_temperature_font);
}

// Cleanup date data structures
static void unload_date_layer() {
  text_layer_destroy(s_date_layer);
  fonts_unload_custom_font(s_date_font);
}

// Cleanup the main window's data structures
static void main_window_unload(Window *window) {
  unload_time_layer();
  unload_background_layer();
  unload_weather_layer();
  unload_date_layer();
  unload_battery_layer();
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  update_date();

  // Get weather update every 30 minutes
  if(tick_time->tm_min % 30 == 0) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);

    // Send the message!
    app_message_outbox_send();
  }
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming information
  static char temperature_buffer[8];
  static char conditions_buffer[32];
  static char weather_layer_buffer[32];

  // Read first item
  Tuple *t = dict_read_first(iterator);

  // For all items
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
    case KEY_TEMPERATURE:
      snprintf(temperature_buffer, sizeof(temperature_buffer), "%dC", (int)t->value->int32);
      break;
    case KEY_CONDITIONS:
      snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", t->value->cstring);
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      break;
    }

    // Look for next item
    t = dict_read_next(iterator);
  }

  // Assemble full string and display
  snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s, %s", temperature_buffer, conditions_buffer);
  text_layer_set_text(s_temperature_layer, weather_layer_buffer);
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

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
