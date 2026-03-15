#include "wooting-rgb-sdk.h"
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef i8 b8;
typedef i32 b32;

typedef float f32;
typedef double f64;

#define ROWS 6
#define COLS 17
#define LUT_SIZE 1024

static u8 lut[LUT_SIZE][3];
static volatile int running = 1;

void handle_sigint(int _sig) { running = 0; }

static u8 lerp_u8(u8 a, u8 b, f32 t) {
  return (u8)((f32)a + ((f32)b - (f32)a) * t);
}

static void blossom_gradient(f32 t, u8 *r, u8 *g, u8 *b) {
  static const u8 colors[5][3] = {
      {180, 20, 70},   {255, 80, 130},  {255, 160, 190},
      {255, 225, 235}, {220, 130, 180},
  };

  t = t - (f32)((int)t);
  if (t < 0.0f)
    t += 1.0f;

  f32 scaled = t * 5.0f;
  int idx = (int)scaled % 5;
  int next = (idx + 1) % 5;
  f32 frac = scaled - (f32)(int)scaled;

  *r = lerp_u8(colors[idx][0], colors[next][0], frac);
  *g = lerp_u8(colors[idx][1], colors[next][1], frac);
  *b = lerp_u8(colors[idx][2], colors[next][2], frac);
}

static void build_lut(void) {
  for (int i = 0; i < LUT_SIZE; i++)
    blossom_gradient((f32)i / (f32)LUT_SIZE, &lut[i][0], &lut[i][1],
                     &lut[i][2]);
}

static long now_ms(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec * 1000L + ts.tv_nsec / 1000000L;
}

static void sleep_ms(long ms) {
  struct timespec ts = {ms / 1000, (ms % 1000) * 1000000L};
  nanosleep(&ts, NULL);
}

int main(void) {
  if (!wooting_rgb_kbd_connected()) {
    fprintf(stderr, "Wooting keyboard not found!\n");
    return 1;
  }

  signal(SIGINT, handle_sigint);
  signal(SIGTERM, handle_sigint);
  build_lut();

  printf("Cherry blossom wave starting... Ctrl+C to exit.\n");

  const f32 dir_x = 0.8f;
  const f32 dir_y = 0.3f;
  const f32 flow_dir = -1.0f;
  const f32 time_step = 0.005f;
  const long frame_target = 10;
  f32 t = 0.0f;

  while (running) {
    long frame_start = now_ms();

    for (int row = 0; row < ROWS; row++) {
      for (int col = 0; col < COLS; col++) {
        f32 pos =
            ((f32)col * dir_x + (f32)row * dir_y) / (f32)COLS + flow_dir * t;
        pos = pos - (f32)((int)pos);
        if (pos < 0.0f)
          pos += 1.0f;

        int idx = (int)(pos * (f32)LUT_SIZE) % LUT_SIZE;

        wooting_rgb_array_set_single((u8)row, (u8)col, lut[idx][0], lut[idx][1],
                                     lut[idx][2]);
      }
    }

    wooting_rgb_array_update_keyboard();

    t += time_step;
    if (t >= 1.0f)
      t -= 1.0f;

    long elapsed = now_ms() - frame_start;
    if (elapsed < frame_target)
      sleep_ms(frame_target - elapsed);
  }

  printf("\nRestoring keyboard lighting...\n");
  wooting_rgb_reset();

  return 0;
}
