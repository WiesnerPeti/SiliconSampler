#include <pebble.h>

static Window *s_main_window;

static char s_battery_buffer[16];
static char s_monthtime_buffer[20];
static char s_daytime_buffer[20];
static char s_time_buffer[8];

static TextLayer *s_second_layer;
static TextLayer *s_first_layer;

static GFont s_font;
static GFont s_small_font;

static int s_phase = 0;

#define GRectMaxY(__RECT__) (__RECT__.origin.y + __RECT__.size.h)

//Helpers
void logFrame(GRect frame)
{
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Frame %d %d %d %d", frame.origin.x, frame.origin.y, frame.size.w, frame.size.h);
}

//Update
static void updateData() {
    // Get a tm structure
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);
    
    s_phase = (temp / 60) % 3;
    
    // Get hours
    strftime(s_time_buffer, sizeof(s_time_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
    
    // Get day name
    strftime(s_daytime_buffer, sizeof(s_daytime_buffer), "%A", tick_time);
    
    // Get month time
    strftime(s_monthtime_buffer, sizeof(s_monthtime_buffer), "%Y %h %d", tick_time);
    
    // Get battery
    BatteryChargeState charge_state = battery_state_service_peek();
    snprintf(s_battery_buffer, sizeof(s_battery_buffer), "Battery:%d%%", charge_state.charge_percent);
}

static void updateTextLayers()
{
    text_layer_set_text(s_first_layer, s_time_buffer);
    
    switch (s_phase) {
        case 1:
            text_layer_set_text(s_second_layer, s_monthtime_buffer);
            break;
        case 2:
            text_layer_set_text(s_second_layer, s_battery_buffer);
            break;
        case 0:
        default:
            text_layer_set_text(s_second_layer, s_daytime_buffer);
            break;
    }
}

static void layoutTextLayers()
{
    Layer *window_layer = window_get_root_layer(s_main_window);
    GRect bounds = layer_get_bounds(window_layer);
    
    GRect firstFrame = GRect(0,
                            bounds.size.h/2.0 - 50,
                            bounds.size.w,
                            76);
    
    GRect secondFrame = GRect(0,
                           GRectMaxY(firstFrame)+2,
                           bounds.size.w,
                           18);
    
    layer_set_frame(text_layer_get_layer(s_first_layer), firstFrame);
    layer_set_frame(text_layer_get_layer(s_second_layer), secondFrame);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    updateData();
    updateTextLayers();
    layoutTextLayers();
}

static TextLayer * textLayerWithFont(GFont font)
{
    TextLayer *layer = text_layer_create(GRectZero);
    
    text_layer_set_background_color(layer, GColorClear);
    text_layer_set_text_color(layer, GColorBlack);
    text_layer_set_text_alignment(layer, GTextAlignmentCenter);
    text_layer_set_font(layer, font);
    
    return layer;
}

static void main_window_load(Window *window) {
    // Get information about the Window
    Layer *window_layer = window_get_root_layer(window);
    
    //RESOURCES
    // Create GFont
    s_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_STEEL_FISH_74));
    s_small_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_SAN_FRANSISCO_16));
    
    s_first_layer    = textLayerWithFont(s_font);
    s_second_layer   = textLayerWithFont(s_small_font);
    
    // Add it as a child layer to the Window's root layer
    layer_add_child(window_layer, text_layer_get_layer(s_first_layer));
    layer_add_child(window_layer, text_layer_get_layer(s_second_layer));
    
    updateData();
    updateTextLayers();
    layoutTextLayers();
}

static void main_window_unload(Window *window) {

    // Destroy TextLayer
    text_layer_destroy(s_first_layer);
    text_layer_destroy(s_second_layer);
    
    // Destroy Fonts
    fonts_unload_custom_font(s_font);
    fonts_unload_custom_font(s_small_font);
}

static void init() {
    
    // Create main Window element and assign to pointer
    s_main_window = window_create();
    window_set_background_color(s_main_window,GColorWhite);
    
    // Set handlers to manage the elements inside the Window
    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = main_window_load,
        .unload = main_window_unload
    });
    
    // Show the Window on the watch, with animated=true
    window_stack_push(s_main_window, true);
    
    // Register with TickTimerService
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
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
