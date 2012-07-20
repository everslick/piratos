/*
 * Copyright (c) 2010 Jiri Svoboda
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

/*
 * ported from HelenOS (c) 2012 Clemens Kirchgatterer <clemens@1541.org>
 */

#ifndef _INPUT_H_
#define _INPUT_H_

#include "vt102.h"

#define HISTORY_LEN     20
#define INPUT_MAX_SIZE  1024

/** Text input field (command line).
 *
 * Applications should treat this structure as opaque.
 */
typedef struct {
	/** Buffer holding text currently being edited */
	char buffer[INPUT_MAX_SIZE + 1];

	/** Screen coordinates of the top-left corner of the text field */
	int col0;
	int row0;

	/** Screen dimensions */
	int con_cols;
	int con_rows;

	/** Number of characters in @c buffer */
	int nc;

	/** Caret position within buffer */
	int pos;

	/** History (dynamically allocated strings) */
	char *history[HISTORY_LEN + 1];

	/** Number of entries in @c history, not counting [0] */
	int hnum;

	/** Current position in history */
	int hpos;

	VT102 *vt;
} Input;

Input *input_new(VT102 *vt102);
void   input_destroy(Input *input);

void   input_new_line(Input *input, const char *prompt);
void   input_read_line(Input *input, char **dstr);

int    input_key_event(Input *input, KBD_Event *ev);

#endif // _INPUT_H_
