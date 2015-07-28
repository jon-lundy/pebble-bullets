#include <pebble.h>
  
static const int TOP_BOTTOM_MARGIN = 2;
static const int TOP_BOTTOM_HEIGHT = 14;
static const int MAX_BATTERY_VALUE = 100;
static const int BATTERY_BULLET_RADIUS = 4;
static const int BATTERY_BULLET_VALUE = 10; //i.e., how much battery one dot represents
static const int TIME_BULLET_MARGIN = 2; // a specific radius isn't mentioned for these, because they take up the rest of the space
static const int DATE_STRING_LEN = 16;

static Window *s_main_window;

static Layer *s_ul_time_bullet_layer;
static Layer *s_ur_time_bullet_layer;
static Layer *s_ll_time_bullet_layer;
static Layer *s_lr_time_bullet_layer;
static Layer *s_battery_layer;
static Layer *s_date_layer;

static char *date_string;

static void update_time() {
  // Get a tm structure, pull out needed data
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
  int hour = tick_time->tm_hour;
  int minute = tick_time->tm_min;
  
  // TODO: add this to a settings page
  // for now, always use 24h for more variety
  
  // convert 24h to 12h time, if requested
  //if (!clock_is_24h_style()) {
  //  if (hour > 12) {
  //    hour -= 12;
  //  } else if (hour == 0) {
  //    hour = 12;
  //  }
  //}
  
  // update time layers data
  *(char*)layer_get_data(s_ul_time_bullet_layer) = (hour / 10) + '0';
  *(char*)layer_get_data(s_ur_time_bullet_layer) = (hour % 10) + '0';
  *(char*)layer_get_data(s_ll_time_bullet_layer) = (minute / 10) + '0';
  *(char*)layer_get_data(s_lr_time_bullet_layer) = (minute % 10) + '0';
  
  // update date layer data
  strftime(date_string, DATE_STRING_LEN, "%a %d %b", tick_time);
  
  // mark layers dirty
  // todo could be smarter by looking at old value, instead of invalidating all
  layer_mark_dirty(s_ul_time_bullet_layer);
  layer_mark_dirty(s_ur_time_bullet_layer);
  layer_mark_dirty(s_ll_time_bullet_layer);
  layer_mark_dirty(s_lr_time_bullet_layer);
  layer_mark_dirty(s_date_layer);
}

static void update_battery_indicator() {
  // get battery state, update layer data and mark dirty
  BatteryChargeState bcs = battery_state_service_peek();
  *(uint8_t*)layer_get_data(s_battery_layer) = bcs.charge_percent;
  layer_mark_dirty(s_battery_layer);
}

static GColor get_color_for_bullet(char *bullet_char) {
  switch (*bullet_char) {
    case '1':
      return GColorIslamicGreen;
    case '2':
      return GColorChromeYellow;
    case '3':
      return GColorBulgarianRose;
    case '4':
      return GColorIndigo;
    case '5':
      return GColorCyan;
    case '6':
      return GColorBrass;
    case '7':
      return GColorRed;
    case '8':
      return GColorMagenta;
    case '9':
      return GColorBlue;
    case '0':
    default:
      return GColorDarkGray;
  }
}

static void draw_bullet_layer(struct Layer *layer, GContext *ctx) {
  // figure out size
  GRect bounds = layer_get_bounds(layer);
  int bullet_radius = bounds.size.w / 2; // assuming width and height are same here -- they should be based on construction
  
  // draw bullet
  char* bullet_char = (char*)layer_get_data(layer);
  graphics_context_set_fill_color(ctx, get_color_for_bullet(bullet_char));
  graphics_fill_circle(ctx, GPoint(bullet_radius, bullet_radius), bullet_radius);
  
  // measure and draw text
  graphics_context_set_text_color(ctx, GColorWhite);
  uint8_t height_fudge = bounds.size.h / 10; // i don't like this either. but I was having trouble geting the text vertically centered otherwise.
  graphics_draw_text(ctx, bullet_char, fonts_get_system_font(FONT_KEY_BITHAM_42_MEDIUM_NUMBERS), GRect(bounds.origin.x, bounds.origin.y + height_fudge, bounds.size.w, bounds.size.h - height_fudge), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
}

static void draw_battery_layer(struct Layer *layer, GContext *ctx) {
  // figure out how much space to allocate for each bullet
  GRect bounds = layer_get_bounds(layer);
  uint8_t bullet_spaces = MAX_BATTERY_VALUE / BATTERY_BULLET_VALUE;
  uint8_t bullet_space_width = bounds.size.w / bullet_spaces;
  uint8_t bullet_space_half_width = bullet_space_width / 2;
  uint8_t unused_width = bounds.size.w - (bullet_space_width * bullet_spaces); // to account for truncation (i.e. unused pixels)
  uint8_t left_margin = unused_width / 2;
  uint8_t bullet_space_half_height = bounds.size.h / 2;
  
  // figure out how many bullets to draw
  uint8_t battery_value = *(uint8_t*)layer_get_data(layer);
  uint8_t bullets_to_draw = battery_value / BATTERY_BULLET_VALUE;
  
  // draw them
  graphics_context_set_fill_color(ctx, GColorWhite);
  for (uint8_t count = 0; count < bullets_to_draw; count++) {
    graphics_fill_circle(ctx, GPoint(left_margin + bullet_space_half_width * (2 * count + 1), bullet_space_half_height), BATTERY_BULLET_RADIUS);
  }
}

static void draw_date_layer(struct Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  graphics_context_set_text_color(ctx, GColorWhite);
  graphics_draw_text(ctx, date_string, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), bounds, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
}

static void setup_bullet_layer(struct Layer *window_root_layer, struct Layer *layer) {
  *(char*)layer_get_data(layer) = '0'; // not sure if this is neccesary? populating with an initial value
  layer_set_update_proc(layer, draw_bullet_layer);
  layer_set_clips(layer, false);
  layer_add_child(window_root_layer, layer);
}

static void main_window_load(Window *window) {
  // commonly-used info for setting up child layers
  Layer *window_root_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_root_layer);
  
  // Create battery state layer (top)
  s_battery_layer = layer_create_with_data(GRect(bounds.origin.x, bounds.origin.y + TOP_BOTTOM_MARGIN, bounds.size.w, TOP_BOTTOM_HEIGHT), sizeof(uint8_t));
  *(uint8_t*)layer_get_data(s_battery_layer) = 50; // not sure if this is neccesary? populating with an initial value
  layer_set_update_proc(s_battery_layer, draw_battery_layer);
  layer_add_child(window_root_layer, s_battery_layer);
  
  // Create and setup layers that will contain the 4 bullets
  uint8_t window_mid_x = bounds.origin.x + bounds.size.w / 2;
  uint8_t window_mid_y = bounds.origin.y + bounds.size.h / 2;  
  uint8_t bullet_layer_height = (bounds.size.h - (2 * TOP_BOTTOM_HEIGHT) - (2 * TOP_BOTTOM_MARGIN) - 4 * TIME_BULLET_MARGIN) / 2;
  uint8_t bullet_layer_width = (bounds.size.w - 4 * TIME_BULLET_MARGIN) / 2;
  uint8_t bullet_layer_size = bullet_layer_height < bullet_layer_width ? bullet_layer_height : bullet_layer_width;
  
  s_ul_time_bullet_layer = layer_create_with_data(GRect(window_mid_x - bullet_layer_size - TIME_BULLET_MARGIN, window_mid_y - bullet_layer_size - TIME_BULLET_MARGIN, bullet_layer_size, bullet_layer_size), sizeof(char));
  setup_bullet_layer(window_root_layer, s_ul_time_bullet_layer);
  
  s_ur_time_bullet_layer = layer_create_with_data(GRect(window_mid_x + TIME_BULLET_MARGIN, window_mid_y - bullet_layer_size - TIME_BULLET_MARGIN, bullet_layer_size, bullet_layer_size), sizeof(char));
  setup_bullet_layer(window_root_layer, s_ur_time_bullet_layer);
  
  s_ll_time_bullet_layer = layer_create_with_data(GRect(window_mid_x - bullet_layer_size - TIME_BULLET_MARGIN, window_mid_y + TIME_BULLET_MARGIN, bullet_layer_size, bullet_layer_size), sizeof(char));
  setup_bullet_layer(window_root_layer, s_ll_time_bullet_layer);
  
  s_lr_time_bullet_layer = layer_create_with_data(GRect(window_mid_x + TIME_BULLET_MARGIN, window_mid_y + TIME_BULLET_MARGIN, bullet_layer_size, bullet_layer_size), sizeof(char));
  setup_bullet_layer(window_root_layer, s_lr_time_bullet_layer);
  
  // Create date layer (bottom)
  s_date_layer = layer_create(GRect(bounds.origin.x, bounds.origin.y + bounds.size.h - TOP_BOTTOM_HEIGHT - TOP_BOTTOM_MARGIN, bounds.size.w, TOP_BOTTOM_HEIGHT));
  layer_set_update_proc(s_date_layer, draw_date_layer);
  layer_add_child(window_root_layer, s_date_layer);
}

static void main_window_unload(Window *window) {  
  // destroy date layer
  layer_destroy(s_date_layer);
  
  // destroy bullet layers
  layer_destroy(s_ul_time_bullet_layer);
  layer_destroy(s_ur_time_bullet_layer);
  layer_destroy(s_ll_time_bullet_layer);
  layer_destroy(s_lr_time_bullet_layer);
  
  // destroy battery state layer
  layer_destroy(s_battery_layer);
}

static void handle_time_tick(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void handle_battery_state(BatteryChargeState charge) {
  update_battery_indicator();
}

static void init() {  
  // init array for storing date string
  date_string = malloc(DATE_STRING_LEN * sizeof(char));
  
  // create main Window (black background), associate handlers, and display
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, handle_time_tick);
  
  // Register with BatteryStateService
  battery_state_service_subscribe(handle_battery_state);
  
  // Display initial state
  update_time();
  update_battery_indicator();
}

static void deinit() {
  // unsubscribe from services
  battery_state_service_unsubscribe();
  tick_timer_service_unsubscribe();
  
  // cleanup Window
  window_destroy(s_main_window);
  
  // free date string
  free(date_string);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}