/* Capture the real transcription pipeline without audio synthesis or NVDA. */
#include <string.h>
#include "transcription.h"
#include "synth.h"

typedef struct { ttscb_t cb; unsigned char *output; size_t capacity, length; int cancel; } capture_t;
static int collect(void *buffer, size_t size, void *user_data) {
  capture_t *capture = user_data;
  if (capture->length + size > capture->capacity) return 1;
  memcpy(capture->output + capture->length, buffer, size);
  capture->length += size;
  return capture->cancel;
}
NEWFON_EXPORT size_t transcribe(const char *text, int flags, unsigned char *output, size_t capacity, int cancel) {
  unsigned char buffer[TRANSCRIPTION_BUFFER_SIZE];
  capture_t capture = {0};
  sink_t sink;
  capture.cb.flags = flags;
  capture.output = output;
  capture.capacity = capacity;
  capture.cancel = cancel;
  sink_setup(&sink, buffer, TRANSCRIPTION_MAXLEN, collect, &capture);
  process_text(text, &sink);
  return capture.length;
}
