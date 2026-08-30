#ifndef _X16_H_
	#define _X16_H_

static int tsb_done = 0;

static unsigned char x16_vram[2 * 65536];
static void b_vpoke(unsigned int bank, unsigned int addr, unsigned char value) {
	if (bank > 1) bank = 1;
	addr &= 0xFFFF;
	x16_vram[(bank << 16) | addr] = value;
}
static int b_vpeek(unsigned int bank, unsigned int addr) {
	if (bank > 1) bank = 1;
	addr &= 0xFFFF;
	return x16_vram[(bank << 16) | addr];
}

static int x16_color_fg = 1, x16_color_bg = 0, x16_border = 14;
static void b_color(int fg, int bg, int border) {
	if (fg     >= 0) x16_color_fg = fg;
	if (bg     >= 0) x16_color_bg = bg;
	if (border >= 0) x16_border    = border;
}

static int x16_cursor_color = 0;
static unsigned char x16_sid[0x19];
static int x16_poke_intercept(unsigned short addr, unsigned char v) {
	if (addr >= 0x0400 && addr <= 0x07E7) { x16_vram[addr] = v; return 1; }
	if (addr >= 0xD800 && addr <= 0xDBE7) { x16_vram[addr] = v; return 1; }
	if (addr == 0xD020) { x16_border = v; return 1; }
	if (addr == 0xD021) { x16_color_bg = v; return 1; }
	if (addr == 0x0286) { x16_cursor_color = v; return 1; }
	if (addr >= 0xD000 && addr <= 0xD017) { x16_vram[0x1FC00 + (addr - 0xD000)] = v; return 1; }
	if (addr >= 0xD400 && addr <= 0xD418) { x16_sid[addr - 0xD400] = v; return 1; }
	return 0;
}
static int x16_peek_intercept(unsigned short addr, unsigned char *out) {
	if (addr >= 0x0400 && addr <= 0x07E7) { *out = x16_vram[addr]; return 1; }
	if (addr >= 0xD800 && addr <= 0xDBE7) { *out = x16_vram[addr]; return 1; }
	if (addr == 0xD020) { *out = (unsigned char)x16_border; return 1; }
	if (addr == 0xD021) { *out = (unsigned char)x16_color_bg; return 1; }
	if (addr == 0x0286) { *out = (unsigned char)x16_cursor_color; return 1; }
	if (addr >= 0xD000 && addr <= 0xD017) { *out = x16_vram[0x1FC00 + (addr - 0xD000)]; return 1; }
	if (addr >= 0xD400 && addr <= 0xD418) { *out = x16_sid[addr - 0xD400]; return 1; }
	return 0;
}

static void b_rect(int x, int y, int w, int h, int color) {
	(void)x;
	(void)y;
	(void)w;
	(void)h;
	(void)color;
}

static bstr_t b_linput(void) {
	char buf[B_INPUT_BUF_SIZE];
	int n = b_getline(buf, (int)sizeof(buf));
	return b_keep(b_dup(buf, n));
}

static void b_mouse(int on) { (void)on; }

static void b_frame(void) {}

static void b_bank(int bank) { (void)bank; }

static void b_psgwav(int voice, int wave) { (void)voice; (void)wave; }

static void b_fm(int ch, int freq, int instr) { (void)ch; (void)freq; (void)instr; }

static void tsb_init(void) { }

static void tsb_hold(void) { fflush(stdout); tsb_done = 1; }

static void tsb_hires(int ink, int paper) { (void)ink; (void)paper; }
static void tsb_nrm(void) {}
static void tsb_reset(void) {}
static void tsb_colour(int b, int bg, int pen) { (void)b; (void)bg; (void)pen; }
static void tsb_plot(int x, int y) { (void)x; (void)y; }
static void tsb_line(int x1, int y1, int x2, int y2, int f) { (void)x1; (void)y1; (void)x2; (void)y2; (void)f; }
static void tsb_rec(int x1, int y1, int x2, int y2, int f) { (void)x1; (void)y1; (void)x2; (void)y2; (void)f; }
static void tsb_block(int x1, int y1, int x2, int y2, int f) { (void)x1; (void)y1; (void)x2; (void)y2; (void)f; }
static void tsb_circle(int cx, int cy, int rx, int ry, int f) { (void)cx; (void)cy; (void)rx; (void)ry; (void)f; }
static void tsb_angl(int cx, int cy, int a, int rx, int ry, int f) { (void)cx; (void)cy; (void)a; (void)rx; (void)ry; (void)f; }
static void tsb_arc(int cx, int cy, int a1, int a2, int sd, int rx, int ry, int f) { (void)cx; (void)cy; (void)a1; (void)a2; (void)sd; (void)rx; (void)ry; (void)f; }
static void tsb_paint(int x, int y, int f) { (void)x; (void)y; (void)f; }
static void tsb_char(int x, int y, int code, int mode, int zoom) { (void)x; (void)y; (void)code; (void)mode; (void)zoom; }
static void tsb_text(int x, int y, const char* s, int len, int mode, int zoom, int kern) { (void)x; (void)y; (void)s; (void)len; (void)mode; (void)zoom; (void)kern; }
static void tsb_rot(int angle, int step) { (void)angle; (void)step; }
static void tsb_draw(const char* s, int len, int x, int y, int f) { (void)s; (void)len; (void)x; (void)y; (void)f; }
static void tsb_drawto(int x, int y, int f) { (void)x; (void)y; (void)f; }
static void tsb_multi_on(int on) { (void)on; }
static void tsb_multi_cols(int c1, int c2, int c3) { (void)c1; (void)c2; (void)c3; }
static void tsb_cls(void) {}
static void tsb_fill(int row, int col, int w, int h, int ch, int color) { (void)row; (void)col; (void)w; (void)h; (void)ch; (void)color; }
static void tsb_fchr(int row, int col, int w, int h, int ch) { (void)row; (void)col; (void)w; (void)h; (void)ch; }
static void tsb_fcol(int row, int col, int w, int h, int color) { (void)row; (void)col; (void)w; (void)h; (void)color; }
static void tsb_inv(int row, int col, int w, int h) { (void)row; (void)col; (void)w; (void)h; }
static void tsb_at(int x, int y) { (void)x; (void)y; }
static void tsb_centre(const char* s, int len, int width) { (void)s; (void)len; (void)width; }
static void tsb_scroll(int row, int col, int w, int h, int dir, int trow, int tcol) { (void)row; (void)col; (void)w; (void)h; (void)dir; (void)trow; (void)tcol; }
static void tsb_mob(int n, int on) { (void)n; (void)on; }
static void tsb_mobcol(int mob, int col) { (void)mob; (void)col; }
static void tsb_mobset(int spr, int block, int color, int prio, int type, int exp) { (void)spr; (void)block; (void)color; (void)prio; (void)type; (void)exp; }
static void tsb_cmob(int c1, int c2) { (void)c1; (void)c2; }
static void tsb_mmob(int spr, int x1, int y1, int x2, int y2, int speed) { (void)spr; (void)x1; (void)y1; (void)x2; (void)y2; (void)speed; }
static void tsb_rlocmob(int spr, int x, int y, int speed) { (void)spr; (void)x; (void)y; (void)speed; }
static void tsb_pause(int n) { (void)n; }

static void tsb_sound(int voice, int freq) { (void)voice; (void)freq; }
static void tsb_wave(int voice, int wf, int pulse) { (void)voice; (void)wf; (void)pulse; }
static void tsb_envelope(int voice, int a, int d, int s, int r) { (void)voice; (void)a; (void)d; (void)s; (void)r; }
static void tsb_vol(int v) { (void)v; }

#endif
