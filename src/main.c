// TAKE II

#include <pebble.h>

#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1
#define KEY_CONFIG_TEMP_UNIT_F 2

static Window      *s_main_window;        // main window
static TextLayer   *s_conditions_layer;   // text layer displaying conditions
static TextLayer   *s_hour_layer0;        // text layer displaying hours
static TextLayer   *s_min_layer0;         // text layer displaying minutes
static TextLayer   *s_hour_layer1;        // text layer displaying hours
static TextLayer   *s_min_layer1;         // text layer displaying minutes
static TextLayer   *s_colon_layer;        // text layer displaying colon
static TextLayer   *s_date_layer;         // text layer
static TextLayer   *s_temperature_layer;  // text layer displaying temperature
static GFont        s_time_font;          // time display font
static GFont        s_conditions_font;    // conditions display font
static GFont        s_temperature_font;   // font used to display temperature
static GFont        s_date_font;          // date display font
static GFont        s_colon_font;         // time colon display font
static GBitmap     *s_background_image;   // background image
static BitmapLayer *s_background_layer;   // background image container

static bool s_configTempF = false;

// define all the fonts used
static void load_fonts() {
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_NHL_VAN_64));
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_NHL_VAN_28));
  s_conditions_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_NHL_VAN_22));
  s_temperature_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_NHL_VAN_22));
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
  s_conditions_layer = text_layer_create(GRect(18, 126, 108, 50));
  text_layer_set_background_color(s_conditions_layer, GColorClear);
  text_layer_set_text_color(s_conditions_layer, GColorWhite);
  text_layer_set_text_alignment(s_conditions_layer, GTextAlignmentRight);
  text_layer_set_font(s_conditions_layer, s_conditions_font);
  text_layer_set_text(s_conditions_layer, "    ");
}

static void create_temperature_layer() {
  s_temperature_layer = text_layer_create(GRect(22, 126, 30, 50)); //GRect: x,y,w,h
  text_layer_set_background_color(s_temperature_layer, GColorClear);
  text_layer_set_text_color(s_temperature_layer, GColorWhite);
  text_layer_set_text_alignment(s_temperature_layer, GTextAlignmentLeft);
  text_layer_set_font(s_temperature_layer, s_conditions_font);
  text_layer_set_text(s_temperature_layer, "    ");
}

// build up the pieces that display the time information
static void create_time_layer() {

  s_hour_layer0 = text_layer_create(GRect(22, 10, 30, 70)); //GRect: x,y,w,h
  s_hour_layer1 = text_layer_create(GRect(45, 10, 30, 70)); //GRect: x,y,w,h

  s_min_layer0 = text_layer_create(GRect(76, 10, 30, 70));
  s_min_layer1 = text_layer_create(GRect(99, 10, 30, 70));

  text_layer_set_background_color(s_hour_layer0, GColorClear);
  text_layer_set_background_color(s_hour_layer1, GColorClear);
  text_layer_set_background_color(s_min_layer0, GColorClear);
  text_layer_set_background_color(s_min_layer1, GColorClear);
  text_layer_set_text_color(s_hour_layer0, GColorWhite);
  text_layer_set_text_color(s_hour_layer1, GColorWhite);
  text_layer_set_text_color(s_min_layer0, GColorWhite);
  text_layer_set_text_color(s_min_layer1, GColorWhite);
  text_layer_set_text(s_hour_layer0, " ");
  text_layer_set_text(s_hour_layer1, " ");
  text_layer_set_text(s_min_layer0, "  ");
  text_layer_set_text(s_min_layer1, "  ");
  text_layer_set_font(s_hour_layer0, s_time_font);
  text_layer_set_font(s_hour_layer1, s_time_font);
  text_layer_set_font(s_min_layer0, s_time_font);
  text_layer_set_font(s_min_layer1, s_time_font);
  text_layer_set_text_alignment(s_hour_layer0, GTextAlignmentLeft);
  text_layer_set_text_alignment(s_hour_layer1, GTextAlignmentLeft);
  text_layer_set_text_alignment(s_min_layer0, GTextAlignmentLeft);
  text_layer_set_text_alignment(s_min_layer1, GTextAlignmentLeft);

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

static void _setNumberLayerAlignment(char *value, TextLayer *layer) {
  int ival = atoi(value);
  if (ival == 1) {
    APP_LOG(APP_LOG_LEVEL_INFO, "%d Value is 1, centering...", ival);
    text_layer_set_text_alignment(layer, GTextAlignmentCenter);
  } else {
    APP_LOG(APP_LOG_LEVEL_INFO, "%d Value is NOT 0, lefting...", ival);
    text_layer_set_text_alignment(layer, GTextAlignmentLeft);
  }
}
static void _setLayerAlignment(char *hour0, char *hour1, char *min0, char *min1) {
  _setNumberLayerAlignment(hour0, s_hour_layer0);
  _setNumberLayerAlignment(hour1, s_hour_layer1);
  _setNumberLayerAlignment(min0, s_min_layer0);
  _setNumberLayerAlignment(min1, s_min_layer1);
}

static void update_time() {
  APP_LOG(APP_LOG_LEVEL_INFO, "update_time()");
  // build and populate the time structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Create 2 long-lived buffers for the hour and min
  static char hour_buffer[] = "00";
  static char min_buffer[] = "00";

  //update the hour and min buffers with formatted values from the time strucure
  if (!clock_is_24h_style()) {
    strftime(hour_buffer, sizeof("00"), "%I", tick_time);
  } else {
    strftime(hour_buffer, sizeof("00"), "%H", tick_time);
  }
  strftime(min_buffer, sizeof("00"), "%M", tick_time);

  static char hour0[] = "0";
  static char hour1[] = "0";
  static char min0[] = "0";
  static char min1[] = "0";
  strncpy(hour0, hour_buffer, 1);
  strncpy(hour1, hour_buffer+1, 1);
  strncpy(min0, min_buffer, 1);
  strncpy(min1, min_buffer+1, 1);

  //update the display's hour and min layers with the formatted values
  _setLayerAlignment(hour0, hour1, min0, min1);
  text_layer_set_text(s_hour_layer0, hour0);
  text_layer_set_text(s_hour_layer1, hour1);
  text_layer_set_text(s_min_layer0, min0);
  text_layer_set_text(s_min_layer1, min1);
}


// Load up the main window
static void main_window_load(Window *window) {
  load_fonts();
  create_time_layer();
  create_background_layer();
  create_weather_layer();
  create_date_layer();
  create_temperature_layer();

  // Add each layer as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_hour_layer0));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_hour_layer1));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_min_layer0));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_min_layer1));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_colon_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_conditions_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_temperature_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_temperature_layer));
}

// Cleanup time data structures
static void unload_time_layer() {
  text_layer_destroy(s_hour_layer0);
  text_layer_destroy(s_hour_layer1);
  text_layer_destroy(s_min_layer0);
  text_layer_destroy(s_min_layer1);
  text_layer_destroy(s_colon_layer);
  fonts_unload_custom_font(s_time_font);
}

static void unload_temperature_layer() {
  text_layer_destroy(s_temperature_layer);
  fonts_unload_custom_font(s_temperature_font);
}

// Cleanup background data structures
static void unload_background_layer() {
  bitmap_layer_destroy(s_background_layer);
  gbitmap_destroy(s_background_image);
}

// Cleanup weather data structures
static void unload_weather_layer() {
  text_layer_destroy(s_conditions_layer);
  fonts_unload_custom_font(s_conditions_font);
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
  unload_temperature_layer();
}

static void update_weather() {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  // Add a key-value pair
  dict_write_uint8(iter, 0, 0);

  // Send the message!
  app_message_outbox_send();
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  update_date();

  // Get weather update every 30 minutes
  if(tick_time->tm_min % 30 == 0) {
    update_weather();
  }
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming information
  static char temperature_buffer[8];
  static char conditions_buffer[32];
  static char weather_layer_buffer[32];
  static int degC = 0;
  static int degF = 0;

  // Read first item
  Tuple *t = dict_read_first(iterator);

  // For all items
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
      case KEY_TEMPERATURE:
        degC = (int)t->value->int32;
        if (s_configTempF) {
          degF = (degC * (9 / 5)) + 32;
          snprintf(temperature_buffer, sizeof(temperature_buffer), "%dF", degF);
        } else {
          snprintf(temperature_buffer, sizeof(temperature_buffer), "%dC", degC);
        }
        text_layer_set_text(s_temperature_layer, temperature_buffer);
        break;
      case KEY_CONDITIONS:
        snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", t->value->cstring);
        text_layer_set_text(s_conditions_layer, conditions_buffer);
        break;
      case KEY_CONFIG_TEMP_UNIT_F:
        s_configTempF = (bool)t->value->int32;
        update_weather();
      default:
        APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
        break;
    }

    // Look for next item
    t = dict_read_next(iterator);
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
