#ifndef _KBD_H_
#define _KBD_H_

typedef enum {
	KBD_MOD_NONE   = 0x0000,
	KBD_MOD_LSHIFT = 0x0001,
	KBD_MOD_RSHIFT = 0x0002,
	KBD_MOD_LCTRL  = 0x0040,
	KBD_MOD_RCTRL  = 0x0080,
	KBD_MOD_LALT   = 0x0100,
	KBD_MOD_RALT   = 0x0200,
	KBD_MOD_LMETA  = 0x0400,
	KBD_MOD_RMETA  = 0x0800,
	KBD_MOD_NUM    = 0x1000,
	KBD_MOD_CAPS   = 0x2000,
	KBD_MOD_MODE   = 0x4000
} KBD_Mod;

#define KBD_MOD_CTRL  (  KBD_MOD_LCTRL | KBD_MOD_RCTRL  )
#define KBD_MOD_SHIFT ( KBD_MOD_LSHIFT | KBD_MOD_RSHIFT )
#define KBD_MOD_ALT   (   KBD_MOD_LALT | KBD_MOD_RALT   )
#define KBD_MOD_META  (  KBD_MOD_LMETA | KBD_MOD_RMETA  )

typedef struct {
	int type;
	int state;
	int symbol;
	int modifier;
} KBD_Event;

enum {
	KBD_EVENT_TYPE_NONE,
	KBD_EVENT_TYPE_KEY
};

enum {
	KBD_EVENT_STATE_NONE,
	KBD_EVENT_STATE_PRESSED,
	KBD_EVENT_STATE_RELEASED
};

int kbd_init(void);
int kbd_fini(void);

int kbd_poll(KBD_Event *ev);

#endif // _KBD_H_
