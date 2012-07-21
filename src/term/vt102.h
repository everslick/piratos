#ifndef _TERM_VT102_H_
#define _TERM_VT102_H_

#define CONSOLE_MAXCOLS 256
#define CONSOLE_MAXROWS 128

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
#define DEL   127  // Delete
#define BEL   '\a' // Bell

enum VT102_State {
	VT102_STATE_NORMAL,
	VT102_STATE_ESCAPE,
	VT102_STATE_ESCAPE_PARAMETER,
	VT102_STATE_ESCAPE_SINGLE_CODE,
	VT102_STATE_SELECT_CHARSET_G0,
	VT102_STATE_SELECT_CHARSET_G1
};

struct VT102_CanvasChar {
	char ch;

	char foreground;
	char background;

	int bold;
	int underscore;
	int blink;
	int inverted;
	int concealed;
	int use_fg;
	int use_bg;
};

struct VT102_Mode {
	int cursor_visible;
	int wrap_around;
	int new_line;
	int origin;
	int insert;
};

struct VT102_CursorPos {
	int posx, posy;
	struct VT102_CanvasChar attributes;
	int mode_origin;

	struct VT102_CursorPos *prev;
};

typedef struct {
	int width, height;
	struct VT102_CanvasChar *canvas[CONSOLE_MAXROWS];

	int cursor_x, cursor_y;
	struct VT102_CursorPos *saved_cursor_pos;

	int scroll_region_top, scroll_region_bottom;

	enum VT102_State state;
	struct VT102_Mode mode;

	int *escape_params;
	int escape_params_count;

	int changed, bell;

	struct VT102_CanvasChar default_char;

	int tabs[CONSOLE_MAXCOLS];

	char char_set[256];
} VT102;

VT102 *vt102_new(void);
void   vt102_free(VT102 *vt102);

struct VT102_CanvasChar **vt102_canvas(VT102 *vt102);

void vt102_puts(VT102 *vt102, const char *stream, int len);
void vt102_putc(VT102 *vt102, char ch);

void vt102_move_to(VT102 *vt102, int col, int row);

void vt102_set_size(VT102 *vt102, int w, int h);

int  vt102_width(VT102 *vt102);
int  vt102_height(VT102 *vt102);

int  vt102_cursor_x(VT102 *vt102);
int  vt102_cursor_y(VT102 *vt102);

int  vt102_cursor_visible(VT102 *vt102);

int  vt102_to_refresh(VT102 *vt102);
void vt102_refreshed(VT102 *vt102);
int  vt102_to_ring(VT102 *vt102);
void vt102_bell_seen(VT102 *vt102);

void vt102_changed(VT102 *vt102);

#endif // _TERM_VT102_H_
