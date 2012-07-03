/*
** This file is part of the Brandy Basic V Interpreter.
** Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005 David Daniels
** and Copyright (C) 2006, 2007 Colin Tuckley
**
** Brandy is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2, or (at your option)
** any later version.
**
** Brandy is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Brandy; see the file COPYING.  If not, write to
** the Free Software Foundation, 59 Temple Place - Suite 330,
** Boston, MA 02111-1307, USA.
**
**
**	This file contains the keyboard handling routines.
**
**	When running under operating systems other than RISC OS the
**	interpreter uses its own keyboard handling functions to
**	provide both line editing and a line recall feature
*/
/*
** Colin Tuckley December 2006:
**  Rewrite to use SDL library Event handling.
*/
/*
** Crispian Daniels August 20th 2002:
**	Included Mac OS X target in conditional compilation.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>

#include "common.h"
#include "target.h"
#include "basicdefs.h"
#include "errors.h"
#include "input.h"
#include "screen.h"

#include "kbd.h"
/*
** Operating system version number returned by 'INKEY'. These values
** are made up
*/
#define OSVERSION 0xFF

#define INKEYMAX 0x7FFF		/* Maximum wait time for INKEY */
#define WAITIME 10		/* Time to wait in centiseconds when dealing with ANSI key sequences */

#define HISTSIZE 1024		/* Size of command history buffer */
#define MAXHIST 20		/* Maximum number of entries in history list */

#define FN_KEY_COUNT 15		/* Number of function keys supported (0 to 15) */

static int32
  place,			/* Offset where next character will be added to buffer */
  highplace,			/* Highest value of 'place' (= number of characters in buffer) */
  histindex,			/* Index of next entry to fill in in history list */
  highbuffer,			/* Index of first free character in 'histbuffer' */
  recalline;			/* Index of last line recalled from history display */

static boolean enable_insert;	/* TRUE if keyboard input code is in insert mode */

static char histbuffer[HISTSIZE];	/* Command history buffer */
static int32 histlength[MAXHIST];	/* Table of sizes of entries in history buffer */

/* function key strings */

static struct {int length; char *text;} fn_key[FN_KEY_COUNT];

/*
** holdcount and holdstack are used when decoding ANSI key sequences. If a
** sequence is read that does not correspond to an ANSI sequence the
** characters are stored here so that they can be returned by future calls
** to 'emulate_get'. Note that this is a *stack* not a queue
*/
static int32 holdcount;		/* Number of characters held on stack */
static int32 holdstack[8];	/* Hold stack - Characters waiting to be passed back via 'get' */

/*
 * fn_string and fn_string_count are used when expanding a function
 * key string. Effectively input switches to the string after a
 * function key with a string associated with it is pressed
 */
static char *fn_string;		/* Non-NULL if taking chars from a function key string */
static int fn_string_count;	/* Count of characters left in function key string */ 

/* The following functions are common to all operating systems */

/*
** 'push_key' adds a key to the held key stack
*/
void push_key(int32 ch) {
  holdcount++;
  holdstack[holdcount] = ch;
}

/*
 * pop_key - Remove a key from the held key stack
 */
static int32 pop_key(void) {
  return holdstack[holdcount--];
}

/*
 * set_fn_string - Define a function key string
 */
void set_fn_string(int key, char *string, int length) {
  if (fn_key[key].text != NIL) free(fn_key[key].text);
  fn_key[key].length = length;
  fn_key[key].text = malloc(length);
  if (fn_key[key].text != NIL) memcpy(fn_key[key].text, string, length);
}

/*
 * switch_fn_string - Called to switch input to a function
 * key string. It returns the first character of the string
 */
static int32 switch_fn_string(int32 key) {
  int32 ch;
  if (fn_key[key].length == 1) return *fn_key[key].text;
  fn_string = fn_key[key].text;
  fn_string_count = fn_key[key].length - 1;
  ch = *fn_string;
  fn_string++;
  return ch;
}

/*
 * read_fn_string - Called when input is being taken from a
 * function key string, that is, fn_string is not NULL. It
 * returns the next character in the string.
 */
static int32 read_fn_string(void) {
  int32 ch;  
  ch = *fn_string;
  fn_string++;
  fn_string_count--;
  if (fn_string_count == 0) fn_string = NIL;	/* Last character read */
  return ch;
}

/*
 * is_fn_key - Returns the function key number if the RISC OS
 * key code passed to it is one for a function key. This is used
 * when checking for function key strings, so shifted and CTRL'ed
 * function keys are of no interest
 */
static int32 is_fn_key(int32 key) {
  if (key >= KEY_F1 && key <= KEY_F9) return key - KEY_F1 + 1;
  if (key >= KEY_F10 && key <= KEY_F12) return key - KEY_F10 + 10;
/* Not a function key */
  return 0;
}

/*
** 'decode_sequence' reads a possible ANSI escape sequence and attempts
** to decode it, converting it to a RISC OS key code. It returns the first
** character of the key code (a null) if the sequence is recognised or
** the first character of the sequence read if it cannot be identified.
** Note that the decoding is incomplete as the function only deals
** with keys of interest to it. Note also that it deals with both Linux
** and NetBSD key sequences, for example, the one for 'F1' under Linux
** is 1B 5B 5B 41 and for NetBSD it is 1B 4F 50. It should also be noted
** that the range of keys that can be decoded is a long way  short of the
** key combinations possible. Lastly, the same ANSI sequences are used for
** more than one key, for example, F11 and shift-F1 both return the same
** code.
**
** States:
** 1  ESC read		2  ESC 'O'		3  ESC '['
** 4  ESC '[' '1'	5  ESC '[' '2'		6  ESC '[' '3'
** 7  ESC '[' '4'	8  ESC '[' '5'		9  ESC '[' '6'
** 10 ESC '[' '['
** 11 ESC '[' '1' '1'	12 ESC '[' '1' '2'	13 ESC '[' '1' '3'
** 14 ESC '[' '1' '4'   15 ESC '[' '1' '5'	16 ESC '[' '1' '7'
** 17 ESC '[' '1' '8'	18 ESC '[' '1' '9'
** 19 ESC '[' '2' '0'	20 ESC '[' '2' '1'	21 ESC '[' '2' '3'
** 12 ESC '[' '2' '4'	23 ESC '[' '2' '5'	24 ESC '[' '2' '6'
** 25 ESC '[' '2' '8'	26 ESC '[' '2' '9'
** 27 ESC '[' '3' '1'	28 ESC '[' '3' '2'	29 ESC '[' '3' '3'
** 30 ESC '[' '3' '4'
**
** This code cannot deal with the 'alt' key sequences that KDE passes
** through. alt-home, for example, is presented as ESC ESC '[' 'H' and
** as 'ESC ESC' is not a recognised sequence the function simply passes
** on the data as supplied.
**
** Note: there seem to be some NetBSD escape sequences missing here
*/
static int32 decode_sequence(void) {
  int state, newstate;
  int32 ch;
  boolean ok;
  static int32 state2key [] = {	/* Maps states 11 to 24 to function key */
    KEY_F1, KEY_F2, KEY_F3, KEY_F4,		/* [11..[14 */
    KEY_F5, KEY_F6, KEY_F7, KEY_F8,		/* [15..[19 */
    KEY_F9, KEY_F10, KEY_F11, KEY_F12,		/* [20..[24 */
    SHIFT_F3, SHIFT_F4, SHIFT_F5, SHIFT_F6,	/* [25..[29 */
    SHIFT_F7, SHIFT_F8, SHIFT_F9, SHIFT_F10	/* [31..[34 */
  };
  static int statelbno[] = {4, 5, 6, 7, 8, 9};
/*
** The following tables give the next machine state for the character
** input when handling the ESC '[' '1', ESC '[' '2' and ESC '[' '3'
** sequences
*/
  static int state1[] = {11, 12, 13, 14, 15, 0, 16, 17, 18};	/* 1..9 */
  static int state2[] = {19, 20, 0, 21, 22, 23, 24, 0, 25, 26};	/* 0..9 */
  static int state3[] = {27, 28, 29, 30};	/* 1..4 */
  state = 1;	/* ESC read */
  ok = TRUE;
  while (ok && waitkey(WAITIME)) {
    ch = read_key();
    switch (state) {
    case 1:	/* ESC read */
      if (ch == 'O')	/* ESC 'O' */
        state = 2;
      else if (ch == '[')	/* ESC '[' */
        state = 3;
      else {	/* Bad sequence */
        ok = FALSE;
      }
      break;
    case 2:	/* ESC 'O' read */
      if (ch >= 'P' && ch <= 'S') {	/* ESC 'O' 'P'..'S' */
        push_key(ch - 'P' + KEY_F1);	/* Got NetBSD F1..F4. Map to RISC OS F1..F4 */
        return NUL;	/* RISC OS first char of key sequence */
      }
      else {	/* Not a known key sequence */
        ok = FALSE;
      }
      break;
    case 3:	/* ESC '[' read */
      switch (ch) {
      case 'A':	/* ESC '[' 'A' - cursor up */
        push_key(UP);
        return NUL;
      case 'B':	/* ESC '[' 'B' - cursor down */
        push_key(DOWN);
        return NUL;
      case 'C':	/* ESC '[' 'C' - cursor right */
        push_key(RIGHT);
        return NUL;
      case 'D':	/* ESC '[' 'D' - cursor left */
        push_key(LEFT);
        return NUL;
      case 'F':	/* ESC '[' 'F' - 'End' key */
        push_key(END);
        return NUL;
      case 'H':	/* ESC '[' 'H' - 'Home' key */
        return HOME;
      case '1':	case '2': case '3': case '4': case '5': case '6': /* ESC '[' '1'..'6' */
        state = statelbno[ch - '1'];
        break;
      case '[':	/* ESC '[' '[' */
        state = 10;
        break;
      default:
        ok = FALSE;
      }
      break;
    case 4:	/* ESC '[' '1' read */
      if (ch >= '1' && ch <= '9') {	/* ESC '[' '1' '1'..'9' */
        newstate = state1[ch - '1'];
        if (newstate == 0)	/* Bad character */
          ok = FALSE;
        else {
          state = newstate;
        }
      }
      else if (ch == '~') 	/* ESC '[' '1 '~' - 'Home' key */
        return HOME;
      else {
        ok = FALSE;
      }
      break;
    case 5:	/* ESC '[' '2' read */
      if (ch >= '0' && ch <= '9') {	/* ESC '[' '2' '0'..'9' */
        newstate = state2[ch - '0'];
        if (newstate == 0)	/* Bad character */
          ok = FALSE;
        else {
          state = newstate;
        }
      }
      else if (ch == '~') {	/* ESC '[' '2' '~' - 'Insert' key */
        push_key(INSERT);
        return NUL;
      }
      else {
        ok = FALSE;
      }
      break;
    case 6:	/* ESC '[' '3' read */
      if (ch >= '1' && ch <= '4') {	/* ESC '[' '3' '1'..'4' */
        newstate = state3[ch - '1'];
        if (newstate == 0)	/* Bad character */
          ok = FALSE;
        else {
          state = newstate;
        }
      }
      else if (ch == '~')
        return CTRL_H;	/* ESC '[' '3' '~' - 'Del' key */
      else {
        ok = FALSE;
      }
      break;
    case 7:	/* ESC '[' '4' read */
      if (ch == '~') {	/* ESC '[' '4' '~' - 'End' key */
        push_key(END);
        return NUL;
      }
      ok = FALSE;
      break;
    case 8:	/* ESC '[' '5' read */
      if (ch == '~') {	/* ESC '[' '5' '~' - 'Page up' key */
        push_key(PGUP);
        return NUL;
      }
      ok = FALSE;
      break;
    case 9:	/* ESC '[' '6' read */
      if (ch == '~') {	/* ESC '[' '6' '~' - 'Page down' key */
        push_key(PGDOWN);
        return NUL;
      }
      ok = FALSE;
      break;
    case 10:	/* ESC '[' '[' read */
      if (ch >= 'A' && ch <= 'E') {	/* ESC '[' '[' 'A'..'E' -  Linux F1..F5 */
        push_key(ch - 'A' + KEY_F1);
        return NUL;
      }
      ok = FALSE;
      break;
    case 11: case 12: case 13: case 14:	/* ESC '[' '1' '1'..'4' */
    case 15: case 16: case 17: case 18:	/* ESC '[' '1' '5'..'9' */
    case 19: case 20:				/* ESC '[' '2' '0'..'1' */
    case 21: case 22: case 23: case 24:	/* ESC '[' '2' '3'..'6' */
    case 25: case 26:				/* ESC '[' '2' '8'..'9' */
    case 27: case 28: case 29: case 30:	/* ESC '[' '3' '1'..'4' */
      if (ch == '~') {
        push_key(state2key[state - 11]);
        return NUL;
      }
      ok = FALSE;
    }
  }
/*
** Incomplete or bad sequence found. If it is bad then 'ok' will be set to
** 'false'. If incomplete, 'ok' will be 'true'. 'ch' will be undefined
** in this case.
*/
  if (!ok) push_key(ch);
  switch (state) {
  case 1:	/* ESC read */
    return ESCAPE;
  case 2:	/* ESC 'O' read */
    push_key('O');
    return ESCAPE;
  case 3:	/* ESC '[' read */
    break;
  case 4: case 5: case 6: case 7:
  case 8: case 9:	/* ESC '[' '1'..'6' read */
    push_key('1' + state - 4);
    break;
  case 10:	/* ESC '[' '[' read */
    push_key('[');
    break;
  case 11: case 12: case 13: case 14: case 15:	/* ESC '[' '1' '1'..'5' read */
    push_key(1 + state - 11);
    push_key('1');
    break;
  case 16: case 17: case 18:	/* ESC '[' '1' '7'..'9' read */
    push_key('7' + state - 16);
    push_key('1');
    break;
  case 19: case 20:			/* ESC '[' '2' '0'..'1' read */
    push_key('0' + state - 19);
    push_key('2');
    break;
  case 21: case 22: case 23: case 24:	/* ESC '[' '2' '3'..'6' read */
    push_key('3' + state - 21);
    push_key('2');
    break;
  case 25: case 26:			/* ESC '[' '2' '8'..'9' read */
    push_key('8' + state - 25);
    push_key('2');
  case 27: case 28: case 29: case 30:	/* ESC '[' '3' '1'..'4' */
    push_key('1' + state - 27);
    push_key(3);
  }
  push_key('[');
  return ESCAPE;
}

/*
** 'init_keyboard' initialises the keyboard code. It checks to
** see if stdin is connected to a keyboard. If it is then the
** function changes stdin to use unbuffered I/O so that the
** individual key presses can be read. This is not done if
** stdin is pointing elsewhere as it is assumed that input is
** most likely being taken from a file.
*/
boolean init_keyboard(void) {
  int n, errcode;
  for (n = 0; n < FN_KEY_COUNT; n++) fn_key[n].text = NIL;
  fn_string_count = 0;
  fn_string = NIL;
  holdcount = 0;
  histindex = 0;
  highbuffer = 0;
  enable_insert = TRUE;
  set_cursor(enable_insert);

  return TRUE;
}

void end_keyboard(void) {
}

/*
** 'emulate_get' emulates the Basic function 'get'. It is also the
** input routine used when reading a line from the keyboard
*/
int32 emulate_get(void) {
  byte ch;
  int32 key, fn_keyno;
  if (basicvars.runflags.inredir) error(ERR_UNSUPPORTED);	/* Not reading from the keyboard */
/*
 * Check if characters are being taken from a function
 * key string and if so return the next one
 */
  if (fn_string != NIL) return read_fn_string();
  if (holdcount > 0) return pop_key();	/* Return character from hold stack if one is present */
  ch = read_key();
  ch = ch & BYTEMASK;
  if (ch != ESCAPE) return ch;
/*
 * Either ESC was pressed or it marks the start of an ANSI
 * escape sequence for a function key, cursor key or somesuch.
 * Try to make sense of what follows. If function key was
 * pressed, check to see if there is a function key string
 * associated with it and return the first character of
 * the string otherwise return a NUL. (The next character
 * returned will be the RISC OS key code in this case)
 */
  key = decode_sequence();
  if (key != NUL) return key;
/* NUL found. Check for function key */
  key = pop_key();
  fn_keyno = is_fn_key(key);
  if (fn_keyno == 0) {	/* Not a function key - Return NUL then key code */
    push_key(key);
    return NUL;
  }
/* Function key hit. Check if there is a function key string */
  if (fn_key[fn_keyno].text == NIL) {	/* No string is defined for this key */
    push_key(key);
    return NUL;
  }
/*
 * There is a function key string. Switch input to string
 * and return the first character
 */
  return switch_fn_string(fn_keyno);
}

/*
** 'emulate_inkey' emulates the Basic function 'inkey'. Only the 'timed wait'
** and 'OS version' flavours of the function are supported.
** Note that the behaviour of the RISC OS version of INKEY with a +ve argument
** appears to be undefined if the wait exceeds 32767 centiseconds.
*/
int32 emulate_inkey(int32 arg) {
  if (arg >= 0) {	/* Timed wait for a key to be hit */
    if (basicvars.runflags.inredir) error(ERR_UNSUPPORTED);	/* There is no keyboard to read */
    if (arg > INKEYMAX) arg = INKEYMAX;	/* Wait must be in range 0..32767 centiseconds */
    if (waitkey(arg))
      return emulate_get();	/* Fetch the key if one is available */
    else {
      return -1;	/* Otherwise return -1 to say that nothing arrived in time */
    }
  }
  else if (arg == -256)		/* Return version of operating system */
    return OSVERSION;
  else {	/* Check is a specific key is being pressed */
    error(ERR_UNSUPPORTED);	/* Check for specific key is unsupported */
  }
  return 0;
}

/*
** 'display' outputs the given character 'count' times
** special case for VDU_CURBACK because it doesn't
** work if echo is off
*/
static void display(int32 what, int32 count) {
  if ( what != VDU_CURBACK) echo_off();
  while (count > 0) {
    emulate_vdu(what);
    count--;
  }
  if ( what != VDU_CURBACK) echo_on();
}

/*
** 'remove_history' removes entries from the command history buffer.
** The first 'count' entries are deleted. The function also takes
** care of updating 'highbuffer' and 'histindex'
*/
static void remove_history(int count) {
  int n, freed;
  freed = 0;
  for (n = 0; n < count; n++) freed+=histlength[n];
  if (count < histindex) {	/* Not deleting everything - Move entries down */
    memmove(histbuffer, &histbuffer[freed], highbuffer-freed);
    for (n = count; n < histindex; n++) histlength[n-count] = histlength[n];
  }
  highbuffer-=freed;
  histindex-=count;
}

/*
** 'add_history' adds an entry to the command history buffer.
** The new command is added to the end of the buffer. If there is not
** enough room for it, one or more commands are removed from the front
** of the buffer to make space for it. Also, if the maximum number of
** entries has been reached, an entry is dropped off the front of the
** buffer.
** 'cmdlen' is the length of the command. It does not include the NULL
** at the end
*/
static void add_history(char command[], int32 cmdlen) {
  int32 wanted, n, freed;
  if (highbuffer+cmdlen >= HISTSIZE) {	/* There is not enough room at the end of the buffer */
    wanted = highbuffer+cmdlen-HISTSIZE+1;	/* +1 for the NULL at the end of the command */
/*
** Figure out how many commands have to be removed from the buffer to make
** room for the new one. Scan from the start of the history list adding up
** the lengths of the commands in the history buffer until the total equals
** or exceeds the number of characters required. Entries from 0 to n-1 have
** to be deleted.
*/
    freed = 0;
    n = 0;
    do {
      freed += histlength[n];
      n++;
    } while (n < histindex && freed < wanted);
    remove_history(n);
  }
  else if (histindex == MAXHIST) {	/* History list is full */
    remove_history(1);	/* Delete the first entry */
  }
  memmove(&histbuffer[highbuffer], command, cmdlen+1);
  histlength[histindex] = cmdlen+1;
  highbuffer += cmdlen+1;
  histindex += 1;
}

static void init_recall(void) {
  recalline = histindex;
}

static void recall_histline(char buffer[], int updown) {
  int n, start, count;
  if (updown < 0) {	/* Move backwards in history list */
    if (recalline == 0) return;	/* Already at start of list */
    recalline -= 1;
  }
  else {	/* Move forwards in history list */
    if (recalline == histindex) return;	/* Already at end of list */
    recalline += 1;
  }
  if (recalline == histindex)	/* Moved to last line */
    buffer[0] = NUL;
  else {
    start = 0;
    for (n = 0; n < recalline; n++) start += histlength[n];
    strcpy(buffer, &histbuffer[start]);
  }
  display(VDU_CURBACK, place);		/* Move cursor to start of old line */
  place = strlen(buffer);
  if (place > 0) emulate_vdustr(buffer, place);
  count = highplace - place;	/* Find difference in old and new line lengths */
  if (count > 0) {	/* Some of the old line is still visible */
    display(' ', count);
    display(VDU_CURBACK, count);
  }
  highplace = place;
}

/*
** 'shift_down' moves all the characters in the buffer down by one
** overwriting the character at 'offset'. It also redraws the line on
** the screen, leaving the cursor at the postion of 'offset'
*/
static void shift_down(char buffer[], int32 offset) {
  int32 count;
  count = highplace-offset;	/* Number of characters by which to move cursor */
  highplace--;
  echo_off();
  while (offset < highplace) {
    buffer[offset] = buffer[offset+1];
    emulate_vdu(buffer[offset]);
    offset++;
  }
  emulate_vdu(DEL);
  echo_on();
  display(VDU_CURBACK, count);  /* Move cursor back to correct position */
}

/*
** 'shift_up' moves the text from 'offset' along by one character to
** make room for a new character, rewriting the line on the screen as
** well. It leaves the cursor at the screen position for the new
** character. The calling routine has to check that there is room
** for another character
*/
static void shift_up(char buffer[], int32 offset) {
  int32 n;
  if (offset == highplace) return;	/* Appending char to end of line */
  n = highplace;
  while (n >= offset+1) {
    buffer[n] = buffer[n-1];
    n--;
  }
  echo_off();
  emulate_vdu(DEL);	/* Where new character goes on screen */
  n = offset+1;
  while (n <= highplace) {
    emulate_vdu(buffer[n]);
    n++;
  }
  echo_on();
  while (n > offset) {	/* Put cursor back where it should be */
    emulate_vdu(VDU_CURBACK);
    n--;
  }
  highplace++;	/* Bump up 'last character' index */
}

/*
** 'emulate_readline' reads a line from the keyboard. It returns 'true'
** if the call worked successfully or 'false' if 'escape' was pressed.
** The data input is stored at 'buffer'. Up to 'length'-1 characters
** can be read. A 'null' is added after the last character. 'buffer'
** can be prefilled with characters if required to allow existing text
** to be edited.
**
** This function uses only the most basic facilities of the underlying
** OS to carry out its task (which counts as everything under DOS) so
** much of this code is ugly.
**
** The function provides both DOS and Unix style line editing facilities,
** for example, both 'HOME' and control 'A' move the cursor to the start
** of the line. It is also possible to recall the last line entered.
**
** There is a problem with LCC-WIN32 running under Windows 98 in that the
** extended key codes for the cursor movement keys (insert, left arrow
** and so on) are not returned correctly by 'getch()'. In theory they
** should appear as a two byte sequence of 0xE0 followed by the key code.
** Only the 0xE0 is returned. This appears to be a bug in the C runtime
** library.
*/
readstate emulate_readline(char buffer[], int32 length) {
  int32 ch, lastplace;
  if (basicvars.runflags.inredir) {	/* There is no keyboard to read - Read fron file stdin */
    char *p;
    p = fgets(buffer, length, stdin);
    if (p == NIL) {	/* Call failed */
      if (ferror(stdin)) error(ERR_READFAIL);	/* I/O error occured on stdin */
      buffer[0] = NUL;		/* End of file */
      return READ_EOF;
    }
    return READ_OK;
  }
  highplace = strlen(buffer);
  if (highplace > 0) emulate_vdustr(buffer, highplace);
  place = highplace;
  lastplace = length-2;		/* Index of last position that can be used in buffer */
  init_recall();
  do {
    ch = emulate_get();
    if (basicvars.escape) return READ_ESC;	/* Check if the escape key has been pressed and bail out if it has */
    switch (ch) {	/* Normal keys */
    case CR: case LF:	/* End of line */
      emulate_vdu('\r');
      emulate_vdu('\n');
      buffer[highplace] = NUL;
      if (highplace > 0) add_history(buffer, highplace);
      break;
    case CTRL_H: case DEL:	/* Delete character to left of cursor */
      if (place > 0) {
        emulate_vdu(VDU_CURBACK);
        place--;
        shift_down(buffer, place);
      }
      break;
    case CTRL_D:	/* Delete character under the cursor */
      if (place < highplace) shift_down(buffer, place);
      break;
    case CTRL_K:	/* Delete from cursor position to end of line */
      display(DEL, highplace-place);		/* Clears characters after cursor */
      display(VDU_CURBACK, highplace-place);		/* Now move cursor back */
      highplace = place;
      break;
    case CTRL_U:	/* Delete whole input line */
      display(VDU_CURBACK, place);	/* Move cursor to start of line */
      display(DEL, highplace);	/* Overwrite text with blanks */
      display(VDU_CURBACK, highplace);	/* Move cursor back to start of line */
      highplace = place = 0;
      break;
    case CTRL_B:	/* Move cursor left */
      if (place > 0) {
        emulate_vdu(VDU_CURBACK);
        place--;
      }
      break;
    case CTRL_F:	/* Move cursor right */
      if (place < highplace) {
        emulate_vdu(buffer[place]);	/* It does the job */
        place++;
      }
      break;
    case CTRL_P:	/* Move backwards one entry in the history list */
      recall_histline(buffer, -1);
      break;
    case CTRL_N:	/* Move forwards one entry in the history list */
      recall_histline(buffer, 1);
      break;
    case CTRL_A:	/* Move cursor to start of line */
      display(VDU_CURBACK, place);
      place = 0;
      break;
    case CTRL_E:		/* Move cursor to end of line */
      echo_off();
      while (place < highplace) {
        emulate_vdu(buffer[place]);	/* It does the job */
        place++;
      }
      echo_on();
      break;
    case HOME:	/* Move cursor to start of line */
      display(VDU_CURBACK, place);
      place = 0;
      break;
    case NUL:			/* Function or special key follows */
      ch = emulate_get();	/*Fetch the key details */
      switch (ch) {
      case END:		/* Move cursor to end of line */
        echo_off();
        while (place < highplace) {
          emulate_vdu(buffer[place]);	/* It does the job */
          place++;
        }
        echo_on();
        break;
      case UP:		/* Move backwards one entry in the history list */
        recall_histline(buffer, -1);
        break;
      case DOWN:	/* Move forwards one entry in the history list */
        recall_histline(buffer, 1);
        break;
      case LEFT:	/* Move cursor left */
        if (place > 0) {
          emulate_vdu(VDU_CURBACK);
          place--;
        }
        break;
      case RIGHT:	/* Move cursor right */
        if (place < highplace) {
          emulate_vdu(buffer[place]);	/* It does the job */
          place++;
        }
        break;
      case DELETE:	/* Delete character at the cursor */
        if (place < highplace) shift_down(buffer, place);
        break;
      case INSERT:	/* Toggle between 'insert' and 'overwrite' mode */
        enable_insert = !enable_insert;
        set_cursor(enable_insert);	/* Change cursor if in graphics mode */
        break;
      default:
        emulate_vdu(VDU_BEEP);	/* Bad character - Ring the bell */
      }
      break;
    default:
      if (ch < ' ' && ch != TAB)	/* Reject any other control character except tab */
        emulate_vdu(VDU_BEEP);
      else if (highplace == lastplace)	/* No room left in buffer */
        emulate_vdu(VDU_BEEP);
      else {	/* Add character to buffer */
        if (enable_insert) shift_up(buffer, place);
        buffer[place] = ch;
        emulate_vdu(ch);
        place++;
        if (place > highplace) highplace = place;
      }
    }
  } while (ch != CR && ch != LF);
  return READ_OK;
}
