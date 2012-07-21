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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <kernel/hal/kbd/kbd.h>

#include "input.h"

#define min(_A_,_B_) ((_A_<=_B_)?_A_:_B_)

/** Seek direction */
enum {
	seek_backward = -1,
	seek_forward  =  1
};

static void input_init(Input *);
static void input_insert_string(Input *, const char *);
static void input_key_ctrl(Input *, KBD_Event *);
static void input_key_unmod(Input *, KBD_Event *);

/** Create a new text input field. */
Input *
input_new(VT102 *vt102) {
	Input *input = (Input *)malloc(sizeof (Input));

	if (input) {
		input->vt = vt102;

		input_init(input);
	}

	return (input);
}

/** Destroy text input field. */
void
input_free(Input *input) {
	free(input);
}

static void
input_display_tail(Input *input, int start, int pad) {
	int x = (input->col0 + start) % input->con_cols;
	int y = input->row0 + (input->col0 + start) / input->con_cols;
	char dbuf[INPUT_MAX_SIZE + 1];
	int p = start;

	vt102_move_to(input->vt, x, y);

	if (p < input->nc) {
		memcpy(dbuf, input->buffer + p, input->nc - p);
		dbuf[input->nc - p] = '\0';
		vt102_puts(input->vt, dbuf, input->nc - p);
	}

	for (p = 0; p < pad; p++) vt102_putc(input->vt, ' ');
}

static char *
input_get_str(Input *input) {
	return (input->buffer);
}

static void
input_position_caret(Input *input) {
	int x = (input->col0 + input->pos) % input->con_cols;
	int y = input->row0 + (input->col0 + input->pos) / input->con_cols;

	vt102_move_to(input->vt, x, y);
}

/** Update row0 in case the screen could have scrolled. */
static void
input_update_origin(Input *input) {
	int width = input->col0 + input->nc;
	int rows = (width / input->con_cols) + 1;

	/* Update row0 if the screen scrolled. */
	if (input->row0 + rows > input->con_rows) {
		input->row0 = input->con_rows - rows;
	}
}

static void
input_insert_char(Input *input, char c) {
	if (input->nc == INPUT_MAX_SIZE) return;

	int new_width = input->col0 + input->nc + 1;
	if (new_width % input->con_cols == 0) {
		/* Advancing to new line. */
		int new_height = (new_width / input->con_cols) + 1;
		if (new_height >= input->con_rows) {
			/* Disallow text longer than 1 page for now. */
			return;
		}
	}

	for (int i = input->nc; i > input->pos; i--) {
		input->buffer[i] = input->buffer[i - 1];
	}

	input->buffer[input->pos] = c;
	input->pos += 1;
	input->nc += 1;
	input->buffer[input->nc] = '\0';

	input_display_tail(input, input->pos - 1, 0);
	input_update_origin(input);
	input_position_caret(input);
}

static void
input_insert_string(Input *input, const char *str) {
	int ilen = min(strlen(str), INPUT_MAX_SIZE - input->nc);

	if (ilen == 0) return;

	int new_width = input->col0 + input->nc + ilen;
	int new_height = (new_width / input->con_cols) + 1;

	if (new_height >= input->con_rows) {
		/* Disallow text longer than 1 page for now. */
		return;
	}

	if (input->nc > 0) {
		for (int i = input->nc; i > input->pos; i--) {
			input->buffer[i + ilen - 1] = input->buffer[i - 1];
		}
	}

	int i = 0;

	while (i < ilen) {
		char c = str[i]; //str_decode(str, &off, STR_NO_LIMIT);

		if (c == '\0') break;

		/* Filter out non-printable chars. */
		if (c < 32) c = 32;

		input->buffer[input->pos + i] = c;
		i++;
	}

	input->pos += ilen;
	input->nc += ilen;
	input->buffer[input->nc] = '\0';

	input_display_tail(input, input->pos - ilen, 0);
	input_update_origin(input);
	input_position_caret(input);
}

static void
input_backspace(Input *input) {
	if (input->pos == 0) return;

	for (int i = input->pos; i < input->nc; i++) {
		input->buffer[i - 1] = input->buffer[i];
	}

	input->pos -= 1;
	input->nc -= 1;
	input->buffer[input->nc] = '\0';

	input_display_tail(input, input->pos, 1);
	input_position_caret(input);
}

static void
input_delete(Input *input) {
	if (input->pos == input->nc) return;

	input->pos += 1;

	input_backspace(input);
}

static void
input_seek_cell(Input *input, int dir) {
	if (dir == seek_forward) {
		if (input->pos < input->nc) input->pos += 1;
	} else {
		if (input->pos > 0) input->pos -= 1;
	}

	input_position_caret(input);
}

static void
input_seek_word(Input *input, int dir) {
	if (dir == seek_forward) {
		if (input->pos == input->nc) return;

		while (1) {
			input->pos += 1;

			if (input->pos == input->nc) break;

			if ((input->buffer[input->pos - 1] == ' ') &&
			    (input->buffer[input->pos] != ' ')) break;
		}
	} else {
		if (input->pos == 0) return;

		while (1) {
			input->pos -= 1;

			if (input->pos == 0) break;

			if (input->buffer[input->pos - 1] == ' ' &&
			    input->buffer[input->pos] != ' ') break;
		}

	}

	input_position_caret(input);
}

static void
input_seek_vertical(Input *input, int dir) {
	if (dir == seek_forward) {
		if (input->pos + input->con_cols <= input->nc) {
			input->pos = input->pos + input->con_cols;
		}
	} else {
		if (input->pos >= input->con_cols) {
			input->pos = input->pos - input->con_cols;
		}
	}

	input_position_caret(input);
}

static void
input_seek_max(Input *input, int dir) {
	if (dir == seek_backward) {
		input->pos = 0;
	} else {
		input->pos = input->nc;
	}

	input_position_caret(input);
}

static void
input_history_insert(Input *input, char *str) {
	if (input->hnum < HISTORY_LEN) {
		input->hnum += 1;
	} else {
		if (input->history[HISTORY_LEN] != NULL) {
			free(input->history[HISTORY_LEN]);
		}
	}

	for (int i = input->hnum; i > 1; i--) {
		input->history[i] = input->history[i - 1];
	}
	input->history[1] = strdup(str);

	if (input->history[0] != NULL) {
		free(input->history[0]);
		input->history[0] = NULL;
	}
}

static void
input_set_str(Input *input, char *str) {
	//str_to_wstr(input->buffer, INPUT_MAX_SIZE, str);
	strncpy(input->buffer, str, INPUT_MAX_SIZE);
	//input->nc = wstr_length(input->buffer);
	input->nc = strlen(input->buffer);
	input->pos = input->nc;
}

static void
input_history_seek(Input *input, int offs) {
	if (offs >= 0) {
		if (input->hpos + offs > input->hnum) return;
	} else {
		if (input->hpos < (int)-offs) return;
	}

	if (input->history[input->hpos] != NULL) {
		free(input->history[input->hpos]);
		input->history[input->hpos] = NULL;
	}

	input->history[input->hpos] = input_get_str(input);
	input->hpos += offs;

	int pad = (int)input->nc - strlen(input->history[input->hpos]);
	if (pad < 0) pad = 0;

	input_set_str(input, input->history[input->hpos]);
	input_display_tail(input, 0, pad);
	input_update_origin(input);
	input_position_caret(input);
}

static void
input_update_dimensions(Input *input) {
	input->con_cols = vt102_width(input->vt);
	input->con_rows = vt102_height(input->vt);

	input->col0 = vt102_cursor_x(input->vt);
	input->row0 = vt102_cursor_y(input->vt);
}

static void
input_init(Input *input) {
	input->hpos = 0;
	input->hnum = 0;
	input->history[0] = NULL;

	input_update_dimensions(input);
}

int
input_key_event(Input *input, KBD_Event *ev) {
	if (((ev->modifier & KBD_MOD_CTRL) != 0) &&
	    ((ev->modifier & (KBD_MOD_ALT | KBD_MOD_SHIFT)) == 0))
		input_key_ctrl(input, ev);

	if ((ev->modifier & (KBD_MOD_CTRL | KBD_MOD_ALT | KBD_MOD_SHIFT)) == 0)
		input_key_unmod(input, ev);

	if (ev->unicode >= ' ') {
		input_insert_char(input, ev->unicode);
	}

	return (0);
}

void
input_read_line(Input *input, char **dstr) {
	char *str = input_get_str(input);

	// add to history
	if (strcmp(str, "") != 0) {
		input_history_insert(input, str);
	}

	*dstr = str;
}

void
input_new_line(Input *input, const char *prompt) {
	vt102_puts(input->vt, prompt, 0);

	input_update_dimensions(input);

	input->buffer[0] = '\0';
	input->pos = 0;
	input->nc = 0;
}

static void
input_key_ctrl(Input *input, KBD_Event *ev) {
	switch (ev->symbol) {
		case KBD_KEY_LEFT:
			input_seek_word(input, seek_backward);
		break;

		case KBD_KEY_RIGHT:
			input_seek_word(input, seek_forward);
		break;

		case KBD_KEY_UP:
			input_seek_vertical(input, seek_backward);
		break;

		case KBD_KEY_DOWN:
			input_seek_vertical(input, seek_forward);
		break;
	}
}

static void
input_key_unmod(Input *input, KBD_Event *ev) {
	switch (ev->symbol) {
		case KBD_KEY_BACKSPACE:
			input_backspace(input);
		break;

		case KBD_KEY_DELETE:
			input_delete(input);
		break;

		case KBD_KEY_LEFT:
			input_seek_cell(input, seek_backward);
		break;

		case KBD_KEY_RIGHT:
			input_seek_cell(input, seek_forward);
		break;

		case KBD_KEY_HOME:
			input_seek_max(input, seek_backward);
		break;

		case KBD_KEY_END:
			input_seek_max(input, seek_forward);
		break;

		case KBD_KEY_UP:
			input_history_seek(input, 1);
		break;

		case KBD_KEY_DOWN:
			input_history_seek(input, -1);
		break;
	}
}
