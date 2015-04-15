#include <pebble.h>
  
static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_day_of_week_layer;
static GFont s_time_font;
static GFont s_date_font;
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;

static void ordinal_suffix(int number, char *buf) {
  int last_digit;
  
  last_digit = number % 10;
  switch(last_digit) {
    case 1:
      strcpy(buf, "st");
      break;
    case 2:
      strcpy(buf, "nd");
      break;
    case 3:
      strcpy(buf, "rd");
      break;
    default:
      strcpy(buf, "th");
  }
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  // Create a long-lived buffer
  static char buffer[] = "00:00";
  
  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    // User 24 hour format
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
  }
  
  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, buffer);
}

static void update_date() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  char day_of_week[16];
  char day_of_month[4];
  char date_suffix[4];
  
  static char day_of_week_buffer[32];
  static char date_buffer[32];
  
  strftime(day_of_week, sizeof(day_of_week), "%A", tick_time);
  snprintf(day_of_month, sizeof(day_of_month), "%d", tick_time->tm_mday);
  ordinal_suffix(tick_time->tm_mday, date_suffix);
  snprintf(day_of_week_buffer, sizeof(day_of_week), "%s", day_of_week);
  snprintf(date_buffer, sizeof(date_buffer), "the %s%s", day_of_month, date_suffix);

  text_layer_set_text(s_day_of_week_layer, day_of_week_buffer);
  text_layer_set_text(s_date_layer, date_buffer);
}

static void main_window_load(Window *window) {
  // Create GBitmap, then set to created BitmapLayer
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
  s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));
  
  // Create GFonts
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_48));
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_16));
  
  // Create time TextLayer
  s_time_layer = text_layer_create(GRect(2, 50, 144, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  
  // Improve the layout to be more like a watchface
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  
   // Create day of week TextLayer
  s_day_of_week_layer = text_layer_create(GRect(2, 20, 144, 25));
  text_layer_set_background_color(s_day_of_week_layer, GColorBlack);
  text_layer_set_text_color(s_day_of_week_layer, GColorWhite);
  text_layer_set_font(s_day_of_week_layer, s_date_font);
  text_layer_set_text_alignment(s_day_of_week_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_day_of_week_layer));
  
  // Create date TextLayer
  s_date_layer = text_layer_create(GRect(2, 140, 144, 25));
  text_layer_set_background_color(s_date_layer, GColorBlack);
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_font(s_date_layer, s_date_font);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
}

static void main_window_unload(Window *window) {
  // Destroy TextLayers
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  
  // Unload GFonts
  fonts_unload_custom_font(s_time_font);
  fonts_unload_custom_font(s_date_font);
  
  // Destroy GBitmap
  gbitmap_destroy(s_background_bitmap);
  
  // Destroy BitmapLayer
  bitmap_layer_destroy(s_background_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  update_date();
}

static void init() {
  // Create main Window element and assign to point
  s_main_window = window_create();
  
  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
    
  // Register with the TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Make sure the time is displayed from the start
  update_time();
  update_date();
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