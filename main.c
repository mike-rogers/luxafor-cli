#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <getopt.h>
#include "hidapi.h"

#define LUXAFOR_VENDOR_ID 0x04d8
#define LUXAFOR_PRODUCT_ID 0xf372

// command bytes from the Luxafor developer guide
#define CMD_STATIC 0x01
#define CMD_FADE 0x02
#define CMD_STROBE 0x03
#define CMD_WAVE 0x04
#define CMD_PATTERN 0x06

// LED target values: 1-6 address individual LEDs (1-3 flag side, 4-6 rear)
#define LED_FRONT 0x41
#define LED_BACK 0x42
#define LED_ALL 0xFF

// lowest channel value that produces visible light (found empirically:
// the flag shows nothing for 0x01)
#define MIN_VISIBLE_CHANNEL 0x02

typedef enum
  {
   RED,
   GREEN,
   BLUE,
   WHITE,
   BLACK
  } SAMPLE_COLOR;

typedef struct {
  SAMPLE_COLOR color;
  const char *name;
  const char *rgbValue;
} s_Color;

static s_Color colorMap[] =
  {
   { RED, "red", "0xff0000" },
   { GREEN, "green", "0x00ff00" },
   { BLUE, "blue", "0x0000ff" },
   { WHITE, "white", "0xffffff" },
   { BLACK, "black", "0x000000" },
  };

typedef enum
  {
   MODE_STATIC,
   MODE_FADE,
   MODE_STROBE,
   MODE_WAVE,
   MODE_PATTERN
  } run_mode;

#ifndef LUXAFOR_VERSION
#define LUXAFOR_VERSION "unknown"
#endif

static void print_usage(FILE *out)
{
  fprintf(out,
	  "luxafor - control a Luxafor Flag USB busylight\n"
	  "\n"
	  "Usage:\n"
	  "  luxafor [options] <color_name | hex_code>\n"
	  "  luxafor --pattern <1-8> [--repeat <n>]\n"
	  "  luxafor --help | --version\n"
	  "\n"
	  "Options:\n"
	  "  -l, --led <target>    which LEDs to set: 1-6, front, back, all (default: all)\n"
	  "                        (1-3 = flag side, 4-6 = rear side)\n"
	  "  -b, --brightness <n>  dim the color to n percent (0-100, default: 100);\n"
	  "                        scaled to how bright it looks, not raw LED power\n"
	  "  -f, --fade <0-255>    fade to the color; higher is slower\n"
	  "  -s, --strobe <0-255>  strobe the color; higher is slower\n"
	  "  -w, --wave <1-5>      wave effect with the color\n"
	  "  -S, --speed <0-255>   wave speed; higher is slower\n"
	  "  -r, --repeat <0-255>  repeat count for strobe/wave/pattern\n"
	  "  -h, --help            show this help\n"
	  "  -v, --version         show version\n"
	  "\n"
	  "Colors:\n"
	  "  ");
  for (size_t i = 0; i < sizeof(colorMap) / sizeof(colorMap[0]); i++) {
    fprintf(out, "%s%s", i ? ", " : "", colorMap[i].name);
  }
  fprintf(out,
	  "\n"
	  "\n"
	  "Examples:\n"
	  "  luxafor blue\n"
	  "  luxafor 0x043f2c\n"
	  "  luxafor --brightness 25 red\n"
	  "  luxafor --fade 60 red\n"
	  "  luxafor --led front --strobe 20 --repeat 5 green\n"
	  "  luxafor --wave 3 --speed 30 blue\n"
	  "  luxafor --pattern 5\n");
}

static unsigned char parse_byte_arg(const char *name, const char *value, long min, long max)
{
  char *end;
  long v = strtol(value, &end, 10);

  if (*value == '\0' || *end != '\0' || v < min || v > max) {
    fprintf(stderr, "* ERROR:\t%s must be a number between %ld and %ld\n", name, min, max);
    exit(EXIT_FAILURE);
  }

  return (unsigned char)v;
}

static unsigned char parse_led_arg(const char *value)
{
  if (!strcmp(value, "all")) return LED_ALL;
  if (!strcmp(value, "front")) return LED_FRONT;
  if (!strcmp(value, "back")) return LED_BACK;
  return parse_byte_arg("--led", value, 1, 6);
}

int main(int argc, char* argv[])
{
  int res;
  unsigned char buf[9] = { 0x00 }; // buf[0] is the HID report ID
  unsigned char color[3];
  hid_device *handle;

  run_mode mode = MODE_STATIC;
  int modeCount = 0;
  unsigned char led = LED_ALL;
  unsigned char fadeTime = 0;
  unsigned char strobeSpeed = 0;
  unsigned char waveType = 0;
  unsigned char patternId = 0;
  unsigned char waveSpeed = 0;
  unsigned char repeat = 0;
  unsigned char brightness = 100;

#ifdef DEBUG
  // used to print out HID device information
  struct hid_device_info *info = hid_enumerate(0, 0);

  if (info == NULL) {
    fprintf(stderr, "* ERROR:\tNo USB device information available\n");
    exit(EXIT_FAILURE);
  }

  for (struct hid_device_info *cur = info; cur != NULL; cur = cur->next) {
    printf("%#02x:%#02x: %s\n", cur->vendor_id, cur->product_id, cur->path);
  }

  hid_free_enumeration(info);
#endif

  static struct option longOpts[] =
    {
     { "led", required_argument, NULL, 'l' },
     { "brightness", required_argument, NULL, 'b' },
     { "fade", required_argument, NULL, 'f' },
     { "strobe", required_argument, NULL, 's' },
     { "wave", required_argument, NULL, 'w' },
     { "pattern", required_argument, NULL, 'p' },
     { "speed", required_argument, NULL, 'S' },
     { "repeat", required_argument, NULL, 'r' },
     { "help", no_argument, NULL, 'h' },
     { "version", no_argument, NULL, 'v' },
     { NULL, 0, NULL, 0 },
    };

  int opt;
  while ((opt = getopt_long(argc, argv, "l:b:f:s:w:p:S:r:hv", longOpts, NULL)) != -1) {
    switch (opt) {
    case 'l':
      led = parse_led_arg(optarg);
      break;
    case 'b':
      brightness = parse_byte_arg("--brightness", optarg, 0, 100);
      break;
    case 'f':
      mode = MODE_FADE;
      modeCount++;
      fadeTime = parse_byte_arg("--fade", optarg, 0, 255);
      break;
    case 's':
      mode = MODE_STROBE;
      modeCount++;
      strobeSpeed = parse_byte_arg("--strobe", optarg, 0, 255);
      break;
    case 'w':
      mode = MODE_WAVE;
      modeCount++;
      waveType = parse_byte_arg("--wave", optarg, 1, 5);
      break;
    case 'p':
      mode = MODE_PATTERN;
      modeCount++;
      patternId = parse_byte_arg("--pattern", optarg, 1, 8);
      break;
    case 'S':
      waveSpeed = parse_byte_arg("--speed", optarg, 0, 255);
      break;
    case 'r':
      repeat = parse_byte_arg("--repeat", optarg, 0, 255);
      break;
    case 'h':
      print_usage(stdout);
      return 0;
    case 'v':
      printf("luxafor %s\n", LUXAFOR_VERSION);
      return 0;
    default:
      print_usage(stderr);
      exit(EXIT_FAILURE);
    }
  }

  if (modeCount > 1) {
    fprintf(stderr, "* ERROR:\tOnly one of --fade, --strobe, --wave, or --pattern may be given.\n");
    exit(EXIT_FAILURE);
  }

  if (mode == MODE_PATTERN) {
    // patterns are built into the device and take no color
    if (optind != argc) {
      fprintf(stderr, "* ERROR:\t--pattern does not take a color argument.\n");
      exit(EXIT_FAILURE);
    }
  } else {
    if (optind != argc - 1) {
      print_usage(stderr);
      exit(EXIT_FAILURE);
    }

    const char *colorArg = argv[optind];

    res = sscanf(colorArg, "0x%02hhX%02hhX%02hhX", color, color + 1, color + 2);

    if (res != 3) {
      int found = 0;
      for (size_t i = 0; i < sizeof(colorMap) / sizeof(colorMap[0]); i++) {
	if (!strcmp(colorArg, colorMap[i].name)) {
	  found = 1;
	  sscanf(colorMap[i].rgbValue, "0x%02hhX%02hhX%02hhX", color, color + 1, color + 2);
	  break;
	}
      }

      if (found == 0) {
	fprintf(stderr, "* ERROR:\tCan't find a color by that name. See 'luxafor --help' for the list.\n");
	exit(EXIT_FAILURE);
      }
    }

    if (brightness < 100) {
      // LED output is linear in the PWM duty but perception isn't:
      // gamma-correct so the percentage tracks apparent brightness
      double factor = pow(brightness / 100.0, 2.2);
      for (int k = 0; k < 3; k++) {
	unsigned char scaled = (unsigned char)(color[k] * factor + 0.5);
	// don't let a lit channel dim below what the hardware can show;
	// only --brightness 0 means off
	if (scaled < MIN_VISIBLE_CHANNEL && color[k] > 0 && brightness > 0) {
	  scaled = MIN_VISIBLE_CHANNEL;
	}
	color[k] = scaled;
      }
    }
  }

  // report layouts from the Luxafor developer guide; buf[0] stays 0x00
  switch (mode) {
  case MODE_STATIC:
    buf[1] = CMD_STATIC;
    buf[2] = led;
    memcpy(buf + 3, color, 3);
    break;
  case MODE_FADE:
    buf[1] = CMD_FADE;
    buf[2] = led;
    memcpy(buf + 3, color, 3);
    buf[6] = fadeTime;
    break;
  case MODE_STROBE:
    buf[1] = CMD_STROBE;
    buf[2] = led;
    memcpy(buf + 3, color, 3);
    buf[6] = strobeSpeed;
    buf[8] = repeat;
    break;
  case MODE_WAVE:
    buf[1] = CMD_WAVE;
    buf[2] = waveType;
    memcpy(buf + 3, color, 3);
    buf[7] = repeat;
    buf[8] = waveSpeed;
    break;
  case MODE_PATTERN:
    buf[1] = CMD_PATTERN;
    buf[2] = patternId;
    buf[3] = repeat;
    break;
  }

  res = hid_init();

  if (res == -1) {
    fprintf(stderr, "* ERROR:\tSomething's up with HIDAPI, like it can't find HID stuff. Does your kernel support HID devices?\n");
    exit(EXIT_FAILURE);
  }

  handle = hid_open(LUXAFOR_VENDOR_ID, LUXAFOR_PRODUCT_ID, NULL);

  if (handle == NULL) {
    fprintf(stderr, "* ERROR:\tUnable to open device. Did you plug in the light?\n");
    exit(EXIT_FAILURE);
  }

  res = hid_write(handle, buf, sizeof(buf));

  if (res == -1) {
    fprintf(stderr, "* ERROR:\tUnable to write bytes to USB device for some reason.\n");
    exit(EXIT_FAILURE);
  }

  hid_close(handle);

  res = hid_exit();

  if (res == -1) {
    fprintf(stderr, "* ERROR:\tThere was an error while deallocating resources.\n");
    exit(EXIT_FAILURE);
  }

  return 0;
}
