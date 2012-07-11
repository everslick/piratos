#ifndef _KBD_SYMBOLS_H_
#define _KBD_SYMBOLS_H_

/** What we really want is a mapping of every raw key on the keyboard.
 *  To support international keyboards, we use the range 0xA1 - 0xFF
 *  as international virtual keycodes.  We'll follow in the footsteps of X11...
 *  @brief The names of the keys
 */
typedef enum {
        /** @name ASCII mapped keysyms
         *  The keyboard syms have been cleverly chosen to map to ASCII
         */
        /*@{*/
	KBD_KEY_UNKNOWN		= 0,
	KBD_KEY_FIRST			= 0,
	KBD_KEY_BACKSPACE		= 8,
	KBD_KEY_TAB				= 9,
	KBD_KEY_CLEAR			= 12,
	KBD_KEY_RETURN			= 13,
	KBD_KEY_PAUSE			= 19,
	KBD_KEY_ESCAPE			= 27,
	KBD_KEY_SPACE			= 32,
	KBD_KEY_EXCLAIM		= 33,
	KBD_KEY_QUOTEDBL		= 34,
	KBD_KEY_HASH			= 35,
	KBD_KEY_DOLLAR			= 36,
	KBD_KEY_AMPERSAND		= 38,
	KBD_KEY_QUOTE			= 39,
	KBD_KEY_LEFTPAREN		= 40,
	KBD_KEY_RIGHTPAREN	= 41,
	KBD_KEY_ASTERISK		= 42,
	KBD_KEY_PLUS			= 43,
	KBD_KEY_COMMA			= 44,
	KBD_KEY_MINUS			= 45,
	KBD_KEY_PERIOD			= 46,
	KBD_KEY_SLASH			= 47,
	KBD_KEY_0				= 48,
	KBD_KEY_1				= 49,
	KBD_KEY_2				= 50,
	KBD_KEY_3				= 51,
	KBD_KEY_4				= 52,
	KBD_KEY_5				= 53,
	KBD_KEY_6				= 54,
	KBD_KEY_7				= 55,
	KBD_KEY_8				= 56,
	KBD_KEY_9				= 57,
	KBD_KEY_COLON			= 58,
	KBD_KEY_SEMICOLON		= 59,
	KBD_KEY_LESS			= 60,
	KBD_KEY_EQUALS			= 61,
	KBD_KEY_GREATER		= 62,
	KBD_KEY_QUESTION		= 63,
	KBD_KEY_AT				= 64,
	/* 
	   Skip uppercase letters
	 */
	KBD_KEY_LEFTBRACKET	= 91,
	KBD_KEY_BACKSLASH		= 92,
	KBD_KEY_RIGHTBRACKET	= 93,
	KBD_KEY_CARET			= 94,
	KBD_KEY_UNDERSCORE	= 95,
	KBD_KEY_BACKQUOTE		= 96,
	KBD_KEY_a				= 97,
	KBD_KEY_b				= 98,
	KBD_KEY_c				= 99,
	KBD_KEY_d				= 100,
	KBD_KEY_e				= 101,
	KBD_KEY_f				= 102,
	KBD_KEY_g				= 103,
	KBD_KEY_h				= 104,
	KBD_KEY_i				= 105,
	KBD_KEY_j				= 106,
	KBD_KEY_k				= 107,
	KBD_KEY_l				= 108,
	KBD_KEY_m				= 109,
	KBD_KEY_n				= 110,
	KBD_KEY_o				= 111,
	KBD_KEY_p				= 112,
	KBD_KEY_q				= 113,
	KBD_KEY_r				= 114,
	KBD_KEY_s				= 115,
	KBD_KEY_t				= 116,
	KBD_KEY_u				= 117,
	KBD_KEY_v				= 118,
	KBD_KEY_w				= 119,
	KBD_KEY_x				= 120,
	KBD_KEY_y				= 121,
	KBD_KEY_z				= 122,
	KBD_KEY_DELETE			= 127,
	/* End of ASCII mapped keysyms */
        /*@}*/

	/** @name International keyboard syms */
        /*@{*/
	KBD_KEY_WORLD_0		= 160,		/* 0xA0 */
	KBD_KEY_WORLD_1		= 161,
	KBD_KEY_WORLD_2		= 162,
	KBD_KEY_WORLD_3		= 163,
	KBD_KEY_WORLD_4		= 164,
	KBD_KEY_WORLD_5		= 165,
	KBD_KEY_WORLD_6		= 166,
	KBD_KEY_WORLD_7		= 167,
	KBD_KEY_WORLD_8		= 168,
	KBD_KEY_WORLD_9		= 169,
	KBD_KEY_WORLD_10		= 170,
	KBD_KEY_WORLD_11		= 171,
	KBD_KEY_WORLD_12		= 172,
	KBD_KEY_WORLD_13		= 173,
	KBD_KEY_WORLD_14		= 174,
	KBD_KEY_WORLD_15		= 175,
	KBD_KEY_WORLD_16		= 176,
	KBD_KEY_WORLD_17		= 177,
	KBD_KEY_WORLD_18		= 178,
	KBD_KEY_WORLD_19		= 179,
	KBD_KEY_WORLD_20		= 180,
	KBD_KEY_WORLD_21		= 181,
	KBD_KEY_WORLD_22		= 182,
	KBD_KEY_WORLD_23		= 183,
	KBD_KEY_WORLD_24		= 184,
	KBD_KEY_WORLD_25		= 185,
	KBD_KEY_WORLD_26		= 186,
	KBD_KEY_WORLD_27		= 187,
	KBD_KEY_WORLD_28		= 188,
	KBD_KEY_WORLD_29		= 189,
	KBD_KEY_WORLD_30		= 190,
	KBD_KEY_WORLD_31		= 191,
	KBD_KEY_WORLD_32		= 192,
	KBD_KEY_WORLD_33		= 193,
	KBD_KEY_WORLD_34		= 194,
	KBD_KEY_WORLD_35		= 195,
	KBD_KEY_WORLD_36		= 196,
	KBD_KEY_WORLD_37		= 197,
	KBD_KEY_WORLD_38		= 198,
	KBD_KEY_WORLD_39		= 199,
	KBD_KEY_WORLD_40		= 200,
	KBD_KEY_WORLD_41		= 201,
	KBD_KEY_WORLD_42		= 202,
	KBD_KEY_WORLD_43		= 203,
	KBD_KEY_WORLD_44		= 204,
	KBD_KEY_WORLD_45		= 205,
	KBD_KEY_WORLD_46		= 206,
	KBD_KEY_WORLD_47		= 207,
	KBD_KEY_WORLD_48		= 208,
	KBD_KEY_WORLD_49		= 209,
	KBD_KEY_WORLD_50		= 210,
	KBD_KEY_WORLD_51		= 211,
	KBD_KEY_WORLD_52		= 212,
	KBD_KEY_WORLD_53		= 213,
	KBD_KEY_WORLD_54		= 214,
	KBD_KEY_WORLD_55		= 215,
	KBD_KEY_WORLD_56		= 216,
	KBD_KEY_WORLD_57		= 217,
	KBD_KEY_WORLD_58		= 218,
	KBD_KEY_WORLD_59		= 219,
	KBD_KEY_WORLD_60		= 220,
	KBD_KEY_WORLD_61		= 221,
	KBD_KEY_WORLD_62		= 222,
	KBD_KEY_WORLD_63		= 223,
	KBD_KEY_WORLD_64		= 224,
	KBD_KEY_WORLD_65		= 225,
	KBD_KEY_WORLD_66		= 226,
	KBD_KEY_WORLD_67		= 227,
	KBD_KEY_WORLD_68		= 228,
	KBD_KEY_WORLD_69		= 229,
	KBD_KEY_WORLD_70		= 230,
	KBD_KEY_WORLD_71		= 231,
	KBD_KEY_WORLD_72		= 232,
	KBD_KEY_WORLD_73		= 233,
	KBD_KEY_WORLD_74		= 234,
	KBD_KEY_WORLD_75		= 235,
	KBD_KEY_WORLD_76		= 236,
	KBD_KEY_WORLD_77		= 237,
	KBD_KEY_WORLD_78		= 238,
	KBD_KEY_WORLD_79		= 239,
	KBD_KEY_WORLD_80		= 240,
	KBD_KEY_WORLD_81		= 241,
	KBD_KEY_WORLD_82		= 242,
	KBD_KEY_WORLD_83		= 243,
	KBD_KEY_WORLD_84		= 244,
	KBD_KEY_WORLD_85		= 245,
	KBD_KEY_WORLD_86		= 246,
	KBD_KEY_WORLD_87		= 247,
	KBD_KEY_WORLD_88		= 248,
	KBD_KEY_WORLD_89		= 249,
	KBD_KEY_WORLD_90		= 250,
	KBD_KEY_WORLD_91		= 251,
	KBD_KEY_WORLD_92		= 252,
	KBD_KEY_WORLD_93		= 253,
	KBD_KEY_WORLD_94		= 254,
	KBD_KEY_WORLD_95		= 255,		/* 0xFF */
        /*@}*/

	/** @name Numeric keypad */
        /*@{*/
	KBD_KEY_KP0				= 256,
	KBD_KEY_KP1				= 257,
	KBD_KEY_KP2				= 258,
	KBD_KEY_KP3				= 259,
	KBD_KEY_KP4				= 260,
	KBD_KEY_KP5				= 261,
	KBD_KEY_KP6				= 262,
	KBD_KEY_KP7				= 263,
	KBD_KEY_KP8				= 264,
	KBD_KEY_KP9				= 265,
	KBD_KEY_KP_PERIOD		= 266,
	KBD_KEY_KP_DIVIDE		= 267,
	KBD_KEY_KP_MULTIPLY	= 268,
	KBD_KEY_KP_MINUS		= 269,
	KBD_KEY_KP_PLUS		= 270,
	KBD_KEY_KP_ENTER		= 271,
	KBD_KEY_KP_EQUALS		= 272,
        /*@}*/

	/** @name Arrows + Home/End pad */
        /*@{*/
	KBD_KEY_UP				= 273,
	KBD_KEY_DOWN			= 274,
	KBD_KEY_RIGHT			= 275,
	KBD_KEY_LEFT			= 276,
	KBD_KEY_INSERT			= 277,
	KBD_KEY_HOME			= 278,
	KBD_KEY_END				= 279,
	KBD_KEY_PAGEUP			= 280,
	KBD_KEY_PAGEDOWN		= 281,
        /*@}*/

	/** @name Function keys */
        /*@{*/
	KBD_KEY_F1				= 282,
	KBD_KEY_F2				= 283,
	KBD_KEY_F3				= 284,
	KBD_KEY_F4				= 285,
	KBD_KEY_F5				= 286,
	KBD_KEY_F6				= 287,
	KBD_KEY_F7				= 288,
	KBD_KEY_F8				= 289,
	KBD_KEY_F9				= 290,
	KBD_KEY_F10				= 291,
	KBD_KEY_F11				= 292,
	KBD_KEY_F12				= 293,
	KBD_KEY_F13				= 294,
	KBD_KEY_F14				= 295,
	KBD_KEY_F15				= 296,
        /*@}*/

	/** @name Key state modifier keys */
        /*@{*/
	KBD_KEY_NUMLOCK		= 300,
	KBD_KEY_CAPSLOCK		= 301,
	KBD_KEY_SCROLLOCK		= 302,
	KBD_KEY_RSHIFT			= 303,
	KBD_KEY_LSHIFT			= 304,
	KBD_KEY_RCTRL			= 305,
	KBD_KEY_LCTRL			= 306,
	KBD_KEY_RALT			= 307,
	KBD_KEY_LALT			= 308,
	KBD_KEY_RMETA			= 309,
	KBD_KEY_LMETA			= 310,
	KBD_KEY_LSUPER			= 311,		/**< Left "Windows" key */
	KBD_KEY_RSUPER			= 312,		/**< Right "Windows" key */
	KBD_KEY_MODE			= 313,		/**< "Alt Gr" key */
	KBD_KEY_COMPOSE		= 314,		/**< Multi-key compose key */
        /*@}*/

	/** @name Miscellaneous function keys */
        /*@{*/
	KBD_KEY_HELP			= 315,
	KBD_KEY_PRINT			= 316,
	KBD_KEY_SYSREQ			= 317,
	KBD_KEY_BREAK			= 318,
	KBD_KEY_MENU			= 319,
	KBD_KEY_POWER			= 320,		/**< Power Macintosh power key */
	KBD_KEY_EURO			= 321,		/**< Some european keyboards */
	KBD_KEY_UNDO			= 322,		/**< Atari keyboard has Undo */
        /*@}*/

	/* Add any other keys here */

	KBD_KEY_LAST
} KBD_Symbol;

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
} KBD_Modifier;

#define KBD_MOD_CTRL  (  KBD_MOD_LCTRL | KBD_MOD_RCTRL  )
#define KBD_MOD_SHIFT ( KBD_MOD_LSHIFT | KBD_MOD_RSHIFT )
#define KBD_MOD_ALT   (   KBD_MOD_LALT | KBD_MOD_RALT   )
#define KBD_MOD_META  (  KBD_MOD_LMETA | KBD_MOD_RMETA  )

#endif // _KBD_SYMBOLS_H_
