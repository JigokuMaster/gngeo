#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "keymap.h"

/* ------------------------------------------------------------------ */
/* Button table, indexed by GNGEO_BUTTON (keep in enum order)          */
/* ------------------------------------------------------------------ */
typedef struct { const char *name; const char *label; } ButtonInfo;

static const ButtonInfo button_info[GN_MAX_KEY] = {
	/* GN_NONE        */ {"NONE",      "None"},
	/* GN_A           */ {"A",         "Button A"},
	/* GN_B           */ {"B",         "Button B"},
	/* GN_C           */ {"C",         "Button C"},
	/* GN_D           */ {"D",         "Button D"},
	/* GN_UP          */ {"UP",        "Up"},
	/* GN_DOWN        */ {"DOWN",      "Down"},
	/* GN_LEFT        */ {"LEFT",      "Left"},
	/* GN_RIGHT       */ {"RIGHT",     "Right"},
	/* GN_START       */ {"START",     "Start"},
	/* GN_SELECT_COIN */ {"COIN",      "Coin"},
	/* GN_SAVESTATE   */ {"SAVESTATE", "Save state"},
	/* GN_LOADSTATE   */ {"LOADSTATE", "Load state"},
	/* GN_MENU_KEY    */ {"MENU",      "Menu"},
	/* GN_HOTKEY1     */ {"HOTKEY1",   "Hotkey 1"},
	/* GN_HOTKEY2     */ {"HOTKEY2",   "Hotkey 2"},
	/* GN_HOTKEY3     */ {"HOTKEY3",   "Hotkey 3"},
	/* GN_HOTKEY4     */ {"HOTKEY4",   "Hotkey 4"},
};

/* legacy config names, parse-only (serialize always writes canonical names) */
static const struct { const char *name; GNGEO_BUTTON id; } button_alias[] = {
	{"UPDOWN",    GN_UP},
	{"LEFTRIGHT", GN_LEFT},
	{"JOY",       GN_UP},
};

const char *keymap_button_name(GNGEO_BUTTON b)
{
	return (b > GN_NONE && b < GN_MAX_KEY) ? button_info[b].name : "NONE";
}

const char *keymap_button_label(GNGEO_BUTTON b)
{
	return (b > GN_NONE && b < GN_MAX_KEY) ? button_info[b].label : "None";
}

GNGEO_BUTTON keymap_button_from_name(const char *name)
{
	int i;
	if (!name) return GN_NONE;
	for (i = GN_NONE + 1; i < GN_MAX_KEY; i++)
		if (!strcmp(name, button_info[i].name)) return (GNGEO_BUTTON)i;
	for (i = 0; i < (int)(sizeof(button_alias) / sizeof(button_alias[0])); i++)
		if (!strcmp(name, button_alias[i].name)) return button_alias[i].id;
	return GN_NONE;
}

const char *keymap_key_name(SDLKey key)
{
	return SDL_GetKeyName(key);
}

/* ------------------------------------------------------------------ */
/* Storage                                                             */
/* ------------------------------------------------------------------ */
static KeyBinding bindings[KM_MAX_BINDINGS];
static int        nbindings = 0;
static Uint8      keys_down[SDLK_LAST];   /* physical key state */
/* Buttons the config marks as deliberately empty ("NAME=K0"): keymap_apply_defaults()
 * must not refill them. Index: [player - 1][button]. */
static Uint8      explicit_empty[2][GN_MAX_KEY];

static int valid_player(int p) { return p >= KM_P1 && p <= KM_BOTH; }
static int valid_button(GNGEO_BUTTON b) { return b > GN_NONE && b < GN_MAX_KEY; }

void keymap_clear(void)
{
	nbindings = 0;
	memset(keys_down, 0, sizeof(keys_down));
	memset(explicit_empty, 0, sizeof(explicit_empty));
}

int keymap_count(void) { return nbindings; }

const KeyBinding *keymap_get(int index)
{
	return (index >= 0 && index < nbindings) ? &bindings[index] : NULL;
}

bool keymap_add(int player, GNGEO_BUTTON b, SDLKey key)
{
	int i;
	if (!valid_player(player) || !valid_button(b)) return false;
	if ((int)key < 0 || (int)key >= SDLK_LAST) return false;
	if (nbindings >= KM_MAX_BINDINGS) return false;
	for (i = 0; i < nbindings; i++)
		if (bindings[i].player == player && bindings[i].button == b && bindings[i].key == key)
			return false;   /* exact duplicate */
	bindings[nbindings].player = (Uint8)player;
	bindings[nbindings].button = b;
	bindings[nbindings].key    = key;
	nbindings++;
	return true;
}

static void remove_at(int i)
{
	memmove(&bindings[i], &bindings[i + 1], (nbindings - i - 1) * sizeof(KeyBinding));
	nbindings--;
}

bool keymap_remove(int player, GNGEO_BUTTON b, SDLKey key)
{
	int i;
	for (i = 0; i < nbindings; i++) {
		if (bindings[i].player == player && bindings[i].button == b && bindings[i].key == key) {
			remove_at(i);
			return true;
		}
	}
	return false;
}

void keymap_remove_button(int player, GNGEO_BUTTON b)
{
	int i = 0;
	while (i < nbindings) {
		if (bindings[i].button == b && (bindings[i].player & player))
			remove_at(i);
		else
			i++;
	}
}

/* ------------------------------------------------------------------ */
/* Queries                                                             */
/* ------------------------------------------------------------------ */
int keymap_keys_for_button(int player, GNGEO_BUTTON b, SDLKey *out, int max)
{
	int i, n = 0;
	for (i = 0; i < nbindings && n < max; i++)
		if (bindings[i].button == b && (bindings[i].player & player))
			out[n++] = bindings[i].key;
	return n;
}

int keymap_buttons_for_key(int player, SDLKey key, GNGEO_BUTTON *out, int max)
{
	int i, n = 0;
	for (i = 0; i < nbindings && n < max; i++)
		if (bindings[i].key == key && (bindings[i].player & player))
			out[n++] = bindings[i].button;
	return n;
}

bool keymap_button_is_mapped(int player, GNGEO_BUTTON b)
{
	int i;
	for (i = 0; i < nbindings; i++)
		if (bindings[i].button == b && (bindings[i].player & player)) return true;
	return false;
}

bool keymap_key_is_bound(SDLKey key)
{
	int i;
	for (i = 0; i < nbindings; i++)
		if (bindings[i].key == key) return true;
	return false;
}

bool keymap_key_has_button(SDLKey key, GNGEO_BUTTON b)
{
	int i;
	for (i = 0; i < nbindings; i++)
		if (bindings[i].key == key && bindings[i].button == b) return true;
	return false;
}

/* ------------------------------------------------------------------ */
/* Config parse / serialize                                            */
/* ------------------------------------------------------------------ */
static void parse_token(int player, char *tok)
{
	char *eq = strchr(tok, '=');
	char *end;
	long code;
	GNGEO_BUTTON id;

	if (!eq || eq[1] != 'K') return;      /* only keyboard entries ("K<code>") */
	*eq = 0;
	code = strtol(eq + 2, &end, 10);
	if (end == eq + 2) return;
	id = keymap_button_from_name(tok);
	if (id == GN_NONE) return;
	if (code == 0)   /* K0 (SDLK_UNKNOWN) = button deliberately left empty, not a real binding */
	{
		if (player & KM_P1) explicit_empty[0][id] = 1;
		if (player & KM_P2) explicit_empty[1][id] = 1;
		return;
	}
	keymap_add(player, id, (SDLKey)code);
}

bool keymap_parse(int player, const char *conf)
{
	char tok[64];
	const char *p = conf;

	if (!conf || !*conf) return false;
	while (*p) {
		const char *comma = strchr(p, ',');
		size_t len = comma ? (size_t)(comma - p) : strlen(p);
		if (len > 0 && len < sizeof(tok)) {
			memcpy(tok, p, len);
			tok[len] = 0;
			parse_token(player, tok);
		}
		p = comma ? comma + 1 : p + len;
	}
	return true;
}

static bool button_has_default(GNGEO_BUTTON b);

static bool append_token(char *buf, int size, int *len, const char *name, int key)
{
	int n = snprintf(buf + *len, size - *len, "%s%s=K%d", *len ? "," : "", name, key);
	if (n < 0 || n >= size - *len) { buf[*len] = 0; return false; }   /* truncated */
	*len += n;
	return true;
}

/* Writes "A=K50,B=K56,..." for the given player (KM_P1 or KM_P2).
 * A button that has a default key but no key of its own is written as "NAME=K0", so that
 * keymap_apply_defaults() does not refill it on the next start. */
int keymap_serialize(int player, char *buf, int size)
{
	int i, b, len = 0;
	if (size <= 0) return 0;
	buf[0] = 0;
	for (i = 0; i < nbindings; i++) {
		if (!(bindings[i].player & player)) continue;
		if (!append_token(buf, size, &len, keymap_button_name(bindings[i].button), (int)bindings[i].key))
			return len;
	}
	for (b = GN_NONE + 1; b < GN_MAX_KEY; b++) {
		if (!button_has_default((GNGEO_BUTTON)b) || keymap_button_is_mapped(player, (GNGEO_BUTTON)b)) continue;
		if (!append_token(buf, size, &len, keymap_button_name((GNGEO_BUTTON)b), 0))
			return len;
	}
	return len;
}

/* ------------------------------------------------------------------ */
/* Defaults                                                            */
/* ------------------------------------------------------------------ */
static const struct { GNGEO_BUTTON b; SDLKey key; } default_keys[] = {
	{GN_A, SDLK_2}, {GN_B, SDLK_8}, {GN_C, SDLK_4}, {GN_D, SDLK_6},
	{GN_START, SDLK_5}, {GN_START, SDLK_RETURN},
	{GN_UP, SDLK_UP}, {GN_DOWN, SDLK_DOWN}, {GN_LEFT, SDLK_LEFT}, {GN_RIGHT, SDLK_RIGHT},
	{GN_SELECT_COIN, SDLK_1},
	/*{GN_SAVESTATE, SDLK_s}, {GN_LOADSTATE, SDLK_l},*/
};

static bool button_has_default(GNGEO_BUTTON b)
{
	int i;
	for (i = 0; i < (int)(sizeof(default_keys) / sizeof(default_keys[0])); i++)
		if (default_keys[i].b == b) return true;
	return false;
}

void keymap_apply_defaults(int player)
{
	int i;
	const int pidx = (player == KM_P2) ? 1 : 0;
	bool missing[GN_MAX_KEY];
	const int n = (int)(sizeof(default_keys) / sizeof(default_keys[0]));

	/* decide per button first, so multi-key defaults (START) stay together */
	memset(missing, 0, sizeof(missing));
	for (i = 0; i < n; i++)
		if (!keymap_button_is_mapped(player, default_keys[i].b) && !explicit_empty[pidx][default_keys[i].b])
			missing[default_keys[i].b] = true;
	for (i = 0; i < n; i++)
		if (missing[default_keys[i].b])
			keymap_add(player, default_keys[i].b, default_keys[i].key);
}

/* ------------------------------------------------------------------ */
/* Runtime: key events -> joy_state                                    */
/* ------------------------------------------------------------------ */

/* A button is down for a player if ANY key bound to it is still held.
 * This is what makes two keys on the same button (START=K53,START=K13)
 * behave correctly when one is released while the other is held. */
static void refresh_button(int playermask, GNGEO_BUTTON b)
{
	int p, i;
	for (p = 0; p < 2; p++) {
		Uint8 down = 0;
		if (!(playermask & (1 << p))) continue;
		for (i = 0; i < nbindings && !down; i++)
			if (bindings[i].button == b && (bindings[i].player & (1 << p)) && keys_down[bindings[i].key])
				down = 1;
		joy_state[p][b] = down;
	}
}

void keymap_key_event(SDLKey key, bool down)
{
	int i;
	if ((int)key < 0 || (int)key >= SDLK_LAST) return;
	keys_down[key] = down ? 1 : 0;
	for (i = 0; i < nbindings; i++)
		if (bindings[i].key == key)
			refresh_button(bindings[i].player, bindings[i].button);
}

void keymap_reset_state(void)
{
	memset(keys_down, 0, sizeof(keys_down));
}
