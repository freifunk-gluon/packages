// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2015 Matthias Schiffer <mschiffer@universe-factory.net>


#ifndef _LIBPLATFORMINFO_H_
#define _LIBPLATFORMINFO_H_

const char * platforminfo_get_target(void);
const char * platforminfo_get_subtarget(void);
const char * platforminfo_get_board_name(void);
const char * platforminfo_get_model(void);
const char * platforminfo_get_image_name(void);

#endif /* _LIBPLATFORMINFO_H_ */
