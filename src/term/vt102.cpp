/*
 * ported from VDR console plugin (C) Jan Rieger <jan@ricomp.de>
 * by Clemens Kirchgatterer <clemens@1541.org>
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "vt102.h"

#define min(A, B) ((A <= B) ? A : B)
#define max(A, B) ((A >= B) ? A : B)

VT102::VT102(void) {
	width = height = 0;
	cursor_x = cursor_y = 0;

	mode.cursor_visible = true;	// TODO load default values
	mode.wrap_around    = true;
	mode.new_line       = false;
	mode.origin         = false;
	mode.insert         = false;

	saved_cursor_pos = NULL;

	// escape sequence parser state machine
	state = STATE_NORMAL;

	changed = false;
	bell = false;

	escape_params = NULL;
	escape_params_count = 0;

	// initialize all possibilities
	for (int i=0; i<CONSOLE_MAXROWS; i++) {
		canvas[i] = 0;
	}

	// initialize tabulators
	int tab = 0;
	for (int i=0; i<CONSOLE_MAXCOLS; i++) {
		tabs[i] = tab += 8;
	}

	// No idea if this is the right size but it is
	// a good value to start.
	SetSize(80, 24);
	SetScrollRegion(0, 24);

	SelectCharSet(0, 'B');
}

VT102::~VT102(void) {
	// releasing all allocated buffers
	for (int i=0; i<height; i++) {
		free(canvas[i]);
	}

	while (saved_cursor_pos) {
		CursorPos* prev = saved_cursor_pos->prev;
		delete saved_cursor_pos;
		saved_cursor_pos = prev;
	}
}

int
VT102::DebugOut(const char *fmt, ...) {
	int n = 0;

#if 0
	va_list ap;

	va_start(ap, fmt);
	n = vprintf(fmt, ap);
	va_end(ap);
#endif

	return (n);
}

void
VT102::SelectCharSet(int g, char set) {
	//DebugOut("New CharSet selected: %i, %c\n", g, set);

	// First reset all chars
	for (int i=0; i<=255; i++) char_set[i] = i;

	if (set == 'A' || set == 'B') {
		// Mapping the pseudo graphics to special values in the lower 32 chars.
		char_set[ 179 ] = 0x19; // vertical line
		char_set[ 180 ] = 0x16; // right tee
		char_set[ 181 ] = 0x16; // right tee
		char_set[ 182 ] = 0x16; // right tee
		char_set[ 183 ] = 0xC;  // upper_right_corner
		char_set[ 184 ] = 0xC;  // upper_right_corner
		char_set[ 185 ] = 0x16; // right tee
		char_set[ 186 ] = 0x19; // vertical line
		char_set[ 187 ] = 0xC;  // upper_right_corner
		char_set[ 188 ] = 0xB;  // lower_right_corner
		char_set[ 189 ] = 0xB;  // lower_right_corner
		char_set[ 190 ] = 0xB;  // lower_right_corner
		char_set[ 191 ] = 0xC;  // upper_right_corner
		char_set[ 192 ] = 0xE;  // lower_left_corner
		char_set[ 193 ] = 0x17; // bottom tee
		char_set[ 194 ] = 0x18; // top tee
		char_set[ 195 ] = 0x15; // left tee
		char_set[ 196 ] = 0x12; // horizontal line / scan line 7
		char_set[ 197 ] = 0xF;  // cross
		char_set[ 198 ] = 0x15; // left tee
		char_set[ 199 ] = 0x15; // left tee
		char_set[ 200 ] = 0xE;  // lower_left_corner
		char_set[ 201 ] = 0xD;  // upper_left_corner
		char_set[ 202 ] = 0x17; // bottom tee
		char_set[ 203 ] = 0x18; // top tee
		char_set[ 204 ] = 0x15; // left tee
		char_set[ 205 ] = 0x12; // horizontal line / scan line 7
		char_set[ 206 ] = 0xF;  // cross
		char_set[ 207 ] = 0x17; // bottom tee
		char_set[ 208 ] = 0x17; // bottom tee
		char_set[ 209 ] = 0x18; // top tee
		char_set[ 210 ] = 0x18; // top tee
		char_set[ 211 ] = 0xE;  // lower_left_corner
		char_set[ 212 ] = 0xE;  // lower_left_corner
		char_set[ 213 ] = 0xD;  // upper_left_corner
		char_set[ 214 ] = 0xD;  // upper_left_corner
		char_set[ 215 ] = 0xF;  // cross
		char_set[ 216 ] = 0xF;  // cross
		char_set[ 217 ] = 0xB;  // lower_right_corner
		char_set[ 218 ] = 0xD;  // upper_left_corner
		// 219..240 missing :-(
		char_set[ 241 ] =	32;   // plus minus
		char_set[ 242 ] = 0x1B; // greater than or equal
		char_set[ 243 ] = 0x1A; // less than or equal
		// 244..255 missing :-(
	} else if (g == 1 && set == '0') {
		// Mapping to the lower 32 chars
		char_set[  96 ] = 0x1;  // diamond
		char_set[  97 ] =	32;   // 50% grid
		char_set[  98 ] =	32;
		char_set[  99 ] =	32;
		char_set[ 100 ] =	32;
		char_set[ 101 ] =	32;
		char_set[ 102 ] =	32;   // degree
		char_set[ 103 ] =	32;   // plus minus
		char_set[ 104 ] =	32;
		char_set[ 105 ] =	32;
		char_set[ 106 ] = 0xB;  // lower_right_corner
		char_set[ 107 ] = 0xC;  // upper_right_corner
		char_set[ 108 ] = 0xD;  // upper_left_corner
		char_set[ 109 ] = 0xE;  // lower_left_corner
		char_set[ 110 ] = 0xF;  // cross
		char_set[ 111 ] = 0x10; // scan line 1
		char_set[ 112 ] = 0x11; // scan line 3
		char_set[ 113 ] = 0x12; // horizontal line / scan line 5
		char_set[ 114 ] = 0x13; // scan line 7 / scan line 9
		char_set[ 115 ] = 0x14; // scan line 9 / horizontal line
		char_set[ 116 ] = 0x15; // left tee
		char_set[ 117 ] = 0x16; // right tee
		char_set[ 118 ] = 0x17; // bottom tee
		char_set[ 119 ] = 0x18; // top tee
		char_set[ 120 ] = 0x19; // vertical line
		char_set[ 121 ] = 0x1A; // less than or equal
		char_set[ 122 ] = 0x1B; // greater than or equal
		char_set[ 123 ] =	32;   // pi
		char_set[ 124 ] =	32;   // not equal
		char_set[ 125 ] =	32;   // british pound
		char_set[ 126 ] =	32;   // dot
	}
}

bool
VT102::SetSize(int w, int h) {
	// if nothing is to do then exit
	if (w == width && h == height) return (false);

	// if the width is the same as before then we don't need to
	// realloc rows that exist already and should exist afterwards. 
	int RowsNoChangeNeeded = min(h, height);

	if (w != width) {
		for (int row=0; row<RowsNoChangeNeeded; row++) {
			CanvasChar* &cvs_row = canvas[row];
			cvs_row = (CanvasChar *)realloc((void *)cvs_row, w * sizeof (CanvasChar));

			// initialize new cols if any
			for (int col=width; col<w; col++) {
				cvs_row[col] = default_char;
			}
		}
	}

	// allocating new rows if any
	for (int row=RowsNoChangeNeeded; row<h; row++) {
		CanvasChar* &cvs_row = canvas[row];
		cvs_row = (CanvasChar *)realloc((void *)cvs_row, w * sizeof (CanvasChar));
		
		// initialize new cols
		for (int col=0; col<w; col++) {
			cvs_row[col] = default_char;
		}
	}

	// releasing old rows if any
	for (int row=h; row<height; row++) {
		CanvasChar* &cvs_row = canvas[row];
		free(cvs_row);
		cvs_row = 0;
	}

	// change scroll region
	if ((scroll_region_top == 0) && (scroll_region_bottom == height - 1)) {
		scroll_region_bottom = h - 1;
		DebugOut("VT102->SetSize: upadting scroll region bottom to %i\n", scroll_region_bottom);
	}

	// save new size
	width = w;
	height = h;

	Changed();

	DebugOut("VT102->SetSize: %i, %i\n", width, height);

	return (true);
}

void VT102::Clear(int x1, int y1, int x2, int y2) {
	// check the range
	if (x1 < 0) {
		x1 = 0;
	} else if (x1 >= width) {
		x1 = width - 1;
	}

	if (y1 < 0) {
		y1 = 0;
	} else if (y1 >= height) {
		y1 = height - 1;
	}

	if (x2 < 0 || x2 >= width) x2 = width - 1;
	if (y2 < 0 || y2 >= height) y2 = height - 1;

	for (int row=y1; row<=y2; row++) {
		CanvasChar* cvs_row = canvas[row];

		for (int col=x1; col<=x2; col++) {
			cvs_row[col] = default_char;
		}
	}

	Changed();
}

void
VT102::ScrollUp(int count, int start) {
	// scroll only if we are in the scrolling range
	if (cursor_y >= scroll_region_top && cursor_y <= scroll_region_bottom) {
		for (int i=0; i<count; i++) {
			free(canvas[start]);

			for (int row=start; row<=scroll_region_bottom-1; row++) {
				canvas[row] = canvas[row + 1];
			}

			CanvasChar *&cvs_row = canvas[scroll_region_bottom];
			cvs_row = (CanvasChar *)malloc(width * sizeof (CanvasChar));

			for (int col=0; col<width; col++) {
				cvs_row[col] = default_char;
			}
		}

		Changed();
	}
}

void
VT102::ScrollDown(int count, int start) {
	// scroll only if we are in the scrolling range
	if (cursor_y >= scroll_region_top && cursor_y <= scroll_region_bottom) {
		for (int i=0; i<count; i++) {
			free(canvas[scroll_region_bottom]);

			for (int row=scroll_region_bottom-1; row>=start; row--) {
				canvas[row + 1] = canvas[row];
			}

			CanvasChar *&cvs_row = canvas[start];
			cvs_row = (CanvasChar *)malloc(width * sizeof (CanvasChar));
	
			for (int col=0; col<width; col++) {
				cvs_row[col] = default_char;
			}
		}

		Changed();
	}
}

void
VT102::Write(const char *stream, int len) {
	if (len) {
		for (int i=0; i<len; i++) {
			Write(stream[i]);
		}
	} else {
		while (*stream) {
			Write(*stream);
			stream++;
		}
	}
}

void
VT102::Write(char ch) {
	if (ch == SI) SelectCharSet(0, '0');
	if (ch == SO) SelectCharSet(1, '0');

printf("Write %i\n", ch);
	// search for escape sequences
	switch (state) {
		case STATE_NORMAL:
			switch (ch) {
				// interpret control chars
				case ESC: state = STATE_ESCAPE; break; // begin of escape sequence
				case  CR: KeyCarriageReturn();  break;
				case  LF:
				case  FF:
				case  VT: KeyLineFeed(true);    break;
				case  HT: KeyTab();             break;
				case  BS:
				case DEL: KeyBackspace();       break;
				case BEL: KeyBell();            break; // 7

				// display all other chars
				default: KeyInsert(char_set[ch]);
			}
		break;

		case STATE_ESCAPE:
			if (ch == '[') {
				// ok, the escape sequence begins here
				state = STATE_ESCAPE_PARAMETER;
			} else if (ch == '#') {
				state = STATE_ESCAPE_SINGLE_CODE;
			} else if (ch == '(') {
				state = STATE_SELECT_CHARSET_G0;
			} else if (ch == ')') {
				state = STATE_SELECT_CHARSET_G1;
			} else {
				DecodeEscapeCode(ch);
				// and back to normal mode
				state = STATE_NORMAL;
			}
		break;

		case  STATE_ESCAPE_PARAMETER:
			switch (ch) {
				case '0' ... '9':
					if (escape_params == 0) {
						// for the first parameter we must allocate memory
						escape_params = (int *)malloc(sizeof (int));
						*escape_params = ch - '0';
						escape_params_count = 1;
					} else {
						// assemble figures to a number
						int &Param = escape_params[escape_params_count - 1];
						Param = Param * 10 + (ch - '0');
					}
				break;

				case ';':
					// here a new parameter follows
					escape_params =
						(int *)realloc(escape_params, ++escape_params_count * sizeof (int));
					escape_params[escape_params_count - 1] = 0;
				break;

				case '?': break; // ignore the questionmark

				case CAN:
				case SUB:
					// cancel the sequence
					free(escape_params);
					escape_params = NULL;
					escape_params_count = 0;
					state = STATE_NORMAL;
				break;

				default:
					// end of sequence reached
					DecodeEscapeSequence(ch);
					free(escape_params);
					escape_params = NULL;
					escape_params_count = 0;
					state = STATE_NORMAL;
				break;
			}
		break;

		case STATE_ESCAPE_SINGLE_CODE:
			DecodeEscapeSingleCode(ch);
			state = STATE_NORMAL;
		break;

		case STATE_SELECT_CHARSET_G0:
		case STATE_SELECT_CHARSET_G1:
			SelectCharSet(state - STATE_SELECT_CHARSET_G0, ch);
			state = STATE_NORMAL;
		break;
	}
}

#define CONSOLE_PARAM(_index, _default) ((_index < escape_params_count) ? escape_params[_index] : _default)

void
VT102::DecodeEscapeSequence(char code) {
	switch (code) {
		case 'G':
			// move to column
			MoveTo(CONSOLE_PARAM(0, 1) - 1, cursor_y);
		break;
		case 'd':
			// move to line
			MoveTo(cursor_x, CONSOLE_PARAM(0, 1) - 1);
		break;
		case 'f':
		case 'H':
			// move to position
			MoveTo(CONSOLE_PARAM(1, 1) - 1, CONSOLE_PARAM(0, 1) - 1);
		break;
		case 'e':
		case 'A':
			// n rows up
			MoveRelative(0, - CONSOLE_PARAM(0, 1));
		break;
		case 'B':
			// n rows down
			MoveRelative(0,	 CONSOLE_PARAM(0, 1));
		break;
		case 'a':
		case 'C':
			// n cols right
			MoveRelative(CONSOLE_PARAM(0, 1), 0);
		break;
		case 'D':
			// n cols left
			MoveRelative(- CONSOLE_PARAM(0, 1), 0);
		break;
		case 'E':
			// n rows down, first col
			MoveRelative(-cursor_x,	 CONSOLE_PARAM(0, 1));
		break;
		case 'F':
			// n rows up, first col
			MoveRelative(-cursor_x, - CONSOLE_PARAM(0, 1));
		break;

		case 'K':
			if (escape_params_count == 0 || escape_params[0] == 0) {
				// clear to end of line
				ClearToEnd();
			} else if (escape_params[0] == 1) {
				// clear from begin of line
				ClearFromBegin();
			} else if (escape_params[0] == 2) {
				// clear the hole line
				ClearLine();
			}
		break;

		case 'J':
			if (escape_params_count == 0 || escape_params[0] == 0) {
				// clear to end of screen
				ClearToEnd();
				if (mode.origin) {
					Clear(0, cursor_y + 1, width - 1, scroll_region_bottom);
				} else {
					Clear(0, cursor_y + 1, width - 1, height - 1);
				}
			} else if (escape_params[0] == 1) {
				// clear from begin of screen
				if (mode.origin) {
					Clear(0, scroll_region_top, width - 1, cursor_y - 1);
				} else {
					Clear(0, 0, width - 1, cursor_y - 1);
				}
				ClearFromBegin();
			} else if (escape_params[0] == 2) {
				// clear the hole screen
				if (mode.origin) {
					Clear(0, scroll_region_top, width - 1, scroll_region_bottom);
				} else {
					Clear();
				}
			}
			MoveTo(0, 0);
		break;

		case 'n': ReportDeviceStatus(CONSOLE_PARAM(0, 0));     break;
		case 'c': ReportDeviceAttributes(CONSOLE_PARAM(0, 0)); break;

		// set display attributes
		case 'm': SetAttributes(escape_params, escape_params_count); break;
		// put in screen mode
		case 'h': SetModes(escape_params, escape_params_count);      break;
		// resets Mode
		case 'l': ResetModes(escape_params, escape_params_count);    break;

		case 'r':
			SetScrollRegion(
				CONSOLE_PARAM(0, 1) - 1, CONSOLE_PARAM(1, CONSOLE_MAXROWS) - 1
			);
		break;

		case 's': CursorPosSave();    break; // saves cursor position
		case 'u': CursorPosRestore(); break; // return to saved cursor position

		case 'g':
			if (escape_params_count == 0 || escape_params[0] == 0) {
 				// clear tab at position
				TabStopRemove(cursor_y);
			} else if (escape_params[0] == 3) {
				// clear all tabs
				TabStopClear();
			}
		break;

		case 'W':
			if (escape_params_count == 0 || escape_params[0] == 0) {
				// add tab at position
				TabStopAdd(cursor_y);
			} else if (escape_params[0] == 2) {
				// clear tab at position
				TabStopRemove(cursor_y );
			} else if (escape_params[0] == 5) {
				// clear all tabs
				TabStopClear();
			}
		break;

		case '@': InsertChar(CONSOLE_PARAM(0, 1));           break;
		case 'X':
		case 'P': DeleteChar(CONSOLE_PARAM(0, 1));           break;
		case 'L': ScrollDown(CONSOLE_PARAM(0, 1), cursor_y); break;
		case 'M': ScrollUp(CONSOLE_PARAM(0, 1), cursor_y);   break;

		// ignore unsupported
		case 'y': break; // self test
		case 'q': break; // show lamp on keyboard (1..4, 0 = off)

		default:
			DebugOut("unknown escape sequence %i\n", code);
		break;
	}
}

void
VT102::DecodeEscapeCode(char code) {
	switch (code) {
		case 'D': KeyLineFeed(false);                      break;
		case 'E': KeyLineFeed(false); MoveTo(0, cursor_y); break;

		case 'M':
			if (cursor_y == scroll_region_top) {
				ScrollDown(1);
			} else if (cursor_y > 0) {
				cursor_y--;
				Changed();
			}
		break;

		// discard subsequent char
		case '@':
			if (cursor_x < width) {
				cursor_x++;
				DeleteChar(1);
				cursor_x--;
			}
		break;

		case 'H': TabStopAdd(cursor_x);      break; // set column tab
		case 'Z': ReportDeviceAttributes(0); break;
		case '7': CursorPosSave();           break;
		case '8': CursorPosRestore();        break;

		// ignore unsupported
		case '=': break; // application keypad
		case '>': break; // numeric keypad
		case 'c': break; // reset terminal

		default: DebugOut("unknown escape code %i\n", code);
	}
}

void
VT102::DecodeEscapeSingleCode(char code) {
	switch (code) {
		// ignore unsupported
		case '3': break; // double height/width
		case '4': break;
		case '5': break; // single width line
		case '6': break;
		case '8': break; // show test pattern

		default: DebugOut("unknown escape single code %i\n", code);
	}
}

void
VT102::TabStopClear(void) {
	for (int i=0; i<CONSOLE_MAXCOLS; i++) {
		tabs[i] = 0;
	}
}

void VT102::TabStopAdd(int tabstop) {
	// search for right position in tab stop array
	for (int i=0; i<CONSOLE_MAXCOLS; i++) {
		if (tabs[i] == tabstop) {
			// the tab stop is already here

			break;
		} else if (tabs[i] == 0) {
			// append on the end
			tabs[i] = tabstop;

			break;
		} else if (tabstop < tabs[i]) {
			//found the right place to insert
			for (int j=CONSOLE_MAXROWS-2; j>=i; j--) {
				tabs[j + 1] = tabs[j];
			}

			tabs[i] = tabstop;

			break;
		}
	}
}

void
VT102::TabStopRemove(int tabstop) {
	// search the position of the tab stop
	for (int i=0; i<CONSOLE_MAXCOLS; i++) {
		if (tabs[i] == tabstop) {
			int j;

			for (j=i; j<CONSOLE_MAXCOLS-1; j++) {
				tabs[j] = tabs[j + 1];
			}

			if (j == CONSOLE_MAXCOLS - 1) {
				tabs[j] = 0;
			}

			break;
		}
	}
}

void
VT102::MoveTo(int col, int row) {
	int y, x = min(max(col, 0), width - 1);

	if (mode.origin) {
		y = min(max(row, scroll_region_top), scroll_region_bottom);
	} else {
		y = min(max(row, 0), height - 1);
	}

	if (x != cursor_x || y != cursor_y) {
		cursor_x = x; cursor_y = y;

		if (mode.cursor_visible) Changed();
	}
}

void
VT102::MoveRelative(int col, int row) {
	MoveTo(cursor_x + col, cursor_y + row);
}

void
VT102::ClearFromBegin(void) {
	CanvasChar* text = canvas[cursor_y];

	for (int col=0; col<=cursor_x; col++) {
		text[col] = default_char;
	} 

	Changed();
}

void
VT102::ClearToEnd(void) {
	CanvasChar* text = canvas[cursor_y];

	for (int col=cursor_x; col<width; col++) {
		text[col] = default_char;
	} 

	Changed();
}

void
VT102::ClearLine(void) {
	CanvasChar* text = canvas[cursor_y];

	for (int col=0; col<width; col++) {
		text[col] = default_char;
	} 

	Changed();
}

void
VT102::InsertChar(int count) {
	if (count > 0) {
		if (count > width - cursor_x) count = width - cursor_x;

		CanvasChar* text = canvas[cursor_y];

		for (int i=width-count-1; i>=cursor_x; i--) {
			text[i + count] = text[i];
		}

		// clear the moved cols and keep the attributes from the first col
		for (int i=cursor_x; i<cursor_x+count; i++) {
			text[i] = default_char; //CanvasChar(default_char.ch, text[cursor_x]);
		}

		Changed();
	}
}

void
VT102::DeleteChar(int count) {
	if (count > 0) {
		if (count > width - cursor_x) count = width - cursor_x;

		CanvasChar* text = canvas[cursor_y];

		for (int i=cursor_x; i<width-count; i++) {
			text[i] = text[i + count];
		}

		// clear the erased cols and keep the attributes from the last col
		for (int i=width-count; i<width; i++) {
			text[i] = CanvasChar(default_char.ch, text[width - 1]);
		}

		Changed();
	}
}

void
VT102::ReportDeviceStatus(int request) {
}

void
VT102::ReportDeviceAttributes(int request) {
}

void
VT102::KeyCarriageReturn(void) {
	MoveTo(0, cursor_y);
}

void
VT102::KeyLineFeed(bool nl) {
	// are we in the scrolling area?
	if (cursor_y == scroll_region_bottom) {
		ScrollUp(1);
	} else {
		// no, don't scroll
		if (cursor_y < height - 1) cursor_y++;
		if (mode.cursor_visible) Changed();
	}

	if (nl && mode.new_line) {
		KeyCarriageReturn();
	}
}

void
VT102::KeyBackspace(void) {
	if (cursor_x > 0) {
		// in insert mode, first move the other chars leftwards
		if (mode.insert) {
			CanvasChar* text = canvas[cursor_y];

			for (int i=cursor_x-1; i<width-1; i++) {
				text[i] = text[i + 1];
			}
		}

		cursor_x--;
		Changed();
	}
}

void
VT102::KeyDelete(void) {
	CanvasChar* text = canvas[cursor_y];

	for (int col=cursor_x; col<width-1; col++) {
		text[col] = text[col + 1];
	}
	text[width - 1] = default_char;

	Changed();
}

void
VT102::KeyTab(void) {
	for (int i=0; i<CONSOLE_MAXCOLS; i++) {
		if (tabs[i] > cursor_x) {
			for (int j=tabs[i]-cursor_x; j>0; j--) KeyInsert(' ');

			return;
		}

		if (tabs[i] == 0) break;
	}

	// no tab found -> do nothing
}

void
VT102::KeyInsert(char ch) {
	// ignore all not printable chars
	//	if (ch < ' ') return;

	int x = cursor_x, y = cursor_y;

	// this is to prevent a line feed if the cursor is on
	// the last col. in this case the cursor stayes out of
	// the screen until it will be placed elsewhere or
	// another char will be put in. then a line feed and if
	// necessary also a scroll up will accour.

	if (mode.wrap_around) {
		if (++cursor_x > width) {
			KeyCarriageReturn();
			KeyLineFeed(false);

			return;
		}
	} else {
		if (++cursor_x >= width) cursor_x = x = width - 1;
	}

	CanvasChar *text = canvas[y];

	// in insert mode, first move the other chars rightwards
	if (mode.insert) {
		for (int i=width-1; i>x; i--) {
			text[i] = text[i - 1];
		}
	}

	text[x] = CanvasChar(ch, default_char);

	Changed();
}

void
VT102::KeyBell(void) {
	// misuse the incoming console (tty) to produce the bell for us
	//DebugOut("\a"); fflush(stdin);

	bell = true;
}

void
VT102::SetScrollRegion(int top, int bottom) {
	if (top < 0) {
		top = 0;
	} else if (top >= height - 1) {
		top = height - 2;
	}

	// range minimum 2 rows!
	if (bottom < top + 1) {
		bottom = top + 1;
	} else if (bottom >= height) {
		bottom = height - 1;
	}

	scroll_region_top = top;
	scroll_region_bottom = bottom;

	DebugOut("VT102->SetScrollRegion to %i, %i\n", top, bottom);
}

void
VT102::CursorPosSave(void) {
	saved_cursor_pos =
		new CursorPos(cursor_x, cursor_y, default_char, mode.origin, saved_cursor_pos);
}

void
VT102::CursorPosRestore(void) {
	CursorPos *old = saved_cursor_pos;

	if (old) {
		mode.origin	= old->mode_origin;

		if (cursor_x != old->posx || cursor_y != old->posy) {
			cursor_x = old->posx; cursor_y = old->posy;
			Changed();
		}

		default_char = old->attributes;
		saved_cursor_pos = old->prev;

		delete (old);
	} else {
		// No cursor position stored, so set the cursor to home.
		MoveTo(0, 0);
	}
}

void
VT102::SetAttributes(int *attributes, int count) {
	if (count > 0) {
		for (int i=0; i<count; i++) {
			switch (attributes[i]) {
				// reset all attributes
				case  0: default_char = CanvasChar(); break;

				case  1: default_char.bold       = true;  break;
				case 22: default_char.bold       = false; break;
				case  4: default_char.underscore = true;  break;
				case 24: default_char.underscore = false; break;
				case  5: default_char.blink      = true;  break;
				case 25: default_char.blink      = false; break;
				case  7: default_char.inverted   = true;  break;
				case 27: default_char.inverted   = false; break;
				case  8: default_char.concealed  = true;  break;
				case 28: default_char.concealed  = false; break;
				case 39: default_char.use_fg     = false; break;
				case 49: default_char.use_bg     = false; break;

				case 30 ... 37:
					// set foreground color
					default_char.foreground = attributes[i] - 30;
					default_char.use_fg = true;
				break;

				case 40 ... 47:
					// set background color
					default_char.background = attributes[i] - 40;
					default_char.use_bg = true;
				break;

				default: return;
			}
		} // for
	} else {
		// no params -> reset attributes
		default_char = CanvasChar();
	} // if
}

void
VT102::SetModes(int *attributes, int count) {
	for (int i=0; i<count; i++) {
		switch (attributes[i]) {
			case  4: mode.insert = true;                    break;
			case  6: SetModeOrigin(true);                   break;
			case  7: mode.wrap_around = true; MoveTo(0, 0); break;
			case 20: mode.new_line = true;                  break;
			case 25: SetCursorVisible(true);                break;

			default: DebugOut("unknown mode set %i\n", attributes[i]);
		}
	}
}

void
VT102::ResetModes(int *attributes, int count) {
	for (int i=0; i<count; i++) {
		switch (attributes[i]) {
			case  4: mode.insert = false;                    break;
			case  6: SetModeOrigin(false);                   break;
			case  7: mode.wrap_around = false; MoveTo(0, 0); break;
			case 20: mode.new_line = false;                  break;
			case 25: SetCursorVisible(false);                break;

			default: DebugOut("unknown mode reset %i\n", attributes[i]);
		}
	}
}

void
VT102::SetCursorVisible(bool visible) {
	if (mode.cursor_visible != visible) {
		mode.cursor_visible = visible;
		Changed();
	}
}

void
VT102::SetModeOrigin(bool origin) {
	if (mode.origin != origin) {
		mode.origin = origin;
		MoveTo(0, 0);
	}
}
