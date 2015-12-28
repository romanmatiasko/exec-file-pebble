#include "exec_file.h"

static Window *main_window;
static TextLayer *text_layer;
static MenuLayer *menu_layer;


static uint16_t menu_get_num_rows_cb(MenuLayer *menu_layer,
                                     uint16_t section_idx, void *ctx) {
  return available_files_length;
}

static void menu_draw_row_cb(GContext* g_ctx, const Layer *cell_layer,
                             MenuIndex *cell_idx, void *ctx) {
  menu_cell_basic_draw(g_ctx, cell_layer, available_files[cell_idx->row][1],
                       available_files[cell_idx->row][0], NULL);
}

#ifdef PBL_RECT
static int16_t menu_get_header_height_cb(MenuLayer *menu_layer,
                                         uint16_t section_idx, void *ctx) {
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void menu_draw_header_cb(GContext* g_ctx, const Layer *cell_layer,
                                uint16_t section_idx, void *ctx) {
  menu_cell_basic_header_draw(g_ctx, cell_layer, "Select file to execute");
}
#endif

#ifdef PBL_ROUND 
static int16_t get_cell_height_cb(MenuLayer *menu_layer, MenuIndex *cell_idx,
                                  void *ctx) {
  if (menu_layer_is_index_selected(menu_layer, cell_idx)) {
    return MENU_CELL_ROUND_FOCUSED_SHORT_CELL_HEIGHT;
  } else {
    return MENU_CELL_ROUND_UNFOCUSED_SHORT_CELL_HEIGHT;
  }
}
#endif

static void menu_select_cb(MenuLayer *menu_layer,
                           MenuIndex *cell_idx, void *ctx) {
  send_app_message(TupletCString(menuItemKey,
                                 available_files[cell_idx->row][0]));
}

static void menu_selection_changed_cb(MenuLayer *menu_layer, MenuIndex new_idx,
                                      MenuIndex old_idx, void *ctx) {
  layer_remove_from_parent(text_layer_get_layer(text_layer));
}

void main_window_add_menu() {
  Layer *window_layer = window_get_root_layer(main_window);
  GRect bounds = layer_get_bounds(window_layer);
  menu_layer = menu_layer_create(bounds);
  GColor8 h_bg_color = PBL_IF_COLOR_ELSE(GColorDarkGreen, GColorWhite);
  GColor8 h_fg_color = PBL_IF_COLOR_ELSE(GColorWhite, GColorBlack);

  menu_layer_set_normal_colors(menu_layer, GColorBlack, GColorWhite);
  menu_layer_set_highlight_colors(menu_layer, h_bg_color, h_fg_color);
  menu_layer_set_callbacks(menu_layer, NULL, (MenuLayerCallbacks) {
    .get_num_rows = menu_get_num_rows_cb,
    .get_header_height = PBL_IF_RECT_ELSE(menu_get_header_height_cb, NULL),
    .draw_header = PBL_IF_RECT_ELSE(menu_draw_header_cb, NULL),
    .draw_row = menu_draw_row_cb,
    .select_click = menu_select_cb,
    .get_cell_height = PBL_IF_ROUND_ELSE(get_cell_height_cb, NULL),
    .selection_changed = menu_selection_changed_cb,
  });
  menu_layer_set_click_config_onto_window(menu_layer, main_window);
  layer_add_child(window_layer, menu_layer_get_layer(menu_layer));
}

void main_window_add_message(Tuple *tuple) {
  Layer *window_layer = window_get_root_layer(main_window);
  GRect bounds = layer_get_bounds(window_layer);
  GTextAlignment text_alignment = PBL_IF_RECT_ELSE(GTextAlignmentLeft,
                                                   GTextAlignmentCenter);
  GColor8 background_color = PBL_IF_COLOR_ELSE(tuple->key == msgErrorKey
    ? GColorBulgarianRose
    : GColorBlack, GColorBlack);

  text_layer_set_text_alignment(text_layer, text_alignment);
  text_layer_set_text(text_layer, tuple->value->cstring);
  text_layer_set_background_color(text_layer, background_color);
  text_layer_set_text_color(text_layer, GColorWhite);
  layer_set_frame(text_layer_get_layer(text_layer), (GRect) {
    .origin = {0, PBL_IF_RECT_ELSE(0, 10)},
    .size = {bounds.size.w, PBL_IF_RECT_ELSE(bounds.size.h,
                                             bounds.size.h - 20)},
  });
  layer_add_child(window_layer, text_layer_get_layer(text_layer));
#ifdef PBL_ROUND
  text_layer_enable_screen_text_flow_and_paging(text_layer, 5);
#endif
}

static void main_window_load(Window *main_window) {
  Layer *window_layer = window_get_root_layer(main_window);
  GRect bounds = layer_get_bounds(window_layer);
  text_layer = text_layer_create((GRect) {
    .origin = {0, 72},
    .size = {bounds.size.w, 20},
  });

  text_layer_set_text(text_layer, "Fetching data...");
  text_layer_set_background_color(text_layer, GColorBlack);
  text_layer_set_text_color(text_layer, GColorWhite);
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));
}

static void main_window_unload(Window *main_window) {
  menu_layer_destroy(menu_layer);
  text_layer_destroy(text_layer);
}

static void init(void) {
  main_window = window_create();

  window_set_background_color(main_window, GColorBlack);
  window_set_window_handlers(main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(main_window, true);
  app_message_init();
}

static void deinit(void) {
  window_destroy(main_window);
  app_message_deinit();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
