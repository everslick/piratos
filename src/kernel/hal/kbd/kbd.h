#ifndef _KBD_H_
#define _KBD_H_

#ifdef __cplusplus
//extern "C" {
#endif

#include "keysym.h"

typedef struct {
	int type;
	int state;
	int symbol;
	int unicode;
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

#ifdef __cplusplus
//}
#endif

#endif // _KBD_H_
