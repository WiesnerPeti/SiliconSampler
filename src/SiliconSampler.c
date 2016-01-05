#include <pebble.h>
#include <stdlib.h>

static Window *s_main_window;

static char s_battery_buffer[16];
static TextLayer *s_battery_layer;

static char s_monthtime_buffer[20];
static TextLayer *s_month_layer;

static char s_daytime_buffer[20];
static TextLayer *s_day_layer;

static char s_time_buffer[8];
static TextLayer *s_time_layer;

static char s_wallText1[70];
static char s_wallText2[70];
static char s_wallText3[70];
static char s_wallText4[70];

static GFont s_font;

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
    
    // Get hours
    strftime(s_time_buffer, sizeof(s_time_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
    
    // Get day name
    strftime(s_daytime_buffer, sizeof(s_daytime_buffer), "%A", tick_time);
    
    // Get month time
    strftime(s_monthtime_buffer, sizeof(s_monthtime_buffer), "%h %d", tick_time);
    
    // Get battery
    BatteryChargeState charge_state = battery_state_service_peek();
    snprintf(s_battery_buffer, sizeof(s_battery_buffer), "%d%%", charge_state.charge_percent);
    
    snprintf(s_wallText1, sizeof(s_wallText1), "%s %s %s %s", s_time_buffer, s_daytime_buffer, s_monthtime_buffer, s_battery_buffer);
    snprintf(s_wallText2, sizeof(s_wallText2), "%s %s %s %s", s_daytime_buffer, s_monthtime_buffer, s_battery_buffer, s_time_buffer);
    snprintf(s_wallText3, sizeof(s_wallText3), "%s %s %s %s", s_monthtime_buffer, s_battery_buffer, s_time_buffer, s_daytime_buffer);
    snprintf(s_wallText4, sizeof(s_wallText4), "%s %s %s %s", s_battery_buffer, s_time_buffer, s_daytime_buffer, s_monthtime_buffer);
    
    text_layer_set_text(s_time_layer, s_wallText1);
    text_layer_set_text(s_day_layer, s_wallText2);
    text_layer_set_text(s_month_layer, s_wallText3);
    text_layer_set_text(s_battery_layer, s_wallText4);
}

static void layoutTexts()
{
    srand(time(NULL));
    
    Layer *window_layer = window_get_root_layer(s_main_window);
    GRect bounds = layer_get_bounds(window_layer);
    
    GSize textSize = GSize(2*bounds.size.w, 44);
    
    GRect timeFrame = GRect((rand()%4)*2-10,
                            -8,
                            textSize.w,
                            textSize.h);
    
    GRect dayFrame = GRect((rand()%6)*4-20,
                           GRectMaxY(timeFrame),
                           textSize.w,
                           textSize.h);
    
    GRect monthFrame = GRect((rand()%5)*6-30,
                             GRectMaxY(dayFrame),
                             textSize.w,
                             textSize.h);
    
    GRect batteryFrame = GRect((rand()%5)*2-8,
                               GRectMaxY(monthFrame),
                               textSize.w,
                               textSize.h);
    
    layer_set_frame(text_layer_get_layer(s_time_layer), timeFrame);
    layer_set_frame(text_layer_get_layer(s_day_layer), dayFrame);
    layer_set_frame(text_layer_get_layer(s_month_layer), monthFrame);
    layer_set_frame(text_layer_get_layer(s_battery_layer), batteryFrame);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    updateData();
    layoutTexts();
}

static TextLayer * textLayerWithFont(GFont font)
{
    TextLayer *layer = text_layer_create(GRectZero);
    
    text_layer_set_background_color(layer, GColorClear);
    text_layer_set_text_color(layer, GColorWhite);
    text_layer_set_text_alignment(layer, GTextAlignmentLeft);
    text_layer_set_font(layer, font);
    
    return layer;
}

static void main_window_load(Window *window) {
    // Get information about the Window
    Layer *window_layer = window_get_root_layer(window);
    
    //RESOURCES
    // Create GFont
    s_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_SAN_FRANSISCO_44));
    
    s_time_layer    = textLayerWithFont(s_font);
    s_day_layer     = textLayerWithFont(s_font);
    s_month_layer   = textLayerWithFont(s_font);
    s_battery_layer = textLayerWithFont(s_font);
    
    // Add it as a child layer to the Window's root layer
    layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
    layer_add_child(window_layer, text_layer_get_layer(s_day_layer));
    layer_add_child(window_layer, text_layer_get_layer(s_month_layer));
    layer_add_child(window_layer, text_layer_get_layer(s_battery_layer));
    
    updateData();
    layoutTexts();
}

static void main_window_unload(Window *window) {

    // Destroy TextLayer
    text_layer_destroy(s_time_layer);
    text_layer_destroy(s_day_layer);
    text_layer_destroy(s_month_layer);
    text_layer_destroy(s_battery_layer);
    
    // Destroy Fonts
    fonts_unload_custom_font(s_font);
}

static void init() {
    
    // Create main Window element and assign to pointer
    s_main_window = window_create();
    window_set_background_color(s_main_window,GColorBlack);
    
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
