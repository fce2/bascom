#ifndef _TSB_H_
	#define _TSB_H_
	#include <string.h>

static unsigned char tsb_chargen[4096];
static int tsb_chargen_loaded = 0;
static int tsb_chargen_found = 0;
static int tsb_no_rom_warned = 0;
static volatile int tsb_need_rom_warn = 0;

static unsigned char tsb_sintab[64];

static int tsb_mode = 0;
static volatile int tsb_dirty = 1;
static volatile int tsb_present_age = 1000;
static volatile int tsb_ever_yielded = 0;
static int tsb_fb = 0;
static volatile long tsb_present_count = 0;
static char tsb_progname[128] = "TSB";

static int tsb_lo = 0, tsb_hi = 199;
static int tsb_full = 1;
static void tsb_mark(int y) { if (y < tsb_lo) tsb_lo = y; if (y > tsb_hi) tsb_hi = y; }
static void tsb_inval(void) { tsb_full = 1; tsb_dirty = 1; }
static const signed char tsb_rottab[64] = {
	1, 0, 0,-1,  0,-1, 1, 0,
	1, 1,-1,-1,  1,-1, 1,-1,
	0, 1,-1, 0,  1, 0, 0,-1,
	-1, 1,-1, 1,  1, 1,-1,-1,
	-1, 0, 0, 1,  0, 1,-1, 0,
	-1,-1, 1, 1, -1, 1,-1, 1,
	0,-1, 1, 0, -1, 0, 0, 1,
	1,-1, 1,-1, -1,-1, 1, 1
};
static int tsb_ink = 1, tsb_paper = 0;
static int tsb_drx = 0, tsb_dry = 0;
static int tsb_rotgr = 1;
static signed char tsb_drawtab[8];
static unsigned char tsb_scrsv_buf[2000];
static int tsb_inited = 0;
static int tsb_loop_on_end = 0;
static int tsb_done = 0;
static void tsb_poll(void);
static void tsb_dump_bitmap(void);

static volatile unsigned char* tsb_pixaddr(int x, int y) {
	return b_mem + 0xE000 + (y >> 3) * 320 + (x >> 3) * 8 + (y & 7);
}
static unsigned short tsb_pixbyte(int x, int y) {
	return (unsigned short)(0xE000u + (unsigned)((y >> 3) * 320 + (x >> 3) * 8 + (y & 7)));
}
static unsigned char tsb_bmget(unsigned short a) {
		#if defined(TARGET_C64)
	unsigned char m = b_mem[1];
	if (m & 0x02) {
		unsigned char v;
		__asm
		{
			php
			sei
		}
		b_mem[1] = (unsigned char)(m & 0xFD);
		v = b_mem[a];
		b_mem[1] = m;
		__asm
		{
			plp
		}
		return v;
	}
		#endif
	return b_mem[a];
}

static void tsb_setpix(int x, int y, int f) {
	if (x < 0 || x >= 320 || y < 0 || y >= 200) return;
	unsigned char m = (unsigned char)(0x80 >> (x & 7));
	unsigned short a = tsb_pixbyte(x, y);
	unsigned char bv = tsb_bmget(a);
	if (f == 0) bv &= (unsigned char)~m;
	else if (f == 1) bv |= m;
	else bv ^= m;
	b_poke(a, bv);

	tsb_mark(y);
}

static void tsb_hires(int ink, int paper) {
	tsb_ink = ink & 15;
	tsb_paper = paper & 15;
	b_poke(0xD011, 0x3B);
	b_poke(0xD018, 0x08);
	b_poke(0xD016, 0xC8);
	b_poke(0xDD00, (unsigned char)(b_peek(0xDD00) & 0xFC));
	{ unsigned short a; for (a = 0xE000u; a < 0xE000u + 8000u; a++) b_poke(a, 0); }
	unsigned char mv = (unsigned char)((tsb_ink << 4) | tsb_paper);
	memset(b_mem + 0xC000, mv, 1000);
	tsb_mode = 1;
}
static void tsb_nrm(void) {
	b_poke(0xD011, 0x1B);
	b_poke(0xD018, 0x15);
	b_poke(0xD016, 0xC8);
	b_poke(0xDD00, (unsigned char)(b_peek(0xDD00) & 0xFC));
	b_poke(0x0288, 0x04);
	tsb_mode = 0;
	tsb_dirty = 1;
}
static void tsb_reset(void) {
	b_poke(0xD011, 0x1B);
	b_poke(0xD016, 0xC8);
	b_poke(0xD018, 0x15);
	b_poke(0xDD00, (unsigned char)(b_peek(0xDD00) | 3));
	b_poke(0xD020, 0x0B);
	b_poke(0xD021, 0x0C);
	b_poke(0x0286, 0x01);
	b_poke(0x0288, 0x04);
	tsb_mode = 0;
	tsb_dirty = 1;
}
static void tsb_colour(int b, int bg, int pen) {
	b_poke(0xD020, (unsigned char)b);
	b_poke(0xD021, (unsigned char)bg);
	if (pen) b_poke(0x0286, (unsigned char)pen);
	tsb_inval();
}

static void tsb_plot(int x, int y) { tsb_setpix(x, y, 1); tsb_dirty = 1; }

static void tsb_line(int x1, int y1, int x2, int y2, int f) {
	int dx = x2 - x1, dy = y2 - y1;
	int ax = dx < 0 ? -dx : dx, ay = dy < 0 ? -dy : dy;
	int sx = dx < 0 ? -1 : 1, sy = dy < 0 ? -1 : 1;
	int err = ax - ay, x = x1, y = y1;
	for (;;) {
		tsb_setpix(x, y, f);
		if (x == x2 && y == y2) break;
		int e2 = 2 * err;
		if (e2 > -ay) { err -= ay; x += sx; }
		if (e2 <  ax) { err += ax; y += sy; }
	}
	tsb_dirty = 1;
}
static void tsb_rec(int x1, int y1, int x2, int y2, int f) {
	tsb_line(x1, y1, x2, y1, f);
	tsb_line(x1, y2, x2, y2, f);
	tsb_line(x1, y1, x1, y2, f);
	tsb_line(x2, y1, x2, y2, f);
}
static void tsb_block(int x1, int y1, int x2, int y2, int f) {
	if (x1 > x2) { int t = x1; x1 = x2; x2 = t; }
	if (y1 > y2) { int t = y1; y1 = y2; y2 = t; }
	if (x2 < 0 || x1 >= 320 || y2 < 0 || y1 >= 200) return;
	if (x1 < 0) x1 = 0;
	if (y1 < 0) y1 = 0;
	if (x2 > 319) x2 = 319;
	if (y2 > 199) y2 = 199;
	for (int y = y1; y <= y2; y++) {
		int c1 = x1 >> 3, c2 = x2 >> 3;
		volatile unsigned char* row = b_mem + 0xE000 + (y >> 3) * 320 + c1 * 8 + (y & 7);
		if (c1 == c2) {
			unsigned char m = 0;
			for (int x = x1; x <= x2; x++) m |= (unsigned char)(0x80 >> (x & 7));
			if (f == 0) *row &= (unsigned char)~m;
			else if (f == 1) *row |= m;
			else *row ^= m;
		} else {
			unsigned char ml = 0, mr = 0;
			for (int x = x1; x < ((c1 + 1) << 3); x++) ml |= (unsigned char)(0x80 >> (x & 7));
			for (int x = (c2 << 3); x <= x2; x++) mr |= (unsigned char)(0x80 >> (x & 7));
			volatile unsigned char* pR = row + (c2 - c1) * 8;
			if (f == 0) { *row &= (unsigned char)~ml; for (volatile unsigned char* p = row + 8; p < pR; p += 8) *p = 0; *pR &= (unsigned char)~mr; }
			else if (f == 1) { *row |= ml; for (volatile unsigned char* p = row + 8; p < pR; p += 8) *p = 0xFF; *pR |= mr; }
			else { *row ^= ml; for (volatile unsigned char* p = row + 8; p < pR; p += 8) *p ^= 0xFF; *pR ^= mr; }
		}
	}
	tsb_dirty = 1;
}

static int tsb_pxget(int x, int y);

static void tsb_dupblit(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh) {
	if (dw <= 0 || dh <= 0) return;
	for (int iy = 0; iy < dh; iy++) {
		int src_y = sy + (int)((unsigned long)iy * (unsigned)sh / (unsigned)dh);
		for (int ix = 0; ix < dw; ix++) {
			int src_x = sx + (int)((unsigned long)ix * (unsigned)sw / (unsigned)dw);
			int bit = tsb_pxget(src_x, src_y);
			tsb_setpix(dx + ix, dy + iy, bit);
		}
	}
	tsb_dirty = 1;
}

static void tsb_using(const char *fmt, int len, int n, const int *items) {
	int idx = 0, i = 0;
	char field[40];
	char digits[40];
	while (i < len) {
		char c = fmt[i];
		if (c == '#' || c == ',') {
			int W = 0;
			while (i < len && (fmt[i] == '#' || fmt[i] == ',')) { field[W++] = fmt[i]; i++; }
			int v = (idx < n) ? items[idx++] : 0;
			int neg = (v < 0);
			unsigned u = neg ? (unsigned)(-(long)v) & 0xFFFFu : (unsigned)v & 0xFFFFu;
			int dc = 0;
			if (u == 0) digits[dc++] = '0';
			while (u) { digits[dc++] = (char)('0' + (int)(u % 10u)); u /= 10u; }
			int dp = 0;
			for (int y = W - 1; y >= 0; y--) {
				if (field[y] == ',') continue;
				field[y] = (dp < dc) ? digits[dp++] : ' ';
			}
			if (neg) {
				for (int y = 0; y < W; y++) if (field[y] == ' ') { field[y] = '-'; break; }
			}
			for (int y = 0; y < W; y++) b_putc((unsigned char)field[y]);
		} else {
			b_putc((unsigned char)c);
			i++;
		}
	}
}

static int tsb_sinmag(int a, int* sign) {
	a &= 0xFF;
	int idx = a & 0x3F;
	if (a & 0x40) idx = 0x3F - idx;
	*sign = (a >> 7) & 1;
	return tsb_sintab[idx];
}
static void tsb_circ_point(int cx, int cy, int rx, int ry, int ang, int* ox, int* oy) {
	int ss, cs;
	int dxm = (rx * tsb_sinmag(ang, &ss)) >> 8;
	int dym = (ry * tsb_sinmag(ang + 64, &cs)) >> 8;
	int dxb = ss ? ((-dxm) & 0xFF) : (dxm & 0xFF);
	int dyb = cs ? ((-dym) & 0xFF) : (dym & 0xFF);
	int dx = dxb;
	if (dx & 0x80) dx -= 256;
	int dy = dyb;
	if (dy & 0x80) dy -= 256;
	*ox = cx + dx;
	*oy = (cy + dyb) & 0xFF;
}
static void tsb_circle(int cx, int cy, int rx, int ry, int f) {
	int px, py;
	tsb_circ_point(cx, cy, rx, ry, 0, &px, &py);
	int fx = px, fy = py;
	for (int a = 4; a < 256; a += 4) {
		int nx, ny;
		tsb_circ_point(cx, cy, rx, ry, a, &nx, &ny);
		tsb_line(px, py, nx, ny, f);
		px = nx;
		py = ny;
	}
	tsb_line(px, py, fx, fy, f);
}
static void tsb_angl(int cx, int cy, int ang_deg, int rx, int ry, int f) {
	int ang = (int)((ang_deg * 256 + 180) / 360) & 0xFF;
	int nx, ny;
	tsb_circ_point(cx, cy, rx, ry, ang, &nx, &ny);
	tsb_line(cx, cy, nx, ny, f);
}
static void tsb_arc(int cx, int cy, int a1d, int a2d, int sd, int rx, int ry, int f) {
	int a1 = (int)((a1d * 256 + 180) / 360) & 0xFF;
	int a2 = (int)((a2d * 256 + 180) / 360) & 0xFF;
	int st = (int)((sd * 256 + 180) / 360) & 0xFF;
	if (st <= 0) st = 1;
	if (a2 < a1) a2 += 256;
	int nseg = (a2 - a1) / st;
	if (nseg <= 0) return;
	int px, py;
	tsb_circ_point(cx, cy, rx, ry, a1, &px, &py);
	for (int i = 1; i <= nseg; i++) {
		int nx, ny;
		tsb_circ_point(cx, cy, rx, ry, a1 + i * st, &nx, &ny);
		tsb_line(px, py, nx, ny, f);
		px = nx;
		py = ny;
	}
}

static int tsb_pxget(int x, int y) {
	return (tsb_bmget(tsb_pixbyte(x, y)) & (unsigned char)(0x80 >> (x & 7))) ? 1 : 0;
}

static void tsb_paint(int x, int y, int f) {
	if (x < 0 || x >= 320 || y < 0 || y >= 200) return;
	unsigned char bg = (unsigned char)tsb_pxget(x, y);
	if (bg == f) return;

	static int stx[512], sty[512];
	int sp = 0;
	stx[sp] = x;
	sty[sp] = y;
	sp++;
	while (sp > 0) {
		sp--;
		x = stx[sp];
		y = sty[sp];
		if (y < 0 || y >= 200) continue;
		int lx = x;
		while (lx >= 0 && tsb_pxget(lx, y) == bg) lx--;
		lx++;
		int rx = lx;
		while (rx < 320 && tsb_pxget(rx, y) == bg) {
			unsigned char m = (unsigned char)(0x80 >> (rx & 7));
			unsigned short a = tsb_pixbyte(rx, y);
			unsigned char bv = tsb_bmget(a);
			if (f == 0) bv &= (unsigned char)~m;
			else if (f == 1) bv |= m;
			else bv ^= m;
			b_poke(a, bv);
			rx++;
		}
		rx--;
		int yy;
		for (yy = y - 1; yy <= y + 1; yy += 2) {
			if (yy < 0 || yy >= 200) continue;
			int i = lx;
			while (i <= rx) {
				if (tsb_pxget(i, yy) == bg) {
					if (sp < 512) { stx[sp] = i; sty[sp] = yy; sp++; }
					while (i <= rx && tsb_pxget(i, yy) == bg) i++;
				} else {
					i++;
				}
			}
		}
	}
	tsb_inval();
}

static void tsb_char(int x, int y, int code, int mode, int zoom) {
	int gc = code & 0x7F;
	for (int r = 0; r < 8; r++) {
		unsigned char bits = tsb_chargen[gc * 8 + r];
		if (code & 0x80) bits ^= 0xFF;
		for (int c = 0; c < 8; c++) {
			int on = (bits >> (7 - c)) & 1;
			int px = on ? 1 : 0;
			if (mode == 0) px = on ? 0 : 0;
			if (mode == 2) px = 2;
			for (int zy = 0; zy < zoom; zy++)
			for (int zx = 0; zx < zoom; zx++) {
				int xx = x + c * zoom + zx, yy = y + r * zoom + zy;
				if (mode == 0) { if (on) tsb_setpix(xx, yy, 0); }
				else if (mode == 2) tsb_setpix(xx, yy, 2);
				else if (on) tsb_setpix(xx, yy, 1);
			}
		}
	}
	tsb_dirty = 1;
}
static void tsb_text(int x, int y, const char* s, int len, int mode, int zoom, int kern) {
	for (int i = 0; i < len; i++) {
		unsigned char ch = (unsigned char)s[i];

		int code;
		if (ch >= 0x20 && ch < 0x40) code = ch;
		else if (ch >= 0x40 && ch < 0x60) code = ch - 0x40;
		else if (ch >= 0x60 && ch < 0x80) code = ch - 0x60;
		else if (ch >= 0xC0) code = ch - 0x80;
		else code = ch & 0x3F;
		tsb_char(x, y, code, mode, zoom);
		x += kern;
	}
}

static void tsb_rot(int angle, int step) {
	angle &= 7;
	tsb_rotgr = step > 0 ? step : 1;
	for (int i = 0; i < 8; i++) tsb_drawtab[i] = tsb_rottab[angle * 8 + i];
}
static void tsb_draw(const char* s, int len, int x, int y, int f) {
	tsb_drx = x;
	tsb_dry = y;
	int i = 0;
	while (i < len) {
		int v = ((unsigned char)s[i] - 0x30);
		i++;
		if (v >= 8) continue;
		int pen = (v < 4) ? 0 : 1;
		int idx = v & 3;
		int dx = tsb_drawtab[idx], dy = tsb_drawtab[idx + 4];
		for (int k = 0; k < tsb_rotgr; k++) {
			tsb_drx += dx;
			tsb_dry += dy;
			if (pen) tsb_setpix(tsb_drx, tsb_dry, f);
		}
	}
	tsb_dirty = 1;
}
static void tsb_drawto(int x, int y, int f) {
	tsb_line(tsb_drx, tsb_dry, x, y, f);
	tsb_drx = x;
	tsb_dry = y;
}

static void tsb_multi_on(int on) {
	unsigned char v = b_peek(0xD016);
	b_poke(0xD016, on ? (unsigned char)(v | 0x10) : (unsigned char)(v & 0xEF));
	tsb_inval();
}
static void tsb_multi_cols(int c1, int c2, int c3) {
	unsigned char pair = (unsigned char)(((c1 & 15) << 4) | (c2 & 15));
	b_poke(0xD022, c1 & 15);
	b_poke(0xD023, c2 & 15);
	b_poke(0xD024, c3 & 15);
	(void)pair;
	b_poke(0xD016, (unsigned char)(b_peek(0xD016) | 0x10));
	tsb_inval();
}

static volatile unsigned char* tsb_scrbase(void) {
	return b_mem + ((unsigned)b_peek(0x0288) << 8);
}
static void tsb_rect_fill(volatile unsigned char* base, int row, int col, int w, int h, int val) {
	for (int r = 0; r < h; r++) {
		int rr = row + r;
		if (rr < 0 || rr >= 25) continue;
		for (int c = 0; c < w; c++) {
			int cc = col + c;
			if (cc < 0 || cc >= 40) continue;
			base[rr * 40 + cc] = (unsigned char)val;
		}
	}
	tsb_inval();
}
static void tsb_rect_inv(volatile unsigned char* base, int row, int col, int w, int h) {
	for (int r = 0; r < h; r++) {
		int rr = row + r;
		if (rr < 0 || rr >= 25) continue;
		for (int c = 0; c < w; c++) {
			int cc = col + c;
			if (cc < 0 || cc >= 40) continue;
			base[rr * 40 + cc] ^= 0x80;
		}
	}
	tsb_inval();
}
static void tsb_cls(void) {
	memset(b_mem + 0x0400, 0x20, 1000);
	b_poke(0xD3, 0);
	b_poke(0xD6, 0);
	tsb_inval();
}
static void tsb_fill(int row, int col, int w, int h, int ch, int color) {
	tsb_rect_fill(tsb_scrbase(), row, col, w, h, ch);
	if (color == 0) color = (int)(unsigned char)b_peek(0x0286);
	tsb_rect_fill(b_mem + 0xD800, row, col, w, h, color);
}
static void tsb_fchr(int row, int col, int w, int h, int ch) {
	tsb_rect_fill(tsb_scrbase(), row, col, w, h, ch);
}
static void tsb_fcol(int row, int col, int w, int h, int color) {
	tsb_rect_fill(b_mem + 0xD800, row, col, w, h, color);
}
static void tsb_inv(int row, int col, int w, int h) {
	tsb_rect_inv(tsb_scrbase(), row, col, w, h);
}
static void tsb_at(int x, int y) { b_poke(0xD3, (unsigned char)x); b_poke(0xD6, (unsigned char)y); b_col = x; b_row = y; tsb_dirty = 1; }
static void tsb_centre(const char* s, int len, int width) {
	int col = b_peek(0xD3);
	int pad = width - col - len;
	if (pad > 0) {
		int half = pad >> 1;
		for (int i = 0; i < half; i++) b_poke(0x0400 + (int)b_peek(0xD6) * 40 + (int)b_peek(0xD3) + i, 0x1D);
	}

	int row = b_peek(0xD6), c = b_peek(0xD3);
	if (pad > 0) c += (pad >> 1);
	for (int i = 0; i < len && c < 40; i++, c++) b_poke(0x0400 + row * 40 + c, (unsigned char)s[i]);
	b_row = row;
	b_col = c;
	b_poke(0xD6, (unsigned char)row);
	b_poke(0xD3, (unsigned char)c);
	tsb_inval();
}

static void tsb_scroll(int row, int col, int w, int h, int dir, int trow, int tcol) {
	volatile unsigned char* scr = tsb_scrbase();
	volatile unsigned char* colr = b_mem + 0xD800;

	if (row < 0) row = 0;
	if (col < 0) col = 0;
	if (w <= 0 || h <= 0) return;
	if (row + h > 25) h = 25 - row;
	if (col + w > 40) w = 40 - col;
		#define TSB_GET(rr,cc)  (scr[(rr)*40+(cc)])
		#define TSB_SET(rr,cc,v) do{ scr[(rr)*40+(cc)]=(unsigned char)(v); }while(0)
	if (dir == 8) {
		if (trow < 0) trow = 0;
		if (tcol < 0) tcol = 0;
		for (int r = 0; r < h; r++) {
			int sr = row + r, dr = trow + r;
			if (dr < 0 || dr >= 25) continue;
			for (int c = 0; c < w; c++) {
				int sc = col + c, dc = tcol + c;
				if (dc < 0 || dc >= 40) continue;
				scr[dr * 40 + dc] = TSB_GET(sr, sc);
				colr[dr * 40 + dc] = colr[sr * 40 + sc];
			}
		}
	} else if (dir == 0 || dir == 1) {
		for (int r = 0; r < h - 1; r++)
		for (int c = 0; c < w; c++) { TSB_SET(row+r,col+c,TSB_GET(row+r+1,col+c)); colr[(row+r)*40+col+c]=colr[(row+r+1)*40+col+c]; }
		for (int c = 0; c < w; c++) { TSB_SET(row+h-1,col+c, dir==1?TSB_GET(row,col+c):0x20); colr[(row+h-1)*40+col+c]=colr[row*40+col+c]; }
	} else if (dir == 2 || dir == 3) {
		for (int r = h-1; r > 0; r--)
		for (int c = 0; c < w; c++) { TSB_SET(row+r,col+c,TSB_GET(row+r-1,col+c)); colr[(row+r)*40+col+c]=colr[(row+r-1)*40+col+c]; }
		for (int c = 0; c < w; c++) { TSB_SET(row,col+c, dir==3?TSB_GET(row+h-1,col+c):0x20); colr[row*40+col+c]=colr[(row+h-1)*40+col+c]; }
	} else if (dir == 4 || dir == 5) {
		for (int r = 0; r < h; r++)
		for (int c = 0; c < w-1; c++) { TSB_SET(row+r,col+c,TSB_GET(row+r,col+c+1)); colr[(row+r)*40+col+c]=colr[(row+r)*40+col+c+1]; }
		for (int r = 0; r < h; r++) { TSB_SET(row+r,col+w-1, dir==5?TSB_GET(row+r,col):0x20); colr[(row+r)*40+col+w-1]=colr[(row+r)*40+col]; }
	} else if (dir == 6 || dir == 7) {
		for (int r = 0; r < h; r++)
		for (int c = w-1; c > 0; c--) { TSB_SET(row+r,col+c,TSB_GET(row+r,col+c-1)); colr[(row+r)*40+col+c]=colr[(row+r)*40+col+c-1]; }
		for (int r = 0; r < h; r++) { TSB_SET(row+r,col, dir==7?TSB_GET(row+r,col+w-1):0x20); colr[(row+r)*40+col]=colr[(row+r)*40+col+w-1]; }
	}
		#undef TSB_GET
		#undef TSB_SET
	tsb_inval();
}

static void tsb_mob(int n, int on) {
	unsigned char mask = (unsigned char)(1 << (n & 7));
	unsigned char v = b_peek(0xD015);
	b_poke(0xD015, on ? (unsigned char)(v | mask) : (unsigned char)(v & ~mask));
	tsb_inval();
}
static void tsb_mobcol(int mob, int col) {
	b_poke(0xD027 + (mob & 7), (unsigned char)col);
	tsb_inval();
}
static void tsb_mobset(int spr, int block, int color, int prio, int type, int exp) {
	b_poke(0xC3F8 + (spr & 7), (unsigned char)block);
	b_poke(0xD027 + (spr & 7), (unsigned char)color);
	unsigned char pb = b_peek(0xD01B);
	if (prio) pb |= (unsigned char)(1 << (spr & 7));
	else pb &= (unsigned char)~(1 << (spr & 7));
	b_poke(0xD01B, pb);
	if (type == 1) b_poke(0xD01C, (unsigned char)(b_peek(0xD01C) | (1 << (spr & 7))));
	if (exp & 1) b_poke(0xD01D, (unsigned char)(b_peek(0xD01D) | (1 << (spr & 7))));
	if (exp & 2) b_poke(0xD017, (unsigned char)(b_peek(0xD017) | (1 << (spr & 7))));
	tsb_inval();
}
static void tsb_cmob(int c1, int c2) { b_poke(0xD025, (unsigned char)c1); b_poke(0xD026, (unsigned char)c2); }
static void tsb_mmob(int spr, int x1, int y1, int x2, int y2, int speed) {

	int x = x1, y = y1;
	(void)x2;
	(void)y2;
	(void)speed;
	b_poke(0xD000 + (spr & 7) * 2, (unsigned char)(x & 0xFF));
	b_poke(0xD010, (unsigned char)((b_peek(0xD010) & ~(1 << (spr & 7))) | ((x >> 8) << (spr & 7))));
	b_poke(0xD001 + (spr & 7) * 2, (unsigned char)y);
	tsb_inval();
}
static void tsb_rlocmob(int spr, int x, int y, int speed) { tsb_mmob(spr, x, y, x, y, speed); }

static void tsb_pause(int n) {
	breal_t t0 = b_jiffy();
	while (b_jiffy() - t0 < (breal_t)n) { tsb_poll(); }
}
static void tsb_dpoke(int addr, int val) {
	b_poke((unsigned)addr, (unsigned char)(val & 0xFF));
	b_poke((unsigned)(addr + 1), (unsigned char)((val >> 8) & 0xFF));
}
static void tsb_sound(int voice, int freq) {
	unsigned base = (voice == 4) ? 0xD415 : (0xD400 + (unsigned)(voice - 1) * 7);
	b_poke(base, (unsigned char)(freq & 0xFF));
	b_poke(base + 1, (unsigned char)((freq >> 8) & 0xFF));
}
static void tsb_wave(int voice, int wf, int pulse) {
	unsigned base = (voice == 4) ? 0xD415 : (0xD400 + (unsigned)(voice - 1) * 7);
	if (pulse >= 0) {
		b_poke(base + 2, (unsigned char)(pulse & 0xFF));
		b_poke(base + 3, (unsigned char)((pulse >> 8) & 0x0F));
	}
	b_poke(base + 4, (unsigned char)wf);
}
static void tsb_envelope(int voice, int a, int d, int s, int r) {
	unsigned base = (0xD400 + (unsigned)(voice - 1) * 7);
	b_poke(base + 5, (unsigned char)(((a & 15) << 4) | (d & 15)));
	b_poke(base + 6, (unsigned char)(((s & 15) << 4) | (r & 15)));
}
static void tsb_vol(int v) { b_poke(0xD418, (unsigned char)v); }

static void tsb_cset(int mode) {
	if (mode == 2) { b_poke(0xD011, 0x3B); b_poke(0xD018, 0x0B); b_poke(0xDD00, 0x94); }
	else { b_poke(0xD018, (unsigned char)(mode == 1 ? 0x17 : 0x15)); }
	tsb_inval();
}
static void tsb_mod(int ink, int paper) {
	unsigned char mv = (unsigned char)(((ink & 15) << 4) | (paper & 15));
	memset(b_mem + 0xC000, mv, 1000);
	tsb_inval();
}
static void tsb_do_null(void) { while (b_getin() < 0) { } }
static void tsb_graphics(int mode) {
	if (mode == 2) { b_poke(0xD011, 0x3B); b_poke(0xD016, 0xD8); }
	else if (mode == 1) { b_poke(0xD011, 0x3B); b_poke(0xD016, 0xC8); }
	else { b_poke(0xD011, 0x1B); b_poke(0xD016, 0xC8); }
	tsb_mode = (mode == 1 || mode == 2) ? 1 : 0;
	tsb_dirty = 1;
}
static void tsb_dispoff(void) { b_poke(0xD011, (unsigned char)(b_peek(0xD011) & 0xEF)); tsb_dirty = 1; }
static void tsb_dispon(void) { b_poke(0xD011, (unsigned char)(b_peek(0xD011) | 0x10)); tsb_dirty = 1; }
static void tsb_hicol(void) { b_poke(0xD016, (unsigned char)(b_peek(0xD016) & 0xEF)); tsb_dirty = 1; }
static void tsb_lowcol(void) { (void)0; }
static void tsb_detect(void) { (void)b_peek(0xD01E); (void)b_peek(0xD01F); }
static void tsb_scrsv(void) {
	memcpy(tsb_scrsv_buf, b_mem + 0x0400, 1000);
	memcpy(tsb_scrsv_buf + 1000, b_mem + 0xD800, 1000);
}
static void tsb_scrld(void) {
	memcpy(b_mem + 0x0400, tsb_scrsv_buf, 1000);
	memcpy(b_mem + 0xD800, tsb_scrsv_buf + 1000, 1000);
	tsb_inval();
}
static void tsb_bckgnds(int c0, int c1, int c2, int c3) {
	b_poke(0xD021, (unsigned char)c0);
	if (c0 >= 0 && c0 < 128) {
		b_poke(0xD011, (unsigned char)((b_peek(0xD011) & 0xDF) | 0x40));
		b_poke(0xD016, (unsigned char)(b_peek(0xD016) & 0xEF));
	}
	if (c1 >= 0) b_poke(0xD022, (unsigned char)c1);
	if (c2 >= 0) b_poke(0xD023, (unsigned char)c2);
	if (c3 >= 0) b_poke(0xD024, (unsigned char)c3);
	tsb_inval();
}

static bstr_t tsb_fetch(const char* ctl, int clen, int max, int mode) {
	(void)mode;
	(void)ctl;
	(void)clen;
	int n = 0;
	while (n < max) {
		int k = b_getin();
		if (k == 13) break;
		if (k >= 32 && k < 127) { b_mem[0x0200 + n] = (unsigned char)k; n++; }
		tsb_poll();
	}
	return b_keep(b_dup((const char*)(b_mem + 0x0200), n));
}

static void tsb_cold(void) { b_flush(); exit(0); }

	#if defined(_WIN32) || defined(__CYGWIN__)
extern unsigned long __stdcall GetModuleFileNameA(void*, char*, unsigned long);
extern int __stdcall MessageBoxA(void*, const char*, const char*, unsigned int);
extern int __stdcall OpenClipboard(void*);
extern int __stdcall CloseClipboard(void);
extern int __stdcall EmptyClipboard(void);
extern void* __stdcall GetClipboardData(unsigned int);
extern void* __stdcall SetClipboardData(unsigned int, void*);
extern void* __stdcall GlobalAlloc(unsigned int, unsigned long);
extern void* __stdcall GlobalLock(void*);
extern int __stdcall GlobalUnlock(void*);
static unsigned int tsb_crc_tab[256];
static int tsb_crc_ok = 0;
static void tsb_crc_init(void) {
	for (unsigned int i = 0; i < 256; i++) {
		unsigned int c = i;
		for (int j = 0; j < 8; j++)
		c = (c & 1u) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
		tsb_crc_tab[i] = c;
	}
	tsb_crc_ok = 1;
}
static unsigned int tsb_crc32_file(const char* path) {
	if (!tsb_crc_ok) tsb_crc_init();
	FILE* f = fopen(path, "rb");
	if (!f) return 0;
	unsigned int crc = 0xFFFFFFFFu;
	unsigned char buf[4096];
	size_t n;
	while ((n = fread(buf, 1, sizeof buf, f)) > 0)
	for (size_t i = 0; i < n; i++)
	crc = tsb_crc_tab[(crc ^ buf[i]) & 0xFFu] ^ (crc >> 8);
	fclose(f);
	return crc ^ 0xFFFFFFFFu;
}
static int tsb_is_chargen_crc(unsigned int crc) {
	static const unsigned int c64cg[] = {
		0x1604F6C1u, 0x377A382Bu, 0xBEE9B3FDu, 0xC890C175u,
		0xEC4272EEu, 0xF649EC3Au
	};
	for (size_t i = 0; i < sizeof c64cg / sizeof c64cg[0]; i++)
	if (c64cg[i] == crc) return 1;
	return 0;
}

static int tsb_load_if_chargen(const char* path) {
	if (!tsb_is_chargen_crc(tsb_crc32_file(path))) return 0;
	FILE* f = fopen(path, "rb");
	if (!f) return 0;
	size_t r = fread(tsb_chargen, 1, 4096, f);
	fclose(f);
	if (r == 4096) { tsb_chargen_found = 1; return 1; }
	return 0;
}
static int tsb_scan_dir(const char* dir);
static int tsb_exedir(char* out, size_t n);
static void tsb_load_chargen(void) {
	if (tsb_chargen_loaded) return;
	tsb_chargen_loaded = 1;
	char ed[1024], d[1120];
	if (tsb_exedir(ed, sizeof ed)) {
		if (tsb_scan_dir(ed)) return;
		if (snprintf(d, sizeof d, "%s../../6502runner", ed) > 0 && tsb_scan_dir(d)) return;
		if (snprintf(d, sizeof d, "%s../6502runner", ed) > 0 && tsb_scan_dir(d)) return;
	}
	if (tsb_scan_dir("../../6502runner")) return;
	if (tsb_scan_dir("../6502runner")) return;
	if (tsb_scan_dir(".")) return;
}
static void tsb_init_tables(void) {
	for (int i = 0; i < 64; i++)
	tsb_sintab[i] = (unsigned char)round(sin((double)i * 3.14159265358979323846 / 128.0) * 255.0);
}
static void tsb_dump_bitmap(void) {
	const char* p = getenv("ABC_TSB_DUMP");
	const char* path = (p && *p && !(p[0] == '1' && p[1] == 0)) ? p : "tsb_dump.txt";
	FILE* f = fopen(path, "w");
	if (!f) return;
	for (int gy = 0; gy < 50; gy++) {
		char line[81];
		for (int gx = 0; gx < 80; gx++) {
			int cnt = 0;
			for (int py = 0; py < 4; py++)
			for (int px = 0; px < 4; px++) {
				int x = gx * 4 + px, y = gy * 4 + py;
				unsigned char byte = b_mem[0xE000 + (y >> 3) * 320 + (x >> 3) * 8 + (y & 7)];
				if (byte & (0x80 >> (x & 7))) cnt++;
			}
			{ int idx = (cnt == 0) ? 0 : 1 + (cnt - 1) * 9 / 16; if (idx > 9) idx = 9;
				line[gx] = " .:-=+*#%@"[idx];
			}
		}
		line[80] = 0;
		fprintf(f, "%s\n", line);
	}
	fclose(f);
}
static void tsb_dump_text(void) {
	if (!getenv("ABC_TSB_DUMP")) return;
	FILE* f = fopen("tsb_dump.txt", "w");
	if (!f) return;
	int d011 = b_peek(0xD011);
	int dd00 = b_peek(0xDD00), d018 = b_peek(0xD018);
	unsigned bankBase = (unsigned)((~(dd00 & 3)) & 3) * 0x4000u;
	unsigned matBase = bankBase + (unsigned)((d018 >> 4) & 0xF) * 0x0400u;
	if (d011 & 0x20) {
		unsigned bmpBase = bankBase + (unsigned)((d018 & 0x08) ? 0x2000u : 0x0000u);
		static const char dch[] = " .:-=+*#";
		for (int cy = 0; cy < 25; cy++) {
			for (int cx = 0; cx < 40; cx++) {
				int bits = 0;
				for (int yr = 0; yr < 8; yr++) {
					unsigned char b = b_mem[bmpBase + cy * 320 + cx * 8 + yr];
					for (int k = 0; k < 8; k++) if (b & (0x80 >> k)) bits++;
				}
				fputc(dch[bits >= 56 ? 7 : bits >= 40 ? 6 : bits >= 24 ? 5 :
				bits >= 12 ? 4 : bits >= 5 ? 3 : bits >= 2 ? 2 :
				bits >= 1 ? 1 : 0], f);
			}
			fputc('\n', f);
		}
		fprintf(f, "presents=%ld hires\n", tsb_present_count);
		fclose(f);
		return;
	}
	int cb = (d018 & 0x0E) >> 1;
	unsigned chBase = (cb == 2 || cb == 3) ? (unsigned)(cb - 2) * 0x800u : 0u;
	int sample = -1;
	for (int cy = 0; cy < 25; cy++) {
		for (int cx = 0; cx < 40; cx++) {
			unsigned char sc = b_mem[matBase + cy * 40 + cx];
			if (cy >= 5 && sc != 0x20 && sample < 0) sample = sc & 0x7F;
			fputc((sc == 0x20) ? ' ' : '#', f);
		}
		fputc('\n', f);
	}
	fprintf(f, "=== char dump ===\n");
	for (int cy = 0; cy < 25; cy++) {
		for (int cx = 0; cx < 40; cx++) {
			unsigned char sc = b_mem[matBase + cy * 40 + cx];
			char ch;
			if (sc == 0x20) ch = ' ';
			else if (sc >= 0x41 && sc <= 0x5A) ch = (char)sc;
			else if (sc >= 0xC1 && sc <= 0xDA) ch = (char)(sc - 0x80);
			else if (sc >= 0x30 && sc <= 0x39) ch = (char)sc;
			else if (sc == 0x1D) ch = '~';
			else if (sc == 0xA0) ch = '#';
			else if (sc >= 0x21 && sc <= 0x3F) ch = (char)sc;
			else if (sc & 0x80) ch = '?';
			else ch = '.';
			fputc(ch, f);
		}
		fputc('\n', f);
	}
	if (sample >= 0) {
		fprintf(f, "sample sc=%d cb=%d glyph:\n", sample, cb);
		for (int yr = 0; yr < 8; yr++) {
			unsigned char bits = tsb_chargen[chBase + sample * 8 + yr];
			for (int b = 0; b < 8; b++) fputc((bits & (0x80 >> b)) ? '#' : '.', f);
			fputc('\n', f);
		}
	}
	fprintf(f, "presents=%ld\n", tsb_present_count);
	fclose(f);
	(void)d011;
}

typedef unsigned int DWORD;
typedef int LONG;
typedef unsigned short WORD;
typedef void* HWND, *HDC, *HGDIOBJ, *HINSTANCE, *HMENU, *HANDLE;
typedef struct { LONG biSize; LONG biWidth; LONG biHeight; WORD biPlanes; WORD biBitCount; DWORD biCompression; DWORD biSizeImage; LONG biXPelsPerMeter; LONG biYPelsPerMeter; DWORD biClrUsed; DWORD biClrImportant; } TSB_BMIH;
typedef struct { TSB_BMIH bmiHeader; } TSB_BMI;

typedef struct { HWND hwnd; unsigned int message; unsigned int _pad; unsigned long long wParam; long long lParam; DWORD time; LONG pt_x; LONG pt_y; } TSB_MSG;
typedef struct { HDC hdc; int fErase; LONG rc_left, rc_top, rc_right, rc_bottom; int fRestore; int fIncUpdate; unsigned char rgbReserved[32]; } TSB_PS;
typedef struct { unsigned int style; void* lpfnWndProc; int cbClsExtra; int cbWndExtra; HINSTANCE hInstance; HGDIOBJ hIcon; HGDIOBJ hCursor; HGDIOBJ hbrBackground; const char* lpszMenuName; const char* lpszClassName; } TSB_WNDCLASS;

typedef struct { DWORD dwFileAttributes; DWORD ftCreationTime[2], ftLastAccessTime[2], ftLastWriteTime[2]; DWORD nFileSizeHigh, nFileSizeLow, dwReserved0, dwReserved1; char cFileName[260]; char cAlternateFileName[14]; } TSB_FIND_DATA;
		#define TSB_WM_PAINT 0x000F
		#define TSB_WM_CLOSE 0x0010
		#define TSB_WM_DESTROY 0x0002
		#define TSB_WM_QUIT 0x0012
		#define TSB_WM_CHAR 0x0102
		#define TSB_WM_KEYDOWN 0x0100
		#define TSB_WM_KEYUP 0x0101
		#define TSB_CS_VREDRAW 0x0001
		#define TSB_CS_HREDRAW 0x0002
		#define TSB_PEEK_REMOVE 0x0001
		#define TSB_SRCCOPY 0x00CC0020
		#define TSB_DIB_RGB_COLORS 0
		#define TSB_HALFTONE 4

typedef struct { LONG left, top, right, bottom; } TSB_RECT;
extern void __stdcall PostQuitMessage(int);
extern int __stdcall PeekMessageA(TSB_MSG*, HWND, unsigned int, unsigned int, unsigned int);
extern int __stdcall TranslateMessage(const TSB_MSG*);
extern long long __stdcall DispatchMessageA(const TSB_MSG*);
extern long long __stdcall DefWindowProcA(HWND, unsigned int, unsigned long long, long long);
extern unsigned short __stdcall RegisterClassA(const TSB_WNDCLASS*);
extern HGDIOBJ __stdcall LoadCursorA(HINSTANCE, const char*);
		#define TSB_IDC_ARROW ((const char*)32512)
extern HWND __stdcall CreateWindowExA(DWORD, const char*, const char*, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, void*);
extern int __stdcall ShowWindow(HWND, int);
extern int __stdcall UpdateWindow(HWND);
extern int __stdcall SetForegroundWindow(HWND);
extern unsigned long __stdcall GetLastError(void);
extern HDC __stdcall GetDC(HWND);
extern HDC __stdcall BeginPaint(HWND, TSB_PS*);
extern int __stdcall EndPaint(HWND, const TSB_PS*);
extern int __stdcall ReleaseDC(HWND, HDC);
extern HDC __stdcall CreateCompatibleDC(HDC);
extern int __stdcall DeleteDC(HDC);
extern int __stdcall DeleteObject(HGDIOBJ);
extern HGDIOBJ __stdcall SelectObject(HDC, HGDIOBJ);
extern int __stdcall BitBlt(HDC, int, int, int, int, HDC, int, int, DWORD);
extern int __stdcall StretchBlt(HDC, int, int, int, int, HDC, int, int, int, int, DWORD);
extern int __stdcall SetStretchBltMode(HDC, int);
extern int __stdcall GetClientRect(HWND, TSB_RECT*);
extern HANDLE __stdcall CreateDIBSection(HDC, const TSB_BMI*, unsigned int, void**, HGDIOBJ, DWORD);
extern int __stdcall InvalidateRect(HWND, const void*, int);
extern int __stdcall SetWindowTextA(HWND, const char*);
extern int __stdcall IsIconic(HWND);
extern void __stdcall Sleep(unsigned long);
extern void __stdcall ExitProcess(unsigned int);
extern HANDLE __stdcall CreateThread(void*, unsigned long, unsigned long(__stdcall*)(void*), void*, unsigned long, unsigned long*);
extern int __stdcall CloseHandle(HANDLE);
extern unsigned long __stdcall GetTickCount(void);
extern HANDLE __stdcall FindFirstFileA(const char*, TSB_FIND_DATA*);
extern int __stdcall FindNextFileA(HANDLE, TSB_FIND_DATA*);
extern int __stdcall FindClose(HANDLE);

static HWND tsb_hwnd;
static HDC tsb_mdc;
static HGDIOBJ tsb_dib;
static unsigned char* tsb_bits;
static unsigned int* tsb_pix;
static long tsb_lock;

static void tsb_lock_acq(void) { while (__atomic_exchange_n(&tsb_lock, 1, __ATOMIC_ACQUIRE)) Sleep(0); }
static void tsb_lock_rel(void) { __atomic_store_n(&tsb_lock, 0, __ATOMIC_RELEASE); }

		#define TSB_DBW 384
		#define TSB_DBH 264
		#define TSB_BORDER 32

static const unsigned int tsb_pal[16] = {
	0x000000,0xFFFFFF,0x880000,0xAAFFEE,0xCC44CC,0x00CC55,0x0000AA,0xEEEE77,
	0xDD8855,0x664400,0xFF7777,0x333333,0x777777,0xAAFF66,0x0088FF,0xBBBBBB
};
static unsigned int tsb_col(int idx) { return tsb_pal[idx & 15]; }

static int tsb_bord_valid = 0;
static unsigned int tsb_bord_cache = 0;
static int tsb_prev_d011 = -1, tsb_prev_d018 = -1, tsb_prev_dd00 = -1;
static int tsb_prev_d016 = -1;
static int tsb_prev_d021 = -1, tsb_prev_d020 = -1;

static void tsb_blit(void) {
	if (!tsb_pix) return;
	int d011 = b_peek(0xD011), d018 = b_peek(0xD018);
	int d016 = b_peek(0xD016);
	int dd00 = b_peek(0xDD00);
	int bank = (~(dd00 & 3)) & 3;
	unsigned bankBase = (unsigned)bank * 0x4000;
	unsigned matBase = bankBase + (unsigned)((d018 >> 4) & 0xF) * 0x0400;
	unsigned bmpBase = bankBase + (unsigned)((d018 & 0x08) ? 0x2000 : 0x0000);
	int bg = b_peek(0xD021) & 15;
	int bord = b_peek(0xD020) & 15;
	unsigned int bordc = tsb_col(bord), bgc = tsb_col(bg);
	int hires = (d011 & 0x20) != 0;
	int mcm = (d016 & 0x10) != 0;
	int enabled = (d011 & 0x10) != 0;

	unsigned int bordp = bordc | 0xFF000000u;

	if (d011 != tsb_prev_d011 || d018 != tsb_prev_d018 || dd00 != tsb_prev_dd00 ||
	d016 != tsb_prev_d016 || bg != tsb_prev_d021) tsb_full = 1;
	if (bord != tsb_prev_d020 || !enabled) tsb_bord_valid = 0;
	int full = tsb_full || !hires || !enabled;

	if (!tsb_bord_valid) {
		int npx = TSB_DBW * TSB_DBH;
		for (int i = 0; i < npx; i++) tsb_pix[i] = bordp;
		tsb_bord_valid = 1;
		tsb_bord_cache = bordp;
		full = 1;
	}
	if (!enabled) {

		tsb_full = 0;
		tsb_lo = 200;
		tsb_hi = -1;
		tsb_prev_d011 = d011;
		tsb_prev_d018 = d018;
		tsb_prev_dd00 = dd00;
		tsb_prev_d016 = d016;
		tsb_prev_d021 = bg;
		tsb_prev_d020 = bord;
		return;
	}
	unsigned int bgp = bgc | 0xFF000000u;

	if (hires) {

		int y0, y1;
		if (full) { y0 = 0; y1 = 200; }
		else if (tsb_lo > tsb_hi) { y0 = 0; y1 = 0; }
		else { y0 = tsb_lo & ~7; y1 = (tsb_hi | 7) + 1;
			if (y0 < 0) y0 = 0;
			if (y1 > 200) y1 = 200;
		}
		for (int y = y0; y < y1; y++) {
			unsigned int* row = tsb_pix + (y + TSB_BORDER) * TSB_DBW + TSB_BORDER;
			int celly = y >> 3, yr = y & 7;
			for (int cx = 0; cx < 40; cx++) {
				unsigned char byte = b_mem[bmpBase + celly * 320 + cx * 8 + yr];
				unsigned int* o = row + cx * 8;
				if (mcm) {
					unsigned char sc = b_mem[matBase + celly * 40 + cx];
					unsigned char co = b_mem[0xD800 + celly * 40 + cx];
					unsigned int c[4];
					c[0] = bgp;
					c[1] = tsb_col(sc >> 4)   | 0xFF000000u;
					c[2] = tsb_col(sc & 15)   | 0xFF000000u;
					c[3] = tsb_col(co & 15)   | 0xFF000000u;
					for (int p = 0; p < 4; p++) {
						unsigned int px = c[(byte >> (6 - p * 2)) & 3];
						o[p * 2] = px;
						o[p * 2 + 1] = px;
					}
				} else {
					unsigned char mv = b_mem[matBase + celly * 40 + cx];
					unsigned int fgp = tsb_col(mv >> 4) | 0xFF000000u;
					unsigned int cbgp = tsb_col(mv & 15) | 0xFF000000u;
					for (int b = 0; b < 8; b++) {
						o[b] = (byte & 0x80) ? fgp : cbgp;
						byte <<= 1;
					}
				}
			}
		}
	} else {
		if (!tsb_chargen_found && !tsb_no_rom_warned) {
			tsb_no_rom_warned = 1;
			tsb_need_rom_warn = 1;
		}
		int cb = (d018 & 0x0E) >> 1;
		unsigned chBase = (cb == 2 || cb == 3) ? (unsigned)(cb - 2) * 0x800u : 0u;
		for (int cy = 0; cy < 25; cy++) {
			for (int cx = 0; cx < 40; cx++) {
				int cell = cy * 40 + cx;
				unsigned char raw = b_mem[matBase + cell];
				unsigned char code = raw & 0x7F;
				unsigned char cc = b_mem[0xD800 + cell] & 15;
				if (mcm && !(cc & 8)) {
					unsigned int c[4];
					c[0] = bgp;
					c[1] = tsb_col(cc & 7) | 0xFF000000u;
					c[2] = tsb_col(b_peek(0xD022) & 15) | 0xFF000000u;
					c[3] = tsb_col(b_peek(0xD023) & 15) | 0xFF000000u;
					for (int yr = 0; yr < 8; yr++) {
						unsigned int* row = tsb_pix + ((cy * 8 + yr) + TSB_BORDER) * TSB_DBW + (cx * 8 + TSB_BORDER);
						unsigned char bits = tsb_chargen[chBase + code * 8 + yr];
						if (raw & 0x80) bits ^= 0xFF;
						for (int p = 0; p < 4; p++) {
							unsigned int px = c[(bits >> (6 - p * 2)) & 3];
							row[p * 2] = px;
							row[p * 2 + 1] = px;
						}
					}
				} else {
					unsigned int ccp = tsb_col(mcm ? (cc & 7) : (cc & 15)) | 0xFF000000u;
					for (int yr = 0; yr < 8; yr++) {
						unsigned int* row = tsb_pix + ((cy * 8 + yr) + TSB_BORDER) * TSB_DBW + (cx * 8 + TSB_BORDER);
						unsigned char bits = tsb_chargen[chBase + code * 8 + yr];
						if (raw & 0x80) bits ^= 0xFF;
						for (int b = 0; b < 8; b++) {
							row[b] = (bits & 0x80) ? ccp : bgp;
							bits <<= 1;
						}
					}
				}
			}
		}
	}

	tsb_full = 0;
	tsb_lo = 200;
	tsb_hi = -1;
	tsb_prev_d011 = d011;
	tsb_prev_d018 = d018;
	tsb_prev_dd00 = dd00;
	tsb_prev_d016 = d016;
	tsb_prev_d021 = bg;
	tsb_prev_d020 = bord;
}

static int tsb_text_mode(void) {
	int d011 = b_peek(0xD011);
	return (d011 & 0x10) && !(d011 & 0x20);
}
static char tsb_scr2ascii(unsigned char code) {
	code &= 0x7F;
	if (code < 0x20) return (char)(code + 0x40);
	if (code < 0x40) return (char)code;
	return '?';
}
static void tsb_copy_screen(void) {
	if (!tsb_text_mode() || !tsb_hwnd) return;
	int dd00 = b_peek(0xDD00), d018 = b_peek(0xD018);
	unsigned bankBase = (unsigned)((~(dd00 & 3)) & 3) * 0x4000u;
	unsigned matBase = bankBase + (unsigned)((d018 >> 4) & 0xF) * 0x0400u;
	char buf[25 * 41];
	int pos = 0;
	for (int cy = 0; cy < 25; cy++) {
		int rowend = pos;
		for (int cx = 0; cx < 40; cx++) {
			unsigned char raw = b_mem[matBase + cy * 40 + cx];
			char ch = tsb_scr2ascii(raw);
			if (ch != ' ') rowend = pos + 1;
			if (pos < (int)sizeof(buf) - 1) buf[pos++] = ch;
		}
		pos = rowend;
		if (pos + 2 < (int)sizeof(buf) - 1) { buf[pos++] = '\r'; buf[pos++] = '\n'; }
	}
	while (pos >= 2 && buf[pos-2] == '\r' && buf[pos-1] == '\n') pos -= 2;
	if (!pos) return;
	if (!OpenClipboard(tsb_hwnd)) return;
	EmptyClipboard();
	void* hg = GlobalAlloc(0x0042u , (unsigned long)pos + 1);
	if (hg) {
		char* p = (char*)GlobalLock(hg);
		if (p) { memcpy(p, buf, pos); p[pos] = 0; GlobalUnlock(hg); SetClipboardData(1u , hg); }
	}
	CloseClipboard();
}
static void tsb_paste_clipboard(void) {
	if (!tsb_text_mode() || !tsb_hwnd) return;
	if (!OpenClipboard(tsb_hwnd)) return;
	void* hg = GetClipboardData(1u );
	if (hg) {
		const char* p = (const char*)GlobalLock(hg);
		if (p) {
			for (; *p && b_getq_n < (int)sizeof(b_getq); p++) {
				unsigned char c = (unsigned char)*p;
				if (c == '\r') { b_getq[b_getq_n++] = 0x0d; if (p[1] == '\n') p++; }
				else if (c == '\n') { b_getq[b_getq_n++] = 0x0d; }
				else if (c >= 'a' && c <= 'z') b_getq[b_getq_n++] = (unsigned char)(c - 32);
				else if (c >= 0x20 && c < 0x7f) b_getq[b_getq_n++] = c;
			}
			GlobalUnlock(hg);
		}
	}
	CloseClipboard();
}

static long long __stdcall tsb_wndproc(HWND h, unsigned int msg, unsigned long long w, long long l) {
	if (msg == TSB_WM_PAINT) {
		TSB_PS ps;
		HDC dc = BeginPaint(h, &ps);
		TSB_RECT rc;
		GetClientRect(h, &rc);
		SetStretchBltMode(dc, TSB_HALFTONE);

		tsb_lock_acq();
		StretchBlt(dc, 0, 0, rc.right, rc.bottom, tsb_mdc, 0, 0, TSB_DBW, TSB_DBH, TSB_SRCCOPY);
		tsb_lock_rel();
		EndPaint(h, &ps);
		return 0;
	}
	if (msg == TSB_WM_CLOSE) { ExitProcess(0); }
	if (msg == TSB_WM_DESTROY) { PostQuitMessage(0); return 0; }
	if (msg == TSB_WM_KEYUP) { b_held_until_us = 0; b_held_fresh = 0; return 0; }
	if (msg == TSB_WM_KEYDOWN) {
		int c = -1;
		if (w == 0x25) c = 0x9d;
		else if (w == 0x27) c = 0x1d;
		else if (w == 0x26) c = 0x91;
		else if (w == 0x28) c = 0x11;
		if (c >= 0) {
			int n = b_mem[198];
			if (n < 10) {
				if (n > 0) memmove(b_mem + 632, b_mem + 631, (size_t)n);
				b_mem[631] = (unsigned char)c;
				b_mem[198] = (unsigned char)(n + 1);
			}
			int sc = b_key_to_scan(c);
			if (sc != 64) { b_held_scan = (unsigned char)sc; b_held_fresh = 1;
				b_held_until_us = b_now_us() + 300000ULL;
			}
		}
		return 0;
	}
	if (msg == TSB_WM_CHAR) {

		int c = (int)(w & 0xFF);
		if (c == 0x03) { tsb_copy_screen();    return 0; }
		if (c == 0x16) { tsb_paste_clipboard(); return 0; }
		if (c >= 'a' && c <= 'z') c -= 32;
		else if (c == 0x0A) c = 0x0D;
		else if (c == 0x08) c = 0x14;
		int n = b_mem[198];
		if (n < 10) {
			if (n > 0) memmove(b_mem + 632, b_mem + 631, (size_t)n);
			b_mem[631] = (unsigned char)c;
			b_mem[198] = (unsigned char)(n + 1);
		}
		{	int sc = b_key_to_scan(c);
			if (sc != 64) { b_held_scan = (unsigned char)sc; b_held_fresh = 1;
				b_held_until_us = b_now_us() + 300000ULL;
			} }
		return 0;
	}
	return DefWindowProcA(h, msg, w, l);
}

static void tsb_present_frame(void) {
	if (!tsb_hwnd || !tsb_bits) return;
	if (IsIconic(tsb_hwnd)) return;
	if (!tsb_dirty && !b_any_poke) return;
	tsb_dirty = 0;
	b_any_poke = 0;
	tsb_full = 1;
	tsb_lock_acq();
	unsigned long long _bt = 0;
	{ static int _on = -1; if (_on < 0) _on = getenv("ABC_FPS") != 0; if (_on) _bt = b_now_us(); }
	tsb_blit();
	if (_bt) {
		unsigned long long dt = b_now_us() - _bt;
		static unsigned long long bsum = 0, bcnt = 0, blast = 0;
		bsum += dt;
		bcnt++;
		unsigned long long now = b_now_us();
		if (!blast) blast = now;
		else if (now - blast >= 1000000ULL) {
			const char* p = getenv("ABC_FPS");
			FILE* f = (p && !(p[0]=='1'&&p[1]==0)) ? fopen(p,"a") : 0;
			fprintf(f?f:stderr, "blit_avg_us=%llu blit_n=%llu\n", bsum/bcnt, bcnt);
			if (f) { fflush(f); fclose(f); }
			bsum = 0;
			bcnt = 0;
			blast = now;
		}
	}
	{ static int _pon = -1; if (_pon < 0) _pon = getenv("ABC_PIX") != 0;
		if (_pon && tsb_pix) {
			unsigned int seen[64];
			int ns = 0;
			unsigned long long ck = 1469598103934665603ULL;
			for (int y = 0; y < 200 && ns < 64; y++) {
				unsigned int* row = tsb_pix + (y + TSB_BORDER) * TSB_DBW + TSB_BORDER;
				for (int x = 0; x < 320; x++) {
					unsigned int c = row[x];
					int k;
					ck ^= (unsigned long long)c;
					ck *= 1099511628211ULL;
					ck ^= (unsigned long long)(x*401+y);
					for (k = 0; k < ns; k++) if (seen[k] == c) break;
					if (k == ns) { if (ns < 64) seen[ns++] = c; }
				}
			}
			const char* p = getenv("ABC_PIX");
			FILE* f = (p && !(p[0]=='1'&&p[1]==0)) ? fopen(p,"a") : 0;
			fprintf(f?f:stderr, "pix_colors=%d ck=%016llx border=%08x bg=%08x\n", ns, ck,
			tsb_bord_cache, tsb_col(b_peek(0xD021)&15)|0xFF000000u);
			for (int k = 0; k < ns; k++) fprintf(f?f:stderr,"  %08x\n", seen[k]);
			if (f) { fflush(f); fclose(f); }
		}
	}
	tsb_lock_rel();
	tsb_present_age = 0;
	tsb_present_count++;
	{	static unsigned long long fps_last_us = 0; static unsigned long fps_frames = 0;
		fps_frames++;
		unsigned long long now = b_now_us();
		if (!fps_last_us) fps_last_us = now;
		else if (now - fps_last_us >= 1000000ULL) {
			unsigned long fps = (unsigned long)(fps_frames * 1000000ULL / (now - fps_last_us));
			char ttl[160];
			snprintf(ttl, sizeof ttl, "%s  FPS=%lu", tsb_progname, fps);
			SetWindowTextA(tsb_hwnd, ttl);
			fps_last_us = now;
			fps_frames = 0;
		}
	}
	tsb_dump_text();
	InvalidateRect(tsb_hwnd, 0, 0);
}
static void tsb_yield_present(void) { tsb_ever_yielded = 1; tsb_present_frame(); }

static unsigned long __stdcall tsb_thread(void* arg) {
	(void)arg;

	tsb_hwnd = CreateWindowExA(0, "tsbwin", "TSB", 0x00CF0000L, 0, 0, 960, 660, 0, 0, 0, 0);
	ShowWindow(tsb_hwnd, 5);
	SetForegroundWindow(tsb_hwnd);
	UpdateWindow(tsb_hwnd);
	HDC dc = GetDC(tsb_hwnd);
	tsb_mdc = CreateCompatibleDC(dc);
	TSB_BMI bmi;
	memset(&bmi, 0, sizeof bmi);
	bmi.bmiHeader.biSize = sizeof(TSB_BMIH);
	bmi.bmiHeader.biWidth = TSB_DBW;
	bmi.bmiHeader.biHeight = -TSB_DBH;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	void* p = 0;
	tsb_dib = CreateDIBSection(dc, &bmi, TSB_DIB_RGB_COLORS, &p, 0, 0);
	tsb_bits = (unsigned char*)p;
	tsb_pix = (unsigned int*)p;
	SelectObject(tsb_mdc, tsb_dib);
	ReleaseDC(tsb_hwnd, dc);
	for (;;) {
		TSB_MSG m;
		while (PeekMessageA(&m, 0, 0, 0, TSB_PEEK_REMOVE)) {
			TranslateMessage(&m);
			DispatchMessageA(&m);
		}
		if (tsb_hwnd && tsb_bits && (tsb_dirty || b_any_poke) && ++tsb_fb >= 1) {
			tsb_fb = 0;
			tsb_present_frame();
		}
		if (tsb_need_rom_warn) {
			tsb_need_rom_warn = 0;
			MessageBoxA(tsb_hwnd,
			"C64 character ROM not found.\n\nText-mode programs will render blank or garbled glyphs.\nPlace a 4096-byte C64 chargen ROM (e.g. 'chargen') in the .exe directory or a sibling '6502runner' folder.",
			"abc host: no char ROM",
			0x00002030u );
		}
		Sleep(16);
	}
	return 0;
}

static int tsb_scan_dir(const char* dir) {
	char pat[1100], path[1120];
	if (snprintf(pat, sizeof pat, "%s/*", dir) <= 0) return 0;
	TSB_FIND_DATA fd;
	HANDLE h = FindFirstFileA(pat, &fd);
	if (!h || h == (HANDLE)(~(unsigned long)0)) return 0;
	int found = 0;
	do {
		if (fd.dwFileAttributes & 0x10) continue;
		unsigned int sz = fd.nFileSizeLow;
		if (sz == 0 || sz > 65536) continue;
		if (snprintf(path, sizeof path, "%s/%s", dir, fd.cFileName) <= 0) continue;
		if (tsb_load_if_chargen(path)) { found = 1; break; }
	} while (FindNextFileA(h, &fd));
	FindClose(h);
	return found;
}
static int tsb_exedir(char* out, size_t n) {
	unsigned long r = GetModuleFileNameA(0, out, (unsigned long)n);
	if (r == 0 || r >= n) return 0;
	char* slash = 0;
	for (char* p = out; *p; p++) if (*p == '/' || *p == '\\') slash = p;
	if (slash) slash[1] = 0;
	else out[0] = 0;
	return 1;
}

static void tsb_init(void) {
	if (tsb_inited) return;
	tsb_inited = 1;
	tsb_init_tables();
	tsb_load_chargen();
	b_poke(0x8B11, 65);
	b_poke(0x8B12, 68);
	{
		char exe[1024];
		unsigned long r = GetModuleFileNameA(0, exe, (unsigned long)sizeof exe);
		if (r && r < sizeof exe) {
			char* base = exe;
			char* s;
			for (s = exe; *s; s++) if (*s == '/' || *s == '\\') base = s + 1;
			size_t n = strlen(base);
			if (n > 4 && (base[n-4]=='.' && (base[n-3]|0x20)=='e' && (base[n-2]|0x20)=='x' && (base[n-1]|0x20)=='e')) n -= 4;
			if (n > sizeof(tsb_progname) - 5) n = sizeof(tsb_progname) - 5;
			memcpy(tsb_progname, base, n);
			memcpy(tsb_progname + n, ".bas", 5);
		}
	}

	tsb_rot(0, 1);

	tsb_reset();
	b_host_window = 1;
	tsb_loop_on_end = getenv("ABC_LOOP") ? 1 : 0;
	b_present_hook = tsb_yield_present;
			#if !defined(ABC_NO_AUTORUN) && !defined(ABC_AUTORUN)
				#define ABC_AUTORUN 1
			#endif
			#ifdef ABC_AUTORUN
	{
		const char* k = getenv("ABC_KEYS");
		if (k) {
			for (; *k && b_getq_n < (int)sizeof(b_getq); k++) {
				if (k[0] == '\\' && (k[1] == 'r' || k[1] == 'n')) { b_getq[b_getq_n++] = 0x0d; k++; }
				else b_getq[b_getq_n++] = (unsigned char)*k;
			}
		} else {
			static const unsigned char dft[11] = { 0x0d,0x0d,0x0d,0x0d,0x0d,0x0d,0x0d,0x0d,0x0d,0x0d,0x5A };
			int i;
			for (i = 0; i < 11 && b_getq_n < (int)sizeof(b_getq); i++) b_getq[b_getq_n++] = dft[i];
		}
	}
			#endif
	if (getenv("ABC_TSB_DUMP")) atexit(tsb_dump_bitmap);
	TSB_WNDCLASS wc;
	memset(&wc, 0, sizeof wc);
	wc.style = TSB_CS_VREDRAW | TSB_CS_HREDRAW;
	wc.lpfnWndProc = (void*)tsb_wndproc;
	wc.hCursor = LoadCursorA(0, TSB_IDC_ARROW);
	wc.lpszClassName = "tsbwin";
	RegisterClassA(&wc);

	HANDLE th = CreateThread(0, 0, tsb_thread, 0, 0, 0);
	if (th) CloseHandle(th);
}

static void tsb_hold(void) {
	if (getenv("ABC_TSB_DUMP")) { tsb_blit(); tsb_dump_text(); tsb_done = 1; return; }
	tsb_present_frame();
	if (tsb_loop_on_end) { Sleep(200); return; }
	for (;;) Sleep(16);
}
static void tsb_poll(void) {   }
	#else
static void tsb_load_chargen(void) { }
static void tsb_init_tables(void) {
	static const unsigned char tab[64] = {
		0x00,0x06,0x0D,0x13,0x19,0x1F,0x25,0x2C,0x32,0x38,0x3E,0x44,0x4A,0x50,0x56,0x5C,
		0x62,0x67,0x6D,0x73,0x78,0x7E,0x83,0x88,0x8E,0x93,0x98,0x9D,0xA2,0xA7,0xAB,0xB0,
		0xB4,0xB9,0xBD,0xC1,0xC5,0xC9,0xCD,0xD0,0xD4,0xD7,0xDB,0xDE,0xE1,0xE4,0xE7,0xE9,
		0xEC,0xEE,0xF0,0xF2,0xF4,0xF6,0xF7,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFE,0xFF,0xFF
	};
	for (int i = 0; i < 64; i++) tsb_sintab[i] = tab[i];
}

static int tsb_scan_dir(const char* dir) { (void)dir; return 0; }
static int tsb_exedir(char* out, size_t n) { (void)out; (void)n; return 0; }
static void tsb_init(void) { if (!tsb_inited) { tsb_inited = 1; tsb_init_tables(); tsb_load_chargen(); tsb_rot(0, 1); b_poke(0x8B11, 65); b_poke(0x8B12, 68);
				#ifdef __TSB__
		tsb_reset();
				#endif
	} }
static void tsb_hold(void) { tsb_done = 1; }
static void tsb_poll(void) { }
static void tsb_blit(void) { }
	#endif

#endif
