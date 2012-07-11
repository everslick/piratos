#ifndef _TERMINAL_VT102_H_
#define _TERMINAL_VT102_H_

#include <sys/ioctl.h>
#include <termios.h>
#include <stdarg.h>

#define CONSOLE_MAXCOLS 256
#define CONSOLE_MAXROWS 256

// define used keys
#define  BS '\010' // Backspace
#define  HT '\011' // Horizontal Tab
#define  LF '\012' // Line Feed
#define  VT '\013' // Vertical Tab
#define  FF '\014' // Form Feed
#define  CR '\015' // Carriage Return
#define  SO '\016' // Shift out
#define  SI '\017' // Shift in

#define CAN '\030' // Cancel
#define SUB '\032' // Substitude
#define ESC '\033' // Escape

class VT102 {

public:

	struct CanvasChar;

	VT102(void);
	virtual ~VT102(void);

	void Write(const unsigned char *stream);

	const CanvasChar **Canvas(void) { return ((const CanvasChar **)canvas); }

	void SetOutputFD(int f) { fd = f; }

	bool SetSize(int w, int h);

	int Width(void)  { return (width);  }
	int Height(void) { return (height); }
	
	int CursorX(void) {
		return (cursor_x);
	}
	int CursorY(void) {
		return (mode.origin ? cursor_y - scroll_region_top : cursor_y);
	}

	bool CursorVisible(void) { return (mode.cursor_visible); }

	bool ToRefresh(void) { return (changed); }
	void Refreshed(void) { changed = false;  }
	bool ToRing(void)    { return (bell);    }
	void BellSeen(void)  { bell = false;     }

	struct CanvasChar {
		CanvasChar(void) {
			ch = ' ';

			foreground = 7;
			background = 1;

			bold       = false;
			underscore = false;
			blink      = false;
			inverted   = false;
			concealed  = false;
			use_fg     = false;
			use_bg     = false;
		}

		CanvasChar(unsigned char c, const CanvasChar &attr) {
			ch = c;

			foreground = attr.foreground;
			background = attr.background;

			bold       = attr.bold;
			underscore = attr.underscore;
			blink      = attr.blink;
			inverted   = attr.inverted;
			concealed  = attr.concealed;
			use_fg     = attr.use_fg;
			use_bg     = attr.use_bg;
		}

		unsigned char ch;

		unsigned char foreground;
		unsigned char background;

		bool bold;
		bool underscore;
		bool blink;
		bool inverted;
		bool concealed;
		bool use_fg;
		bool use_bg;
	};

private:

	enum VT102State {
		STATE_NORMAL,
		STATE_ESCAPE,
		STATE_ESCAPE_PARAMETER,
		STATE_ESCAPE_SINGLE_CODE,
		STATE_SELECT_CHARSET_G0,
		STATE_SELECT_CHARSET_G1
	};

	struct CursorPos {
		CursorPos(int x, int y, CanvasChar &attr, bool orig, CursorPos *pos) {
			posx = x;
			posy = y;

			attributes = attr;
			mode_origin = orig;

			prev = pos;
		}

		int posx, posy;
		CanvasChar attributes;
		bool mode_origin;

		CursorPos *prev;
	};

	struct Mode {
		bool cursor_visible;
		bool wrap_around;
		bool new_line;
		bool origin;
		bool insert;
	} mode;

	void Changed(void) { changed = true; }

	void Clear(int x1 = 0, int y1 = 0, int x2 = -1, int y2 = -1);

	void ScrollUp(int count = 1)   { ScrollUp(count, scroll_region_top );  }
	void ScrollDown(int count = 1) { ScrollDown(count, scroll_region_top); }
	
	void ScrollUp(int count, int start);
	void ScrollDown(int count, int start);

	void DecodeEscapeSequence(char code);
	void DecodeEscapeCode(char code);
	void DecodeEscapeSingleCode(char code);

	void TabStopClear(void);
	void TabStopAdd(int tabstop);
	void TabStopRemove(int tabstop);

	void KeyCarriageReturn(void);
	void KeyLineFeed(bool nl);
	void KeyBackspace(void);
	void KeyDelete(void);
	void KeyTab(void);
	void KeyInsert(unsigned char ch = ' ');
	void KeyBell(void);

	void SetAttributes(int *attributes, int count);

	void SetModes(int *attributes, int count);
	void ResetModes(int *attributes, int count);

	void SetCursorVisible(bool visible);
	void SetModeOrigin(bool origin);

	void ClearToEnd(void);
	void ClearFromBegin(void);
	void ClearLine(void);

	void InsertChar(int count);
	void DeleteChar(int count);
	void InsertLine(int count);
	void DeleteLine(int count);

	void ReportDeviceStatus(int request);
	void ReportDeviceAttributes(int request);

	void MoveTo(int col, int row);
	void MoveRelative(int col, int row);

	void SetScrollRegion(int top, int bottom);

	void CursorPosSave(void);
	void CursorPosRestore(void);

	void Write(unsigned char ch);

	void SelectCharSet(int g, char set);

	int DebugOut(const char *fmt, ...);

	int width, height;
	CanvasChar *canvas[CONSOLE_MAXROWS];

	int cursor_x, cursor_y;
	CursorPos *saved_cursor_pos;

	int scroll_region_top, scroll_region_bottom;

	VT102State state;

	int *escape_params;
	int escape_params_count;

	bool changed, bell;

	CanvasChar default_char;

	int tabs[CONSOLE_MAXCOLS];

	unsigned char char_set[256];

	int fd;

};

#endif // _TERMINAL_VT102_H_
