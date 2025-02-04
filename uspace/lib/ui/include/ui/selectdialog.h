/*
 * Copyright (c) 2023 Jiri Svoboda
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - The name of the author may not be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/** @addtogroup libui
 * @{
 */
/**
 * @file Select dialog
 */

#ifndef _UI_SELECTDIALOG_H
#define _UI_SELECTDIALOG_H

#include <errno.h>
#include <types/ui/list.h>
#include <types/ui/selectdialog.h>
#include <types/ui/ui.h>

extern void ui_select_dialog_params_init(ui_select_dialog_params_t *);
extern errno_t ui_select_dialog_create(ui_t *, ui_select_dialog_params_t *,
    ui_select_dialog_t **);
extern void ui_select_dialog_set_cb(ui_select_dialog_t *, ui_select_dialog_cb_t *,
    void *);
extern void ui_select_dialog_destroy(ui_select_dialog_t *);
extern errno_t ui_select_dialog_append(ui_select_dialog_t *,
    ui_list_entry_attr_t *);
extern errno_t ui_select_dialog_paint(ui_select_dialog_t *);

#endif

/** @}
 */
