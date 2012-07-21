/*
 * ported from VDR console plugin (C) Jan Rieger <jan@ricomp.de>
 * by Clemens Kirchgatterer <clemens@1541.org>
 *
 */

#include <stdlib.h>

#include <lib/sys/log.h>

#include "vt102.h"

#define min(A, B) ((A <= B) ? A : B)
#define max(A, B) ((A >= B) ? A : B)

static void Clear(VT102 *vt102, int x1, int y1, int x2, int y2); // 0, 0, -1, -1

static void SetSize(VT102 *vt102, int w, int h);
static void MoveTo(VT102 *vt102, int col, int row);
static void MoveRelative(VT102 *vt102, int col, int row);

static void ScrollUp(VT102 *vt102, int count);   // 1
static void ScrollDown(VT102 *vt102, int count); // 1
 
static void ScrollUpFrom(VT102 *vt102, int count, int start);
static void ScrollDownFrom(VT102 *vt102, int count, int start);

static void DecodeEscapeSequence(VT102 *vt102, char code);
static void DecodeEscapeCode(VT102 *vt102, char code);
static void DecodeEscapeSingleCode(VT102 *vt102, char code);

static void TabStopClear(VT102 *vt102);
static void TabStopAdd(VT102 *vt102, int tabstop);
static void TabStopRemove(VT102 *vt102, int tabstop);

static void KeyCarriageReturn(VT102 *vt102);
static void KeyLineFeed(VT102 *vt102, int nl);
static void KeyBackspace(VT102 *vt102);
static void KeyDelete(VT102 *vt102);
static void KeyTab(VT102 *vt102);
static void KeyInsert(VT102 *vt102, char ch); // ' '
static void KeyBell(VT102 *vt102);

static void SetAttributes(VT102 *vt102, int *attributes, int count);

static void SetModes(VT102 *vt102, int *attributes, int count);
static void ResetModes(VT102 *vt102, int *attributes, int count);

static void SetCursorVisible(VT102 *vt102, int visible);
static void SetModeOrigin(VT102 *vt102, int origin);

static void ClearToEnd(VT102 *vt102);
static void ClearFromBegin(VT102 *vt102);
static void ClearLine(VT102 *vt102);

static void InsertChar(VT102 *vt102, int count);
static void DeleteChar(VT102 *vt102, int count);
static void InsertLine(VT102 *vt102, int count);
static void DeleteLine(VT102 *vt102, int count);

static void ReportDeviceStatus(VT102 *vt102, int request);
static void ReportDeviceAttributes(VT102 *vt102, int request);

static void SetScrollRegion(VT102 *vt102, int top, int bottom);

static void CursorPosSave(VT102 *vt102);
static void CursorPosRestore(VT102 *vt102);

static void SelectCharSet(VT102 *vt102, int g, char set);

static struct VT102_CursorPos *
SaveCursorPos(int x, int y, struct VT102_CanvasChar *attr, int orig, struct VT102_CursorPos *pos) {
	struct VT102_CursorPos *cp = (struct VT102_CursorPos *)malloc(sizeof (struct VT102_CursorPos));

	cp->posx = x;
	cp->posy = y;

	cp->attributes = *attr;
	cp->mode_origin = orig;

	cp->prev = pos;

	return (cp);
}

static void
ResetCanvasChar(struct VT102_CanvasChar *cc) {
	cc->ch = ' ';

	cc->foreground = 7;
	cc->background = 1;

	cc->bold       = 0;
	cc->underscore = 0;
	cc->blink      = 0;
	cc->inverted   = 0;
	cc->concealed  = 0;
	cc->use_fg     = 0;
	cc->use_bg     = 0;
}

static void
SetCanvasChar(struct VT102_CanvasChar *cc, char c, struct VT102_CanvasChar *attr) {
	cc->ch = c;

	cc->foreground = attr->foreground;
	cc->background = attr->background;

	cc->bold       = attr->bold;
	cc->underscore = attr->underscore;
	cc->blink      = attr->blink;
	cc->inverted   = attr->inverted;
	cc->concealed  = attr->concealed;
	cc->use_fg     = attr->use_fg;
	cc->use_bg     = attr->use_bg;
}

VT102 *
vt102_new(void) {
	VT102 *vt102 = (VT102 *)malloc(sizeof (VT102));

	vt102->width    = vt102->height   = 0;
	vt102->cursor_x = vt102->cursor_y = 0;

	vt102->mode.cursor_visible = 1;	// TODO load default values
	vt102->mode.wrap_around    = 1;
	vt102->mode.new_line       = 1;
	vt102->mode.origin         = 0;
	vt102->mode.insert         = 0;

	vt102->saved_cursor_pos = NULL;

	// escape sequence parser state machine
	vt102->state = VT102_STATE_NORMAL;

	vt102->changed = 0;
	vt102->bell    = 0;

	vt102->escape_params = NULL;
	vt102->escape_params_count = 0;

	// initialize all possibilities
	for (int i=0; i<CONSOLE_MAXROWS; i++) {
		vt102->canvas[i] = 0;
	}

	// initialize tabulators
	int tab = 0;
	for (int i=0; i<CONSOLE_MAXCOLS; i++) {
		vt102->tabs[i] = tab += 8;
	}

	ResetCanvasChar(&vt102->default_char);

	// No idea if this is the right size but it is
	// a good value to start.
	SetSize(vt102, 80, 24);
	SetScrollRegion(vt102, 0, 24);

	SelectCharSet(vt102, 0, 'B');

	return (vt102);
}

void
vt102_free(VT102 *vt102) {
	// releasing all allocated buffers
	for (int i=0; i<vt102->height; i++) {
		free(vt102->canvas[i]);
	}

	while (vt102->saved_cursor_pos) {
		struct VT102_CursorPos *prev = vt102->saved_cursor_pos->prev;

		free(vt102->saved_cursor_pos);

		vt102->saved_cursor_pos = prev;
	}
}

struct VT102_CanvasChar **
vt102_canvas(VT102 *vt102) {
	return (vt102->canvas);
}

int
vt102_width(VT102 *vt102) {
	return (vt102->width);
}

int
vt102_height(VT102 *vt102) {
	return (vt102->height);
}
	
int
vt102_cursor_x(VT102 *vt102) {
	return (vt102->cursor_x);
}

int
vt102_cursor_y(VT102 *vt102) {
	return (vt102->mode.origin ? vt102->cursor_y - vt102->scroll_region_top : vt102->cursor_y);
}

int
vt102_cursor_visible(VT102 *vt102) {
	return (vt102->mode.cursor_visible);
}

int
vt102_to_refresh(VT102 *vt102) {
	return (vt102->changed);
}

void
vt102_refreshed(VT102 *vt102) {
	vt102->changed = 0;
}

int
vt102_to_ring(VT102 *vt102) {
	return (vt102->bell);
}

void
vt102_bell_seen(VT102 *vt102) {
	vt102->bell = 0;
}

void
vt102_changed(VT102 *vt102) {
	vt102->changed = 1;
}

void
vt102_move_to(VT102 *vt102, int col, int row) {
	MoveTo(vt102, col, row);
}

void
vt102_set_size(VT102 *vt102, int w, int h) {
	SetSize(vt102, w, h);
}

void
ScrollUp(VT102 *vt102, int count) {
	ScrollUpFrom(vt102, count, vt102->scroll_region_top);
}

void
ScrollDown(VT102 *vt102, int count) {
	ScrollDownFrom(vt102, count, vt102->scroll_region_top);
}
	
void
SelectCharSet(VT102 *vt102, int g, char set) {
	// First reset all chars
	for (int i=0; i<=255; i++) vt102->char_set[i] = i;

	if (set == 'A' || set == 'B') {
		// Mapping the pseudo graphics to special values in the lower 32 chars.
		vt102->char_set[ 179 ] = 0x19; // vertical line
		vt102->char_set[ 180 ] = 0x16; // right tee
		vt102->char_set[ 181 ] = 0x16; // right tee
		vt102->char_set[ 182 ] = 0x16; // right tee
		vt102->char_set[ 183 ] = 0xC;  // upper_right_corner
		vt102->char_set[ 184 ] = 0xC;  // upper_right_corner
		vt102->char_set[ 185 ] = 0x16; // right tee
		vt102->char_set[ 186 ] = 0x19; // vertical line
		vt102->char_set[ 187 ] = 0xC;  // upper_right_corner
		vt102->char_set[ 188 ] = 0xB;  // lower_right_corner
		vt102->char_set[ 189 ] = 0xB;  // lower_right_corner
		vt102->char_set[ 190 ] = 0xB;  // lower_right_corner
		vt102->char_set[ 191 ] = 0xC;  // upper_right_corner
		vt102->char_set[ 192 ] = 0xE;  // lower_left_corner
		vt102->char_set[ 193 ] = 0x17; // bottom tee
		vt102->char_set[ 194 ] = 0x18; // top tee
		vt102->char_set[ 195 ] = 0x15; // left tee
		vt102->char_set[ 196 ] = 0x12; // horizontal line / scan line 7
		vt102->char_set[ 197 ] = 0xF;  // cross
		vt102->char_set[ 198 ] = 0x15; // left tee
		vt102->char_set[ 199 ] = 0x15; // left tee
		vt102->char_set[ 200 ] = 0xE;  // lower_left_corner
		vt102->char_set[ 201 ] = 0xD;  // upper_left_corner
		vt102->char_set[ 202 ] = 0x17; // bottom tee
		vt102->char_set[ 203 ] = 0x18; // top tee
		vt102->char_set[ 204 ] = 0x15; // left tee
		vt102->char_set[ 205 ] = 0x12; // horizontal line / scan line 7
		vt102->char_set[ 206 ] = 0xF;  // cross
		vt102->char_set[ 207 ] = 0x17; // bottom tee
		vt102->char_set[ 208 ] = 0x17; // bottom tee
		vt102->char_set[ 209 ] = 0x18; // top tee
		vt102->char_set[ 210 ] = 0x18; // top tee
		vt102->char_set[ 211 ] = 0xE;  // lower_left_corner
		vt102->char_set[ 212 ] = 0xE;  // lower_left_corner
		vt102->char_set[ 213 ] = 0xD;  // upper_left_corner
		vt102->char_set[ 214 ] = 0xD;  // upper_left_corner
		vt102->char_set[ 215 ] = 0xF;  // cross
		vt102->char_set[ 216 ] = 0xF;  // cross
		vt102->char_set[ 217 ] = 0xB;  // lower_right_corner
		vt102->char_set[ 218 ] = 0xD;  // upper_left_corner
		// 219..240 missing :-(
		vt102->char_set[ 241 ] =	32;   // plus minus
		vt102->char_set[ 242 ] = 0x1B; // greater than or equal
		vt102->char_set[ 243 ] = 0x1A; // less than or equal
		// 244..255 missing :-(
	} else if (g == 1 && set == '0') {
		// Mapping to the lower 32 chars
		vt102->char_set[  96 ] = 0x1;  // diamond
		vt102->char_set[  97 ] =	32;   // 50% grid
		vt102->char_set[  98 ] =	32;
		vt102->char_set[  99 ] =	32;
		vt102->char_set[ 100 ] =	32;
		vt102->char_set[ 101 ] =	32;
		vt102->char_set[ 102 ] =	32;   // degree
		vt102->char_set[ 103 ] =	32;   // plus minus
		vt102->char_set[ 104 ] =	32;
		vt102->char_set[ 105 ] =	32;
		vt102->char_set[ 106 ] = 0xB;  // lower_right_corner
		vt102->char_set[ 107 ] = 0xC;  // upper_right_corner
		vt102->char_set[ 108 ] = 0xD;  // upper_left_corner
		vt102->char_set[ 109 ] = 0xE;  // lower_left_corner
		vt102->char_set[ 110 ] = 0xF;  // cross
		vt102->char_set[ 111 ] = 0x10; // scan line 1
		vt102->char_set[ 112 ] = 0x11; // scan line 3
		vt102->char_set[ 113 ] = 0x12; // horizontal line / scan line 5
		vt102->char_set[ 114 ] = 0x13; // scan line 7 / scan line 9
		vt102->char_set[ 115 ] = 0x14; // scan line 9 / horizontal line
		vt102->char_set[ 116 ] = 0x15; // left tee
		vt102->char_set[ 117 ] = 0x16; // right tee
		vt102->char_set[ 118 ] = 0x17; // bottom tee
		vt102->char_set[ 119 ] = 0x18; // top tee
		vt102->char_set[ 120 ] = 0x19; // vertical line
		vt102->char_set[ 121 ] = 0x1A; // less than or equal
		vt102->char_set[ 122 ] = 0x1B; // greater than or equal
		vt102->char_set[ 123 ] =	32;   // pi
		vt102->char_set[ 124 ] =	32;   // not equal
		vt102->char_set[ 125 ] =	32;   // british pound
		vt102->char_set[ 126 ] =	32;   // dot
	}
}

void
SetSize(VT102 *vt102, int w, int h) {
	// if nothing is to do then exit
	if (w == vt102->width && h == vt102->height) return;

	// if the width is the same as before then we don't need to
	// realloc rows that exist already and should exist afterwards. 
	int RowsNoChangeNeeded = min(h, vt102->height);

	if (w != vt102->width) {
		for (int row=0; row<RowsNoChangeNeeded; row++) {
			struct VT102_CanvasChar *cvs_row = vt102->canvas[row];

			vt102->canvas[row] = realloc(cvs_row, w * sizeof (struct VT102_CanvasChar));

			// initialize new cols if any
			for (int col=vt102->width; col<w; col++) {
				vt102->canvas[row][col] = vt102->default_char;
			}
		}
	}

	// allocating new rows if any
	for (int row=RowsNoChangeNeeded; row<h; row++) {
		struct VT102_CanvasChar *cvs_row = vt102->canvas[row];

		vt102->canvas[row] = realloc(cvs_row, w * sizeof (struct VT102_CanvasChar));
		
		// initialize new cols
		for (int col=0; col<w; col++) {
			vt102->canvas[row][col] = vt102->default_char;
		}
	}

	// releasing old rows if any
	for (int row=h; row<vt102->height; row++) {
		free(vt102->canvas[row]);
		vt102->canvas[row] = 0;
	}

	// change scroll region
	if ((vt102->scroll_region_top == 0) && (vt102->scroll_region_bottom == vt102->height - 1)) {
		vt102->scroll_region_bottom = h - 1;
		syslog(SYS_LOG_DEBUG, "VT102->SetSize: upadting scroll region bottom to %i\n",
			vt102->scroll_region_bottom);
	}

	// save new size
	vt102->width = w;
	vt102->height = h;

	vt102_changed(vt102);

	syslog(SYS_LOG_DEBUG, "VT102->SetSize: %i, %i\n", vt102->width, vt102->height);

	return;
}

void
Clear(VT102 *vt102, int x1, int y1, int x2, int y2) {
	// check the range
	if (x1 < 0) {
		x1 = 0;
	} else if (x1 >= vt102->width) {
		x1 = vt102->width - 1;
	}

	if (y1 < 0) {
		y1 = 0;
	} else if (y1 >= vt102->height) {
		y1 = vt102->height - 1;
	}

	if (x2 < 0 || x2 >= vt102->width) x2 = vt102->width - 1;
	if (y2 < 0 || y2 >= vt102->height) y2 = vt102->height - 1;

	for (int row=y1; row<=y2; row++) {
		struct VT102_CanvasChar* cvs_row = vt102->canvas[row];

		for (int col=x1; col<=x2; col++) {
			cvs_row[col] = vt102->default_char;
		}
	}

	vt102_changed(vt102);
}

void
ScrollUpFrom(VT102 *vt102, int count, int start) {
	// scroll only if we are in the scrolling range
	if (vt102->cursor_y >= vt102->scroll_region_top && vt102->cursor_y <= vt102->scroll_region_bottom) {
		for (int i=0; i<count; i++) {
			free(vt102->canvas[start]);

			for (int row=start; row<=vt102->scroll_region_bottom-1; row++) {
				vt102->canvas[row] = vt102->canvas[row + 1];
			}

			struct VT102_CanvasChar *cvs_row = vt102->canvas[vt102->scroll_region_bottom];
			cvs_row = (struct VT102_CanvasChar *)malloc(vt102->width * sizeof (struct VT102_CanvasChar));

			for (int col=0; col<vt102->width; col++) {
				cvs_row[col] = vt102->default_char;
			}
		}

		vt102_changed(vt102);
	}
}

void
ScrollDownFrom(VT102 *vt102, int count, int start) {
	// scroll only if we are in the scrolling range
	if (vt102->cursor_y >= vt102->scroll_region_top && vt102->cursor_y <= vt102->scroll_region_bottom) {
		for (int i=0; i<count; i++) {
			free(vt102->canvas[vt102->scroll_region_bottom]);

			for (int row=vt102->scroll_region_bottom-1; row>=start; row--) {
				vt102->canvas[row + 1] = vt102->canvas[row];
			}

			struct VT102_CanvasChar *cvs_row = vt102->canvas[start];
			cvs_row = (struct VT102_CanvasChar *)malloc(vt102->width * sizeof (struct VT102_CanvasChar));
	
			for (int col=0; col<vt102->width; col++) {
				cvs_row[col] = vt102->default_char;
			}
		}

		vt102_changed(vt102);
	}
}

void
vt102_puts(VT102 *vt102, const char *stream, int len) {
	if (len) {
		for (int i=0; i<len; i++) {
			vt102_putc(vt102, stream[i]);
		}
	} else {
		while (*stream) {
			vt102_putc(vt102, *stream);
			stream++;
		}
	}
}

void
vt102_putc(VT102 *vt102, char ch) {
	if (ch == SI) SelectCharSet(vt102, 0, '0');
	if (ch == SO) SelectCharSet(vt102, 1, '0');

	// search for escape sequences
	switch (vt102->state) {
		case VT102_STATE_NORMAL:
			switch (ch) {
				// interpret control chars
				case ESC: vt102->state = VT102_STATE_ESCAPE; break; // begin of escape sequence
				case  CR: KeyCarriageReturn(vt102);  break;
				case  LF:
				case  FF:
				case  VT: KeyLineFeed(vt102, 1);     break;
				case  HT: KeyTab(vt102);             break;
				case  BS:
				case DEL: KeyBackspace(vt102);       break;
				case BEL: KeyBell(vt102);            break; // 7

				// display all other chars
				default: KeyInsert(vt102, vt102->char_set[ch]);
			}
		break;

		case VT102_STATE_ESCAPE:
			if (ch == '[') {
				// ok, the escape sequence begins here
				vt102->state = VT102_STATE_ESCAPE_PARAMETER;
			} else if (ch == '#') {
				vt102->state = VT102_STATE_ESCAPE_SINGLE_CODE;
			} else if (ch == '(') {
				vt102->state = VT102_STATE_SELECT_CHARSET_G0;
			} else if (ch == ')') {
				vt102->state = VT102_STATE_SELECT_CHARSET_G1;
			} else {
				DecodeEscapeCode(vt102, ch);
				// and back to normal mode
				vt102->state = VT102_STATE_NORMAL;
			}
		break;

		case  VT102_STATE_ESCAPE_PARAMETER:
			switch (ch) {
				case '0' ... '9':
					if (vt102->escape_params == 0) {
						// for the first parameter we must allocate memory
						vt102->escape_params = (int *)malloc(sizeof (int));
						*vt102->escape_params = ch - '0';
						vt102->escape_params_count = 1;
					} else {
						// assemble figures to a number
						int *param = &vt102->escape_params[vt102->escape_params_count - 1];
						*param = *param * 10 + (ch - '0');
					}
				break;

				case ';':
					// here a new parameter follows
					vt102->escape_params =
						(int *)realloc(vt102->escape_params, ++vt102->escape_params_count * sizeof (int));
					vt102->escape_params[vt102->escape_params_count - 1] = 0;
				break;

				case '?': break; // ignore the questionmark

				case CAN:
				case SUB:
					// cancel the sequence
					free(vt102->escape_params);
					vt102->escape_params = NULL;
					vt102->escape_params_count = 0;
					vt102->state = VT102_STATE_NORMAL;
				break;

				default:
					// end of sequence reached
					DecodeEscapeSequence(vt102, ch);
					free(vt102->escape_params);
					vt102->escape_params = NULL;
					vt102->escape_params_count = 0;
					vt102->state = VT102_STATE_NORMAL;
				break;
			}
		break;

		case VT102_STATE_ESCAPE_SINGLE_CODE:
			DecodeEscapeSingleCode(vt102, ch);
			vt102->state = VT102_STATE_NORMAL;
		break;

		case VT102_STATE_SELECT_CHARSET_G0:
		case VT102_STATE_SELECT_CHARSET_G1:
			SelectCharSet(vt102, vt102->state - VT102_STATE_SELECT_CHARSET_G0, ch);
			vt102->state = VT102_STATE_NORMAL;
		break;
	}
}

#define CONSOLE_PARAM(_index, _default) \
	((_index < vt102->escape_params_count) ? vt102->escape_params[_index] : _default)

void
DecodeEscapeSequence(VT102 *vt102, char code) {
	switch (code) {
		case 'G':
			// move to column
			MoveTo(vt102, CONSOLE_PARAM(0, 1) - 1, vt102->cursor_y);
		break;
		case 'd':
			// move to line
			MoveTo(vt102, vt102->cursor_x, CONSOLE_PARAM(0, 1) - 1);
		break;
		case 'f':
		case 'H':
			// move to position
			MoveTo(vt102, CONSOLE_PARAM(1, 1) - 1, CONSOLE_PARAM(0, 1) - 1);
		break;
		case 'e':
		case 'A':
			// n rows up
			MoveRelative(vt102, 0, - CONSOLE_PARAM(0, 1));
		break;
		case 'B':
			// n rows down
			MoveRelative(vt102, 0, CONSOLE_PARAM(0, 1));
		break;
		case 'a':
		case 'C':
			// n cols right
			MoveRelative(vt102, CONSOLE_PARAM(0, 1), 0);
		break;
		case 'D':
			// n cols left
			MoveRelative(vt102, - CONSOLE_PARAM(0, 1), 0);
		break;
		case 'E':
			// n rows down, first col
			MoveRelative(vt102, -vt102->cursor_x, CONSOLE_PARAM(0, 1));
		break;
		case 'F':
			// n rows up, first col
			MoveRelative(vt102, -vt102->cursor_x, - CONSOLE_PARAM(0, 1));
		break;

		case 'K':
			if (vt102->escape_params_count == 0 || vt102->escape_params[0] == 0) {
				// clear to end of line
				ClearToEnd(vt102);
			} else if (vt102->escape_params[0] == 1) {
				// clear from begin of line
				ClearFromBegin(vt102);
			} else if (vt102->escape_params[0] == 2) {
				// clear the hole line
				ClearLine(vt102);
			}
		break;

		case 'J':
			if (vt102->escape_params_count == 0 || vt102->escape_params[0] == 0) {
				// clear to end of screen
				ClearToEnd(vt102);
				if (vt102->mode.origin) {
					Clear(vt102, 0, vt102->cursor_y + 1, vt102->width - 1, vt102->scroll_region_bottom);
				} else {
					Clear(vt102, 0, vt102->cursor_y + 1, vt102->width - 1, vt102->height - 1);
				}
			} else if (vt102->escape_params[0] == 1) {
				// clear from begin of screen
				if (vt102->mode.origin) {
					Clear(vt102, 0, vt102->scroll_region_top, vt102->width - 1, vt102->cursor_y - 1);
				} else {
					Clear(vt102, 0, 0, vt102->width - 1, vt102->cursor_y - 1);
				}
				ClearFromBegin(vt102);
			} else if (vt102->escape_params[0] == 2) {
				// clear the hole screen
				if (vt102->mode.origin) {
					Clear(vt102, 0, vt102->scroll_region_top, vt102->width - 1, vt102->scroll_region_bottom);
				} else {
					Clear(vt102, 0, 0, -1,-1);
				}
			}
			MoveTo(vt102, 0, 0);
		break;

		case 'n': ReportDeviceStatus(vt102, CONSOLE_PARAM(0, 0));     break;
		case 'c': ReportDeviceAttributes(vt102, CONSOLE_PARAM(0, 0)); break;

		// set display attributes
		case 'm': SetAttributes(vt102, vt102->escape_params, vt102->escape_params_count); break;
		// put in screen mode
		case 'h': SetModes(vt102, vt102->escape_params, vt102->escape_params_count);      break;
		// resets Mode
		case 'l': ResetModes(vt102, vt102->escape_params, vt102->escape_params_count);    break;

		case 'r':
			SetScrollRegion(vt102, 
				CONSOLE_PARAM(0, 1) - 1, CONSOLE_PARAM(1, CONSOLE_MAXROWS) - 1
			);
		break;

		case 's': CursorPosSave(vt102);    break; // saves cursor position
		case 'u': CursorPosRestore(vt102); break; // return to saved cursor position

		case 'g':
			if (vt102->escape_params_count == 0 || vt102->escape_params[0] == 0) {
 				// clear tab at position
				TabStopRemove(vt102, vt102->cursor_y);
			} else if (vt102->escape_params[0] == 3) {
				// clear all tabs
				TabStopClear(vt102);
			}
		break;

		case 'W':
			if (vt102->escape_params_count == 0 || vt102->escape_params[0] == 0) {
				// add tab at position
				TabStopAdd(vt102, vt102->cursor_y);
			} else if (vt102->escape_params[0] == 2) {
				// clear tab at position
				TabStopRemove(vt102, vt102->cursor_y);
			} else if (vt102->escape_params[0] == 5) {
				// clear all tabs
				TabStopClear(vt102);
			}
		break;

		case '@': InsertChar(vt102, CONSOLE_PARAM(0, 1));                      break;
		case 'X':
		case 'P': DeleteChar(vt102, CONSOLE_PARAM(0, 1));                      break;
		case 'L': ScrollDownFrom(vt102, CONSOLE_PARAM(0, 1), vt102->cursor_y); break;
		case 'M': ScrollUpFrom(vt102, CONSOLE_PARAM(0, 1), vt102->cursor_y);   break;

		// ignore unsupported
		case 'y': break; // self test
		case 'q': break; // show lamp on keyboard (1..4, 0 = off)

		default:
			syslog(SYS_LOG_WARN, "unknown escape sequence %i\n", code);
		break;
	}
}

void
DecodeEscapeCode(VT102 *vt102, char code) {
	switch (code) {
		case 'D': KeyLineFeed(vt102, 0);                                    break;
		case 'E': KeyLineFeed(vt102, 0); MoveTo(vt102, 0, vt102->cursor_y); break;

		case 'M':
			if (vt102->cursor_y == vt102->scroll_region_top) {
				ScrollDownFrom(vt102, 1, 1);
			} else if (vt102->cursor_y > 0) {
				vt102->cursor_y--;
				vt102_changed(vt102);
			}
		break;

		// discard subsequent char
		case '@':
			if (vt102->cursor_x < vt102->width) {
				vt102->cursor_x++;
				DeleteChar(vt102, 1);
				vt102->cursor_x--;
			}
		break;

		case 'H': TabStopAdd(vt102, vt102->cursor_x); break; // set column tab
		case 'Z': ReportDeviceAttributes(vt102, 0);   break;
		case '7': CursorPosSave(vt102);               break;
		case '8': CursorPosRestore(vt102);            break;

		// ignore unsupported
		case '=': break; // application keypad
		case '>': break; // numeric keypad
		case 'c': break; // reset terminal

		default: syslog(SYS_LOG_WARN, "unknown escape code %i\n", code);
	}
}

void
DecodeEscapeSingleCode(VT102 *vt102, char code) {
	switch (code) {
		// ignore unsupported
		case '3': break; // double height/width
		case '4': break;
		case '5': break; // single width line
		case '6': break;
		case '8': break; // show test pattern

		default: syslog(SYS_LOG_WARN, "unknown escape single code %i\n", code);
	}
}

void
TabStopClear(VT102 *vt102) {
	for (int i=0; i<CONSOLE_MAXCOLS; i++) {
		vt102->tabs[i] = 0;
	}
}

void
TabStopAdd(VT102 *vt102, int tabstop) {
	// search for right position in tab stop array
	for (int i=0; i<CONSOLE_MAXCOLS; i++) {
		if (vt102->tabs[i] == tabstop) {
			// the tab stop is already here

			break;
		} else if (vt102->tabs[i] == 0) {
			// append on the end
			vt102->tabs[i] = tabstop;

			break;
		} else if (tabstop < vt102->tabs[i]) {
			//found the right place to insert
			for (int j=CONSOLE_MAXROWS-2; j>=i; j--) {
				vt102->tabs[j + 1] = vt102->tabs[j];
			}

			vt102->tabs[i] = tabstop;

			break;
		}
	}
}

void
TabStopRemove(VT102 *vt102, int tabstop) {
	// search the position of the tab stop
	for (int i=0; i<CONSOLE_MAXCOLS; i++) {
		if (vt102->tabs[i] == tabstop) {
			int j;

			for (j=i; j<CONSOLE_MAXCOLS-1; j++) {
				vt102->tabs[j] = vt102->tabs[j + 1];
			}

			if (j == CONSOLE_MAXCOLS - 1) {
				vt102->tabs[j] = 0;
			}

			break;
		}
	}
}

void
MoveTo(VT102 *vt102, int col, int row) {
	int y, x = min(max(col, 0), vt102->width - 1);

	if (vt102->mode.origin) {
		y = min(max(row, vt102->scroll_region_top), vt102->scroll_region_bottom);
	} else {
		y = min(max(row, 0), vt102->height - 1);
	}

	if (x != vt102->cursor_x || y != vt102->cursor_y) {
		vt102->cursor_x = x; vt102->cursor_y = y;

		if (vt102->mode.cursor_visible) vt102_changed(vt102);
	}
}

void
MoveRelative(VT102 *vt102, int col, int row) {
	MoveTo(vt102, vt102->cursor_x + col, vt102->cursor_y + row);
}

void
ClearFromBegin(VT102 *vt102) {
	struct VT102_CanvasChar* text = vt102->canvas[vt102->cursor_y];

	for (int col=0; col<=vt102->cursor_x; col++) {
		text[col] = vt102->default_char;
	} 

	vt102_changed(vt102);
}

void
ClearToEnd(VT102 *vt102) {
	struct VT102_CanvasChar* text = vt102->canvas[vt102->cursor_y];

	for (int col=vt102->cursor_x; col<vt102->width; col++) {
		text[col] = vt102->default_char;
	} 

	vt102_changed(vt102);
}

void
ClearLine(VT102 *vt102) {
	struct VT102_CanvasChar* text = vt102->canvas[vt102->cursor_y];

	for (int col=0; col<vt102->width; col++) {
		text[col] = vt102->default_char;
	} 

	vt102_changed(vt102);
}

void
InsertChar(VT102 *vt102, int count) {
	if (count > 0) {
		if (count > vt102->width - vt102->cursor_x) count = vt102->width - vt102->cursor_x;

		struct VT102_CanvasChar* text = vt102->canvas[vt102->cursor_y];

		for (int i=vt102->width-count-1; i>=vt102->cursor_x; i--) {
			text[i + count] = text[i];
		}

		// clear the moved cols and keep the attributes from the first col
		for (int i=vt102->cursor_x; i<vt102->cursor_x+count; i++) {
			text[i] = vt102->default_char; //CanvasChar(default_char.ch, text[cursor_x]);
		}

		vt102_changed(vt102);
	}
}

void
DeleteChar(VT102 *vt102, int count) {
	if (count > 0) {
		if (count > vt102->width - vt102->cursor_x) count = vt102->width - vt102->cursor_x;

		struct VT102_CanvasChar* text = vt102->canvas[vt102->cursor_y];

		for (int i=vt102->cursor_x; i<vt102->width-count; i++) {
			text[i] = text[i + count];
		}

		// clear the erased cols and keep the attributes from the last col
		for (int i=vt102->width-count; i<vt102->width; i++) {
			SetCanvasChar(&text[i], vt102->default_char.ch, &text[vt102->width - 1]);
		}

		vt102_changed(vt102);
	}
}

void
ReportDeviceStatus(VT102 *vt102, int request) {
}

void
ReportDeviceAttributes(VT102 *vt102, int request) {
}

void
KeyCarriageReturn(VT102 *vt102) {
	MoveTo(vt102, 0, vt102->cursor_y);
}

void
KeyLineFeed(VT102 *vt102, int nl) {
	// are we in the scrolling area?
	if (vt102->cursor_y == vt102->scroll_region_bottom) {
		ScrollUp(vt102, 1);
	} else {
		// no, don't scroll
		if (vt102->cursor_y < vt102->height - 1) vt102->cursor_y++;
		if (vt102->mode.cursor_visible) vt102_changed(vt102);
	}

	if (nl && vt102->mode.new_line) {
		KeyCarriageReturn(vt102);
	}
}

void
KeyBackspace(VT102 *vt102) {
	if (vt102->cursor_x > 0) {
		// in insert mode, first move the other chars leftwards
		if (vt102->mode.insert) {
			struct VT102_CanvasChar* text = vt102->canvas[vt102->cursor_y];

			for (int i=vt102->cursor_x-1; i<vt102->width-1; i++) {
				text[i] = text[i + 1];
			}
		}

		vt102->cursor_x--;
		vt102_changed(vt102);
	}
}

void
KeyDelete(VT102 *vt102) {
	struct VT102_CanvasChar* text = vt102->canvas[vt102->cursor_y];

	for (int col=vt102->cursor_x; col<vt102->width-1; col++) {
		text[col] = text[col + 1];
	}
	text[vt102->width - 1] = vt102->default_char;

	vt102_changed(vt102);
}

void
KeyTab(VT102 *vt102) {
	for (int i=0; i<CONSOLE_MAXCOLS; i++) {
		if (vt102->tabs[i] > vt102->cursor_x) {
			for (int j=vt102->tabs[i]-vt102->cursor_x; j>0; j--) KeyInsert(vt102, ' ');

			return;
		}

		if (vt102->tabs[i] == 0) break;
	}

	// no tab found -> do nothing
}

void
KeyInsert(VT102 *vt102, char ch) {
	// ignore all not printable chars
	//	if (ch < ' ') return;

	int x = vt102->cursor_x, y = vt102->cursor_y;

	// this is to prevent a line feed if the cursor is on
	// the last col. in this case the cursor stayes out of
	// the screen until it will be placed elsewhere or
	// another char will be put in. then a line feed and if
	// necessary also a scroll up will accour.

	if (vt102->mode.wrap_around) {
		if (++vt102->cursor_x > vt102->width) {
			KeyCarriageReturn(vt102);
			KeyLineFeed(vt102, 0);

			return;
		}
	} else {
		if (++vt102->cursor_x >= vt102->width) vt102->cursor_x = x = vt102->width - 1;
	}

	struct VT102_CanvasChar *text = vt102->canvas[y];

	// in insert mode, first move the other chars rightwards
	if (vt102->mode.insert) {
		for (int i=vt102->width-1; i>x; i--) {
			text[i] = text[i - 1];
		}
	}

	SetCanvasChar(&text[x], ch, &vt102->default_char);

	vt102_changed(vt102);
}

void
KeyBell(VT102 *vt102) {
	syslog(SYS_LOG_DEBUG, "console bell!!!\n");
	vt102->bell = 1;
}

void
SetScrollRegion(VT102 *vt102, int top, int bottom) {
	if (top < 0) {
		top = 0;
	} else if (top >= vt102->height - 1) {
		top = vt102->height - 2;
	}

	// range minimum 2 rows!
	if (bottom < top + 1) {
		bottom = top + 1;
	} else if (bottom >= vt102->height) {
		bottom = vt102->height - 1;
	}

	vt102->scroll_region_top = top;
	vt102->scroll_region_bottom = bottom;

	syslog(SYS_LOG_DEBUG, "VT102->SetScrollRegion to %i, %i\n", top, bottom);
}

void
CursorPosSave(VT102 *vt102) {
	vt102->saved_cursor_pos =
		SaveCursorPos(vt102->cursor_x, vt102->cursor_y, &vt102->default_char,
		vt102->mode.origin, vt102->saved_cursor_pos);
}

void
CursorPosRestore(VT102 *vt102) {
	struct VT102_CursorPos *old = vt102->saved_cursor_pos;

	if (old) {
		vt102->mode.origin = old->mode_origin;

		if (vt102->cursor_x != old->posx || vt102->cursor_y != old->posy) {
			vt102->cursor_x = old->posx; vt102->cursor_y = old->posy;
			vt102_changed(vt102);
		}

		vt102->default_char = old->attributes;
		vt102->saved_cursor_pos = old->prev;

		free(old);
	} else {
		// No cursor position stored, so set the cursor to home.
		MoveTo(vt102, 0, 0);
	}
}

void
SetAttributes(VT102 *vt102, int *attributes, int count) {
	if (count > 0) {
		for (int i=0; i<count; i++) {
			switch (attributes[i]) {
				// reset all attributes
				case  0: ResetCanvasChar(&vt102->default_char); break;

				case  1: vt102->default_char.bold       = 1; break;
				case 22: vt102->default_char.bold       = 0; break;
				case  4: vt102->default_char.underscore = 1; break;
				case 24: vt102->default_char.underscore = 0; break;
				case  5: vt102->default_char.blink      = 1; break;
				case 25: vt102->default_char.blink      = 0; break;
				case  7: vt102->default_char.inverted   = 1; break;
				case 27: vt102->default_char.inverted   = 0; break;
				case  8: vt102->default_char.concealed  = 1; break;
				case 28: vt102->default_char.concealed  = 0; break;
				case 39: vt102->default_char.use_fg     = 0; break;
				case 49: vt102->default_char.use_bg     = 0; break;

				case 30 ... 37:
					// set foreground color
					vt102->default_char.foreground = attributes[i] - 30;
					vt102->default_char.use_fg = 1;
				break;

				case 40 ... 47:
					// set background color
					vt102->default_char.background = attributes[i] - 40;
					vt102->default_char.use_bg = 1;
				break;

				default: return;
			}
		} // for
	} else {
		// no params -> reset attributes
		ResetCanvasChar(&vt102->default_char);
	} // if
}

void
SetModes(VT102 *vt102, int *attributes, int count) {
	for (int i=0; i<count; i++) {
		switch (attributes[i]) {
			case  4: vt102->mode.insert = 1;                           break;
			case  6: SetModeOrigin(vt102, 1);                          break;
			case  7: vt102->mode.wrap_around = 1; MoveTo(vt102, 0, 0); break;
			case 20: vt102->mode.new_line = 1;                         break;
			case 25: SetCursorVisible(vt102, 1);                       break;

			default: syslog(SYS_LOG_WARN, "unknown mode set %i\n", attributes[i]);
		}
	}
}

void
ResetModes(VT102 *vt102, int *attributes, int count) {
	for (int i=0; i<count; i++) {
		switch (attributes[i]) {
			case  4: vt102->mode.insert      = 0;                      break;
			case  6: SetModeOrigin(vt102, 0);                          break;
			case  7: vt102->mode.wrap_around = 0; MoveTo(vt102, 0, 0); break;
			case 20: vt102->mode.new_line    = 0;                      break;
			case 25: SetCursorVisible(vt102, 0);                       break;

			default: syslog(SYS_LOG_WARN, "unknown mode reset %i\n", attributes[i]);
		}
	}
}

void
SetCursorVisible(VT102 *vt102, int visible) {
	if (vt102->mode.cursor_visible != visible) {
		vt102->mode.cursor_visible = visible;
		vt102_changed(vt102);
	}
}

void
SetModeOrigin(VT102 *vt102, int origin) {
	if (vt102->mode.origin != origin) {
		vt102->mode.origin = origin;
		MoveTo(vt102, 0, 0);
	}
}
