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
#ifndef COMMON_H
#define COMMON_H

#include "libdiagexploit/diag.h"

unsigned int get_uevent_helper_address(void);
int prepare_injection_data(struct diag_values *data, size_t data_size,
                           unsigned int uevent_helper_address,
                           const char *helper_command_path);

#endif /* COMMON_H */
/*
vi:ts=2:nowrap:ai:expandtab:sw=2
*/
