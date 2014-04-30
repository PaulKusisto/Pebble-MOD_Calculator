#include "pebble.h"

// This is a custom defined key for saving our count field
#define PERCENT_OXYGEN_PKEY 1

// You can define defaults for values in persistent storage
#define PERCENT_OXYGEN_DEFAULT 21

#define MAXIMUM_PARTIAL_PRESSURE 1.4
  
static Window *window;

static GBitmap *action_icon_plus;
static GBitmap *action_icon_minus;

static ActionBarLayer *action_bar;

static TextLayer *header_text_layer;
static TextLayer *body_text_layer;
static TextLayer *label_text_layer;

// We'll save the count in memory from persistent storage
static int percent_oxygen = PERCENT_OXYGEN_DEFAULT;

static int update_MOD()
{
  float temp = 100 * (MAXIMUM_PARTIAL_PRESSURE / percent_oxygen);
  temp = (temp - 1)*33;
  return temp;
}

static void update_text() {
  static char body_text[50];
  snprintf(body_text, sizeof(body_text), "%u%% Oxygen", percent_oxygen);
  text_layer_set_text(body_text_layer, body_text);
  
  static char label_text[50];
  snprintf(label_text, sizeof(label_text), "MOD: %u feet", update_MOD());
  text_layer_set_text(label_text_layer, label_text);
}

static void increment_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (percent_oxygen >= 100) {
    // Keep the counter at 100
    return;
  }
  
  percent_oxygen++;
  update_text();
}

static void decrement_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (percent_oxygen <= 0) {
    // Keep the counter at zero
    return;
  }

  percent_oxygen--;
  update_text();
}

static void click_config_provider(void *context) {
  const uint16_t repeat_interval_ms = 50;
  window_single_repeating_click_subscribe(BUTTON_ID_UP, repeat_interval_ms, (ClickHandler) increment_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, repeat_interval_ms, (ClickHandler) decrement_click_handler);
}

static void window_load(Window *me) {
  action_bar = action_bar_layer_create();
  action_bar_layer_add_to_window(action_bar, me);
  action_bar_layer_set_click_config_provider(action_bar, click_config_provider);

  action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, action_icon_plus);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, action_icon_minus);

  Layer *layer = window_get_root_layer(me);
  const int16_t width = layer_get_frame(layer).size.w - ACTION_BAR_WIDTH - 3;

  header_text_layer = text_layer_create(GRect(4, 0, width, 60));
  text_layer_set_font(header_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_background_color(header_text_layer, GColorClear);
  text_layer_set_text(header_text_layer, "MOD Calculator");
  layer_add_child(layer, text_layer_get_layer(header_text_layer));

  body_text_layer = text_layer_create(GRect(4, 44, width, 60));
  text_layer_set_font(body_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_background_color(body_text_layer, GColorClear);
  layer_add_child(layer, text_layer_get_layer(body_text_layer));

  label_text_layer = text_layer_create(GRect(4, 44 + 28, width, 60));
  text_layer_set_font(label_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_background_color(label_text_layer, GColorClear);
  layer_add_child(layer, text_layer_get_layer(label_text_layer));

  update_text();
}

static void window_unload(Window *window) {
  text_layer_destroy(header_text_layer);
  text_layer_destroy(body_text_layer);
  text_layer_destroy(label_text_layer);

  action_bar_layer_destroy(action_bar);
}

static void init(void) {
  action_icon_plus = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_ICON_PLUS);
  action_icon_minus = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_ICON_MINUS);

  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });

  // Get the count from persistent storage for use if it exists, otherwise use the default
  percent_oxygen = persist_exists(PERCENT_OXYGEN_PKEY) ? persist_read_int(PERCENT_OXYGEN_PKEY) : PERCENT_OXYGEN_DEFAULT;

  window_stack_push(window, true /* Animated */);
}

static void deinit(void) {
  // Save the count into persistent storage on app exit
  persist_write_int(PERCENT_OXYGEN_PKEY, percent_oxygen);

  window_destroy(window);

  gbitmap_destroy(action_icon_plus);
  gbitmap_destroy(action_icon_minus);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
