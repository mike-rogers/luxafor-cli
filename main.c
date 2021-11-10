#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hidapi.h"

#define MAX_STR 255
#define LUXAFOR_VENDOR_ID 0x04d8
#define LUXAFOR_PRODUCT_ID 0xf372


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

int main(int argc, char* argv[])
{
  int res;
  // there are some magic bytes here that represent the 'change color' command for the luxafor flag product,
  // and i'm not entirely sure where i sourced them from.
  unsigned char buf[] = { 0x00, 0x01, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  unsigned char color[3];
  wchar_t wstr[MAX_STR];
  hid_device *handle;
  int i;

#ifdef DEBUG
  // used to print out HID device information
  struct hid_device_info *info = hid_enumerate(0, 0);

  if (info == NULL) {
    printf("* ERROR:\tNo USB device information available\n");
    exit(-1);
  }

  do {
    printf("%#02x:%#02x: %s\n", info->vendor_id, info->product_id, info->path);
    info = info->next;
  } while (info->next != NULL);

  hid_free_enumeration(info);
#endif

  if (argc != 2) {
    printf("* ERROR:\tPlease supply one argument: [ color_name | color_code ]\n");
    exit(-1);
  }

  res = sscanf(argv[1], "0x%02hhX%02hhX%02hhX", color, color + 1, color + 2);

  if (res != 3) {
    int found = 0;
    int i;
    for (i = 0; i < sizeof(colorMap) / sizeof(colorMap[0]); i++) {
      if (!strcmp(argv[1], colorMap[i].name)) {
	found = 1;
	sscanf(colorMap[i].rgbValue, "0x%02hhX%02hhX%02hhX", color, color + 1, color + 2);
	break;
      }
    }

    if (found == 0) {
      printf("* ERROR:\tCan't find a color by that name. Try a standard color.\n");
      exit(-1);
    }
  }

  res = hid_init();

  if (res == -1) {
    printf("* ERROR:\tSomething's up with HIDAPI, like it can't find HID stuff. Does your kernel support HID devices?\n");
    exit(-1);
  }

  handle = hid_open(LUXAFOR_VENDOR_ID, LUXAFOR_PRODUCT_ID, NULL);

  if (handle == NULL) {
    printf("* ERROR:\tUnable to open device. Did you plug in the light?\n");
    exit(-1);
  }

  for (int k = 0; k < 3; k++) {
    buf[3 + k] = color[k];
  }

  res = hid_write(handle, buf, sizeof(buf));

  if (res == -1) {
    printf("* ERROR:\tUnable to write bytes to USB device for some reason.\n");
    exit(-1);
  }

  hid_close(handle);

  res = hid_exit();

  if (res == -1) {
    printf("* ERROR:\tThere was an error while deallocating resources.\n");
    exit(-1);
  }

  return 0;
}
