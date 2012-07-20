#ifndef _MOUSE_H_
#define _MOUSE_H_

#ifdef __cplusplus
//extern "C" {
#endif

typedef struct {
	int type;
	int state;
	int button;
	int x, y;
} MOUSE_Event;

enum {
	MOUSE_EVENT_TYPE_NONE,
	MOUSE_EVENT_TYPE_MOTION,
	MOUSE_EVENT_TYPE_BUTTON
};

enum {
	MOUSE_EVENT_STATE_NONE,
	MOUSE_EVENT_STATE_PRESSED,
	MOUSE_EVENT_STATE_RELEASED
};

int mouse_init(void);
int mouse_fini(void);

int mouse_poll(MOUSE_Event *ev);

#ifdef __cplusplus
//}
#endif

#endif // _MOUSE_H_
