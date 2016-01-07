#include <pebble.h>

static Window *s_main_window;

static char s_battery_buffer[16];
static char s_date_buffer[20];
static char s_weekday_buffer[20];
static char s_time_buffer[8];

static TextLayer *s_second_layer;
static TextLayer *s_first_layer;

static GFont s_font;
static GFont s_small_font;

#define GRectMaxY(__RECT__) (__RECT__.origin.y + __RECT__.size.h)
#define inRange(lower, v, upper) (lower <= v && v < upper)

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

    //First layer is always time
    strftime(s_time_buffer, sizeof(s_time_buffer), "%H:%M", tick_time);
    text_layer_set_text(s_first_layer, s_time_buffer);

    int hour = tick_time->tm_hour;
    int minutes = tick_time->tm_min;

    //Away from desktop time, now the exact date is important
    if (inRange(7, hour, 8) || inRange(12, hour, 13) || inRange(18, hour, 22))
    {
        strftime(s_date_buffer, sizeof(s_date_buffer), "%Y %h %d", tick_time);
        text_layer_set_text(s_second_layer, s_date_buffer);
    }
    //Night time, battery important to charge if needed
    else if (inRange(0, hour, 7) || hour == 22 || hour == 23)
    {
        text_layer_set_text(s_second_layer, s_battery_buffer);
    }
    //Rotate information
    else
    {
        int s_phase = minutes % 3;
        switch (s_phase) {
            case 1:
                strftime(s_date_buffer, sizeof(s_date_buffer), "%Y %h %d", tick_time);
                text_layer_set_text(s_second_layer, s_date_buffer);
                break;
            case 2:
                text_layer_set_text(s_second_layer, s_battery_buffer);
                break;
            case 0:
            default:{
                strftime(s_weekday_buffer, sizeof(s_weekday_buffer), "%A", tick_time);
                text_layer_set_text(s_second_layer, s_weekday_buffer);
                break;
            }
        }
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
}

static void battery_handler(BatteryChargeState charge_state) {
  
    snprintf(s_battery_buffer, sizeof(s_battery_buffer), "Battery:%d%%", charge_state.charge_percent);
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
    
    Layer *window_layer = window_get_root_layer(window);
    
    s_font          = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_STEEL_FISH_74));
    s_small_font    = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_SAN_FRANSISCO_16));
    
    s_first_layer    = textLayerWithFont(s_font);
    s_second_layer   = textLayerWithFont(s_small_font);
    
    layer_add_child(window_layer, text_layer_get_layer(s_first_layer));
    layer_add_child(window_layer, text_layer_get_layer(s_second_layer));
    
    battery_handler(battery_state_service_peek());

    updateData();

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
    
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

    battery_state_service_subscribe(battery_handler);
}

static void deinit() {
    
    window_destroy(s_main_window);

    battery_state_service_unsubscribe();
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}
