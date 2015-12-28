#pragma once

#include <pebble.h>
#include "exec_file.h"

typedef enum {
  menuItemKey = 0,
  filesLengthKey = 1,
  fileNameKey = 2,
  fileLabelKey = 3,
  msgSuccessKey = 4,
  msgErrorKey = 5,
} appKeys;

extern char ***available_files;
extern int available_files_length;

void app_message_init(void);
void app_message_deinit(void);
void send_app_message(Tuplet tuplet);
