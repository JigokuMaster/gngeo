#ifndef GN_KEYMAP_H
#define GN_KEYMAP_H

/*
 * Keyboard mapping for GnGeo (flat binding list).
 *
 * One binding = (player, button, key). Any number of bindings may exist, so
 * one key can drive several buttons and one button can have several keys.
 * Only exact duplicates (same player+button+key) are rejected.
 *
 * Fixed-size storage, no heap use (friendly to S60v3 memory).
 */

#include <stdbool.h>
#include "SDL.h"
#include "event.h"          /* GNGEO_BUTTON, joy_state */

#define KM_P1    1
#define KM_P2    2
#define KM_BOTH  3          /* bitmask: bit0 = P1, bit1 = P2 */

#define KM_MAX_BINDINGS 128

typedef struct {
	Uint8        player;    /* KM_P1 / KM_P2 / KM_BOTH */
	GNGEO_BUTTON button;
	SDLKey       key;
} KeyBinding;

/* ---- button names (for config + GUI) ---- */
const char  *keymap_button_name(GNGEO_BUTTON b);   /* config token: "A", "COIN", "UP" */
const char  *keymap_button_label(GNGEO_BUTTON b);  /* GUI text: "Button A", "Coin" */
GNGEO_BUTTON keymap_button_from_name(const char *name); /* GN_NONE if unknown; accepts aliases */
const char  *keymap_key_name(SDLKey key);          /* SDL key name, e.g. "up" */

/* ---- editing ---- */
void keymap_clear(void);
bool keymap_add(int player, GNGEO_BUTTON b, SDLKey key);    /* false: dup / full / invalid */
bool keymap_remove(int player, GNGEO_BUTTON b, SDLKey key);
void keymap_remove_button(int player, GNGEO_BUTTON b);      /* drop all keys of a button */
bool keymap_parse(int player, const char *conf);            /* "A=K50,B=K56,..." (adds); "NAME=K0" = button deliberately unmapped */
int  keymap_serialize(int player, char *buf, int size);     /* inverse of parse (writes NAME=K0 for emptied buttons); returns length */
void keymap_apply_defaults(int player);                     /* only for buttons with no key and no "NAME=K0" mark */

/* ---- queries (GUI editor) ---- */
int               keymap_count(void);
const KeyBinding *keymap_get(int index);
/* keys bound to a button of `player` (P1 or P2). Returns number written. */
int  keymap_keys_for_button(int player, GNGEO_BUTTON b, SDLKey *out, int max);
/* buttons a key triggers for `player`. Returns number written. */
int  keymap_buttons_for_key(int player, SDLKey key, GNGEO_BUTTON *out, int max);
bool keymap_button_is_mapped(int player, GNGEO_BUTTON b);
bool keymap_key_is_bound(SDLKey key);                       /* any player/button */
bool keymap_key_has_button(SDLKey key, GNGEO_BUTTON b);     /* any player */

/* ---- runtime ---- */
void keymap_key_event(SDLKey key, bool down);  /* updates joy_state */
void keymap_reset_state(void);

#endif
