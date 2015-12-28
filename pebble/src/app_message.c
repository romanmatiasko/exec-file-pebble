#include "app_message.h"

char ***available_files;
int available_files_length = 0;
static const int file_tuple_length = 2;


static void append_received_file(int received_files_length,
                                 DictionaryIterator *iterator) {
  Tuple *file_tuple = dict_find(iterator, fileNameKey);
  Tuple *label_tuple = dict_find(iterator, fileLabelKey);
  const int idx = available_files_length;
  available_files_length++;

  if (idx == 0) {
    available_files = malloc(received_files_length * sizeof(char *));
    if (available_files == NULL) {
      APP_LOG(APP_LOG_LEVEL_ERROR,
              "OUT_OF_MEMORY available_files[%d]", received_files_length);
      window_stack_pop_all(false);
      return;
    }
  }

  available_files[idx] = malloc(file_tuple_length * sizeof(char *));
  available_files[idx][0] = malloc(strlen(file_tuple->value->cstring) + 1);
  available_files[idx][1] = malloc(strlen(label_tuple->value->cstring) + 1);
  strcpy(available_files[idx][0], file_tuple->value->cstring);
  strcpy(available_files[idx][1], label_tuple->value->cstring);

  if (available_files_length == received_files_length) {
    main_window_add_menu();
  }
}

static void inbox_received_handler(DictionaryIterator *iterator, void *ctx) {
  Tuple *tuple = dict_find(iterator, filesLengthKey);

  if (tuple) {
    append_received_file(tuple->value->int8, iterator);
    return;
  } else {
    tuple = dict_read_first(iterator);
  }

  switch (tuple->key) {
    case msgSuccessKey:
    case msgErrorKey:
      main_window_add_message(tuple);
      break;
  }
}

static void inbox_dropped_handler(AppMessageResult reason, void *ctx) {
  APP_LOG(APP_LOG_LEVEL_WARNING,
          "APP_MESSAGE_INBOX_DROPPED with code %d", reason);
}

static void outbox_failed_handler(DictionaryIterator *iterator,
                                  AppMessageResult reason, void *ctx) {
  APP_LOG(APP_LOG_LEVEL_WARNING,
          "APP_MESSAGE_OUTBOX_FAILED with code %d", reason);
}

void app_message_init(void) {
  app_message_register_inbox_received(inbox_received_handler);
  app_message_register_inbox_dropped(inbox_dropped_handler);
  app_message_register_outbox_failed(outbox_failed_handler);
  AppMessageResult result = app_message_open(512, 64);

  if (result != APP_MSG_OK) {
    APP_LOG(APP_LOG_LEVEL_ERROR,
            "APP_MESSAGE_OPEN_FAILED with code %d", result);
  }
}

void app_message_deinit(void) {
  app_message_deregister_callbacks();

  if (available_files == NULL) return;
  for (int i = 0; i < available_files_length; i++) {
    if (available_files[i] == NULL) return;
    for (int j = 0; j < file_tuple_length; j++) {
      free(available_files[i][j]);
    }
    free(available_files[i]);
  }
  free(available_files);
}

void send_app_message(Tuplet tuplet) {
  DictionaryIterator *iter;
  AppMessageResult result = app_message_outbox_begin(&iter);

  if (iter == NULL) {
    APP_LOG(APP_LOG_LEVEL_ERROR,
            "APP_MESSAGE_OUTBOX_BEGIN_FAILED with code %d", result);
    return;
  }

  dict_write_tuplet(iter, &tuplet);
  dict_write_end(iter);
  app_message_outbox_send();
}
