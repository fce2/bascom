#ifndef _RUNTIME_H_

	#define _RUNTIME_H_

	#if defined(__C64__) || defined(__mos__) || defined(__mos6502__) || defined(__CC65__)
		#define TARGET_C64 1
		#define DOUBLE float
		#include <math.h>
	#elif defined(__AVR__)
		#define TARGET_AVR 1
		#ifndef B_TINY
			#define B_TINY  0
			#define B_SMALL 1
			#define B_FULL  2
		#endif
		#ifdef __AVRCC__
			#ifndef B_TIER
				#if defined(__AVRCC_RAM_SIZE__) && __AVRCC_RAM_SIZE__ < 256
					#define B_TIER B_TINY
				#elif defined(__AVRCC_RAM_SIZE__) && __AVRCC_RAM_SIZE__ <= 2048
					#define B_TIER B_SMALL
				#else
					#define B_TIER B_FULL
				#endif
			#endif
		#else
			#include <avr/io.h>
			#ifndef B_TIER
				#define B_TIER B_FULL
			#endif
		#endif
		#if B_TIER <= B_TINY
			#define DOUBLE long
		#elif defined(__AVRCC__) && B_TIER >= B_FULL
			#define DOUBLE double
		#else
			#define DOUBLE float
		#endif
		#if B_TIER >= B_FULL
			#include <math.h>
		#endif
		#if !defined(__AVRCC__) && B_TIER >= B_SMALL && defined(F_CPU) && defined(BAUD)
			#include <util/setbaud.h>
			#define B_UART 1
		#endif
	#else
		#define TARGET_HOST 1
		#ifndef DOUBLE
			#ifdef TARGET_BENCH
				#define DOUBLE double
			#else
				#define DOUBLE float
			#endif
		#endif
		#include <math.h>
	#endif
	#if (defined(TARGET_AVR) && B_TIER < B_FULL)
		#define B_NO_FLOATMATH 1
	#endif

	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <limits.h>

typedef long  bint_t;
typedef DOUBLE breal_t;

	#if INT_MAX < 2147483647L
		#define B_INT(t)   (bint_t)(t)
	#else
		#define B_INT(t)   (t)
	#endif
typedef struct bstr_tag
{
	const char* p;
	int len;
} bstr_t;

typedef struct
{
	int kind;
	bint_t i;
	breal_t f;
	const char* s;
	int slen;
} b_datum;
extern const b_datum b_data[];

extern int b_dataPtr;
extern int b_data_len;

	#define b_scr    (b_mem + b_scr_base())
	#define b_colram (b_mem + 0xD800)

static int b_row = 0, b_col = 0;
static int b_ptop = 0;
	#if defined(TARGET_AVR)

		#ifndef B_GOSUB_DEPTH
			#if B_TIER <= B_TINY
				#define B_GOSUB_DEPTH 8
			#else
				#define B_GOSUB_DEPTH 16
			#endif
		#endif
static int b_gosubStack[B_GOSUB_DEPTH];
	#else
static int b_gosubStack[256];
	#endif
static int b_gosubSP = 0;
static unsigned long b_rndSeed = 0x12345678u;

static void b_scroll(void);
static int  b_getin(void);

	#ifdef TARGET_C64
static volatile unsigned char* b_mem = (unsigned char*)0x0000;
static unsigned char b_pet2scr(unsigned char c)
{
	if (c >= 0x41 && c <= 0x5A) return (unsigned char)(c - 0x40);
	if (c >= 0xC1 && c <= 0xDA) return (unsigned char)(c - 0xC0);
	return c;
}

static unsigned b_scr_base(void)
{
	unsigned pg = (unsigned)b_mem[0x0288];
	if (pg) return pg << 8;
	unsigned bank = (unsigned)(3 - (b_mem[56576] & 3));
	unsigned vm   = (unsigned)((b_mem[53272] >> 4) & 15);
	return bank * 0x4000u + vm * 0x400u;
}

static breal_t b_jiffy(void)
{
	return (breal_t)((unsigned long)b_mem[0xA2] | ((unsigned long)b_mem[0xA1] << 8) | ((unsigned long)b_mem[0xA0] << 16));
}
		#ifndef B_POOL_SIZE
			#define B_POOL_SIZE 1024
		#endif
static char b_pool[B_POOL_SIZE];
	#elif defined(TARGET_AVR)
static volatile unsigned char* b_mem = (unsigned char*)0x0000;
		#define b_pet2scr(c) c
		#if B_TIER >= B_SMALL

			#ifndef B_POOL_SIZE
				#define B_POOL_SIZE 64
			#endif
static char b_pool[B_POOL_SIZE];
		#else
static char b_pool[1];
		#endif
static unsigned b_scr_base(void) { return 0; }
	#else
		#include <math.h>
		#include <sys/time.h>
		#include <time.h>
static unsigned char b_mem[65536];

static unsigned b_scr_base(void)
{
	unsigned pg = (unsigned)b_mem[0x0288];
	if (pg) return pg << 8;
	unsigned bank = (unsigned)(3 - (b_mem[56576] & 3));
	unsigned vm   = (unsigned)((b_mem[53272] >> 4) & 15);
	return bank * 0x4000u + vm * 0x400u;
}
static unsigned b_bmp_base(void)
{
	unsigned bank = (unsigned)(3 - (b_mem[56576] & 3));
	return bank * 0x4000u + ((b_mem[53272] & 0x08) ? 0x2000u : 0u);
}

static unsigned char b_pet2scr(unsigned char c)
{
	if (c < 0x20)  return c;
	if (c <= 0x3F) return c;
	if (c <= 0x5F) return (unsigned char)(c - 0x40);
	if (c <= 0x7F) return (unsigned char)(c - 0x20);
	if (c <= 0x9F) return (unsigned char)(c - 0x80);
	if (c <= 0xBF) return (unsigned char)(c - 0x40);
	if (c <= 0xDF) return (unsigned char)(c - 0x80);
	return (unsigned char)(c - 0x80);
}
static int b_host_window = 0;
static void (*b_present_hook)(void);

static int b_any_poke = 0;
static char b_pool[65536];
		#define B1541_SLOTS 16
		#define B1541_STORE 16
		#define B1541_CAP   16384
static struct { int used; int laf; int dev; int mode; int sidx; int pos; } b_slots[B1541_SLOTS];
static struct { int used; char name[64]; int len; unsigned char buf[B1541_CAP]; } b_store[B1541_STORE];
static int b_out_laf = -1, b_in_laf = -1;
static int b_slot(int laf) { int i; for (i = 0; i < B1541_SLOTS; i++) if (b_slots[i].used && b_slots[i].laf == laf) return i; return -1; }
static bstr_t b_dup(const char* s, int len);
static breal_t b_atof(const char* s);
		#ifdef TARGET_BENCH
static unsigned long long b_screen_writes = 0;
static FILE* b_cntfile = NULL;
static void b_cnt_flush(void)
{
	if (!b_cntfile) return;
	fseek(b_cntfile, 0, SEEK_SET);
	fprintf(b_cntfile, "%llu\n", (unsigned long long)b_screen_writes);
	fflush(b_cntfile);
}
static void b_cnt_bump(void)
{
	b_screen_writes++;
	if (b_cntfile && (b_screen_writes & 0x3FF) == 0) b_cnt_flush();
}
		#else
			#define b_cnt_bump()  ((void)0)
			#define b_cnt_flush() ((void)0)
		#endif

		#if defined(_WIN32) || defined(__CYGWIN__)

extern unsigned long __stdcall GetTickCount(void);
static long b_wall_ms(void) { return (long)GetTickCount(); }
		#else

static long b_wall_ms(void) { return (long)(clock() * 1000.0 / CLOCKS_PER_SEC); }
		#endif
		#if defined(TARGET_HOST)

static unsigned long b_jiffy_origin_ms = 0;
static breal_t b_jiffy(void) {
	return (breal_t)((unsigned long)(b_wall_ms() - (long)b_jiffy_origin_ms) * 60UL / 1000UL);
}
		#endif
	#endif

	#ifdef TARGET_AVR
static void b_init_avr_uart(void)
{
			#ifdef B_UART
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;
	UCSR0B = (1 << TXEN0) | (1 << RXEN0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
			#endif
}
	#endif
static void b_init(void)
{
		#ifdef TARGET_AVR

	b_mem = (unsigned char*)0x0000;
	b_row = b_col = 0;
	b_ptop = 0;
	b_gosubSP = 0;
	b_dataPtr = 0;
	b_rndSeed = 0x12345678u;
	b_init_avr_uart();
		#else
	int i;

	b_mem[56576] = 0x03;
	b_mem[53272] = 0x15;
	b_mem[646] = 14;
	for (i = 0; i < 25 * 40; i++) { b_scr[i] = (unsigned char)' '; b_colram[i] = 14; }
	b_row = b_col = 0;
	b_ptop = 0;
	b_gosubSP = 0;
	b_mem[53280] = 14;
	b_mem[53281] = 6;
			#ifdef TARGET_C64
	b_mem[198] = 0;
			#else
	b_any_poke = 0;
	b_jiffy_origin_ms = (unsigned long)b_wall_ms();
				#ifdef TARGET_BENCH
	{ const char* cf = getenv("BENCH_CNTFILE"); if (cf && cf[0]) b_cntfile = fopen(cf, "w"); }
				#endif
			#endif
		#endif
}

static inline void b_uart_tx(unsigned char c)
{
		#if defined(TARGET_AVR)
			#if B_TIER >= B_SMALL
				#ifdef B_UART
	while (!(UCSR0A & (1 << UDRE0))) ;
				#endif
				#ifdef UDR0
	UDR0 = c;
				#else
	*(volatile unsigned char*)0xC6 = c;
				#endif
			#else
	(void)c;
			#endif
		#endif
}
	#if defined(TARGET_HOST)
static int b_need_cr = 1;
	#endif
static void b_newline(void)
{
	b_col = 0;
	b_row++;
	if (b_row >= 25) { b_scroll(); b_row = 24; }
		#if defined(TARGET_HOST)
	b_need_cr = 1;
		#endif
	b_mem[0xD6] = (unsigned char)b_row;
	b_mem[0xD3] = (unsigned char)b_col;
}
static void b_putc(unsigned char c)
{
		#if defined(TARGET_AVR)
			#if B_TIER >= B_SMALL
	if (c == 147) { b_uart_tx(0x1b); b_uart_tx('['); b_uart_tx('2'); b_uart_tx('J'); return; }
	if (c == 0x0d) { b_uart_tx('\r'); b_uart_tx('\n'); return; }
	b_uart_tx(c);
			#else
	(void)c;
			#endif
		#else
	int cell;
			#if defined(TARGET_HOST)
	if (b_out_laf >= 0)
	{
		int s = b_out_laf, si;
		if (b_slots[s].dev != 3 && b_slots[s].sidx >= 0)
		{
			si = b_slots[s].sidx;
			if (b_slots[s].mode == 0) { b_mem[144] |= 0x02; return; }
			if (b_slots[s].pos < B1541_CAP) {
				b_store[si].buf[b_slots[s].pos++] = c;
				if (b_slots[s].pos > b_store[si].len) b_store[si].len = b_slots[s].pos;
			}
			else b_mem[144] |= 0x08;
			return;
		}
	}
			#endif
			#if defined(__X16__)
	{
		unsigned char a = c;
		if (a == 0x0d || a == 0x0a) { fputc('\n', stdout); }
		else if (a >= 0x20 && a < 0x40) { fputc((int)a, stdout); }
		else if (a >= 0x41 && a <= 0x5A) { fputc((int)a, stdout); }
		else if (a >= 0x61 && a <= 0x7A) { fputc((int)a, stdout); }
		else if (a >= 0xC1 && a <= 0xDA) { fputc((int)(a - 0x80), stdout); }
	}
			#endif
	if (c == 147)
	{
		int i;
		b_row = b_col = 0;
		b_mem[0xD6] = 0;
		b_mem[0xD3] = 0;
		for (i = 0; i < 25 * 40; i++)
		{
			b_scr[i] = (unsigned char)' ';
					#if defined(TARGET_HOST)
			b_colram[i] = b_mem[646];
			b_cnt_bump();
					#endif
		}
		return;
	}
	if (c == 19)
	{
		b_row = b_col = 0;
		b_mem[0xD6] = 0;
		b_mem[0xD3] = 0;
		return;
	}
	if (c == 0x0d)
	{
		b_newline();
		return;
	}
	if (b_col >= 40)
	b_newline();
	cell = b_row * 40 + b_col;
	b_scr[cell] = b_pet2scr(c);
			#if defined(TARGET_HOST)
	b_any_poke = 1;
	b_cnt_bump();
			#endif
	b_col++;
	b_mem[0xD6] = (unsigned char)b_row;
	b_mem[0xD3] = (unsigned char)b_col;
		#endif
}
	#if defined(__X16__)
static int x16_poke_intercept(unsigned short addr, unsigned char v);
static int x16_peek_intercept(unsigned short addr, unsigned char *out);
	#endif
static inline void b_poke(unsigned short addr, unsigned char v)
{
		#if defined(__X16__)
	if (x16_poke_intercept(addr, v)) return;
		#endif
		#if defined(TARGET_C64)
	if (addr >= 0xE000 && addr < 0xFF40) {
		unsigned char m = b_mem[1];
		if (m & 0x01) { b_mem[1] = (unsigned char)(m & 0xFE); b_mem[addr] = v; b_mem[1] = m; return; }
	}
		#endif
	b_mem[addr] = v;
		#if defined(TARGET_HOST)
	{ unsigned sb = b_scr_base();
		int scr = (addr >= sb && addr < sb + 25 * 40);
		int col = (addr >= 55296 && addr < 55296 + 25 * 40);
		int vic = (addr == 53280 || addr == 53281);
		if (scr || col || vic) b_any_poke = 1;
		if (scr) b_cnt_bump();
		if (b_mem[0xD011] & 0x20) { unsigned bb = b_bmp_base();
			if (addr >= bb && addr < bb + 8000) b_any_poke = 1;
		} }
		#endif
}

	#ifdef SYS6502
		#if defined(TARGET_HOST)
			#include "cpu6502.h"
			#include "cpu6502.c"
			#undef A
			#undef X
			#undef Y
			#undef P
			#undef SP
			#undef PC
			#undef ram
			#undef halted
static struct CPU6502 b_cpu;
static void b_sys(unsigned addr)
{
	unsigned i;
	static int b_cpu_inited = 0;
	if (!b_cpu_inited) { cpu6502_init(&b_cpu, b_mem, NULL, NULL); b_cpu_inited = 1; }
	b_cpu.SP = 0xFD;
	b_mem[0x01FE] = 0xFE;
	b_mem[0x01FF] = 0xFF;
	b_cpu.PC = (unsigned short)(addr & 0xFFFF);
	b_cpu.halted = 0;
	for (i = 0; i < 200000 && !cpu6502_is_halted(&b_cpu); i++)
	{
		cpu6502_step(&b_cpu);
		if (b_cpu.PC == 0xFFFF || b_cpu.halted) break;
	}
	b_any_poke = 1;
}
		#else
static void b_sys(unsigned addr) { (void)addr; }
		#endif
	#else
static void b_sys(unsigned addr) { (void)addr; }
	#endif

static breal_t b_sin(breal_t x)
{
		#ifdef B_NO_FLOATMATH
	(void)x;
	return 0;
		#else
	return sin(x);
		#endif
}
static breal_t b_cos(breal_t x)
{
		#ifdef B_NO_FLOATMATH
	(void)x;
	return 0;
		#else
	return cos(x);
		#endif
}
static breal_t b_tan(breal_t x)
{
		#ifdef B_NO_FLOATMATH
	(void)x;
	return 0;
		#else
	return tan(x);
		#endif
}
static breal_t b_atn(breal_t x)
{
		#ifdef B_NO_FLOATMATH
	(void)x;
	return 0;
		#else
	return atan(x);
		#endif
}
static breal_t b_exp(breal_t x)
{
		#ifdef B_NO_FLOATMATH
	(void)x;
	return 0;
		#else
	return exp(x);
		#endif
}
static breal_t b_log(breal_t x)
{
		#ifdef B_NO_FLOATMATH
	(void)x;
	return 0;
		#else
	return x <= 0 ? 0 : log(x);
		#endif
}
static breal_t b_pow(breal_t x, breal_t y)
{
		#ifdef B_NO_FLOATMATH
	breal_t r;
	long yi;
	int i;
	if (x == 0) return 0;
	yi = (long)y;
	if ((breal_t)yi == y) { r = 1; if (yi < 0) { x = 1.0 / x; yi = -yi; } for (i = 0; i < yi; i++) r *= x; return r; } (void)y; return 0;
		#else
	return pow(x, y);
		#endif
}
static breal_t b_sqr(breal_t x)
{
		#ifdef B_NO_FLOATMATH
	breal_t r;
	int i;
	if (x <= 0) return 0;
	r = x;
	for (i = 0; i < 40; i++) { r = (r + x / r) / 2; } return r;
		#else
	return x < 0 ? 0 : sqrt(x);
		#endif
}

static int b_getline(char* out, int max)
{
	int i = 0, c;
	while (i < max - 1) { c = b_getin(); if (c < 0 || c == 0x0d) break; out[i++] = (char)c; }
	out[i] = 0;
	return i;
}

static void b_flush(void)
{
		#if defined(TARGET_C64) || defined(TARGET_AVR)
		#else
	b_cnt_flush();
	if (b_present_hook) b_present_hook();
		#endif
}

static void b_scroll(void)
{
		#if defined(TARGET_AVR)
		#else
	int i;
	memmove(b_scr, b_scr + 40, 24 * 40);
	memmove(b_colram, b_colram + 40, 24 * 40);
	for (i = 0; i < 40; i++) { b_scr[24 * 40 + i] = (unsigned char)' '; b_colram[24 * 40 + i] = 14; }
		#endif
}
static void b_str(const char* s, int len)
{
	int i;
	for (i = 0; i < len; i++) b_putc((unsigned char)s[i]);
}
static inline void b_putint(bint_t v)
{
	char tmp[24];
	int n = 0;
	if (v >= 0) b_putc(' ');
	else { b_putc('-'); v = -v; }
	if (v == 0) tmp[n++] = '0';
	else while (v > 0 && n < 23) { tmp[n++] = (char)('0' + (int)(v % 10)); v /= 10; }
	while (n > 0) b_putc((unsigned char)tmp[--n]);
	b_putc(' ');
}
	#if defined(TARGET_AVR) && B_TIER <= B_TINY
static void b_ftoa(char* buf, breal_t v) { (void)v; buf[0] = '0'; buf[1] = 0; }
static void b_real(breal_t v) { (void)v; }
	#else
static void b_ftoa(char* buf, breal_t v)
{
	char d[16];
	int neg = 0, e = 0, i, n;
	long iv;
	DOUBLE w;
	if (v < 0) { neg = 1; v = -v; }
	if (v == 0) { buf[0] = '0'; buf[1] = 0; return; }
	w = (DOUBLE)v;
	if (w >= 10) { int g = 0; do { w /= 10.0; e++; } while (w >= 10.0 && ++g < 400); }
	else if (w < 1) { int g = 0; do { w *= 10.0; e--; } while (w < 1.0 && ++g < 400); }
	iv = (long)(w * 100000000.0 + 0.5);
	if (iv >= 1000000000L) { iv = 100000000L; e++; }
	{ long t = iv; int j; for (j = 8; j >= 0; j--) { d[j] = (char)('0' + (int)(t % 10)); t /= 10; } }
	d[9] = 0;
	n = 9;
	while (n > 1 && d[n - 1] == '0') n--;
	i = 0;
	if (neg) buf[i++] = '-';
	if (e >= -4 && e <= 8) {
		int j;
		if (e >= 0)
		{
			if (e + 1 >= n) { for (j = 0; j < n; j++) buf[i++] = d[j]; for (j = n; j <= e; j++) buf[i++] = '0'; }
			else { for (j = 0; j <= e; j++) buf[i++] = d[j]; buf[i++] = '.'; for (j = e + 1; j < n; j++) buf[i++] = d[j]; }
		}
		else
		{
			buf[i++] = '0';
			buf[i++] = '.';
			for (j = 0; j < (-e - 1); j++) buf[i++] = '0';
			for (j = 0; j < n; j++) buf[i++] = d[j];
		}
	}
	else
	{
		int j, x = e;
		buf[i++] = d[0];
		if (n > 1) { buf[i++] = '.'; for (j = 1; j < n; j++) buf[i++] = d[j]; }
		buf[i++] = 'E';
		if (x < 0) { buf[i++] = '-'; x = -x; }
		else buf[i++] = '+';
		{ char eb[8]; int k = 0; if (x == 0) eb[k++] = '0'; while (x) { eb[k++] = (char)('0' + x % 10); x /= 10; } while (k < 2) eb[k++] = '0'; while (k > 0) buf[i++] = eb[--k]; }
	}
	buf[i] = 0;
}
static inline void b_real(breal_t v)
{
	char buf[64];
	int i, k;
	if (v >= 0) b_putc(' ');
	else { b_putc('-'); v = -v; }
	if (v == 0) { b_putc('0'); b_putc(' '); return; }
	b_ftoa(buf, v);
	if (strchr(buf, '.')) { k = (int)strlen(buf); while (k > 1 && buf[k - 1] == '0') buf[--k] = 0; if (k > 1 && buf[k - 1] == '.') buf[--k] = 0; }
	for (i = 0; buf[i]; i++) b_putc((unsigned char)buf[i]);
	b_putc(' ');
}
	#endif
static inline void b_nl(void) { b_putc(0x0d); }
static inline void b_zone(void)
{
	if (b_col < 40) b_col = (b_col / 10 + 1) * 10;
	if (b_col >= 40) { b_col = 0; b_row++; if (b_row >= 25) { b_scroll(); b_row = 24; } }
}
static inline void b_tab(int n) { if (n < 0)n = 0; if (n > 39)n = 39; if (n > b_col) { while (b_col < n) b_putc(' '); } }
static inline void b_spc(int n) { int i; if (n < 0)n = 0; for (i = 0; i < n; i++) b_putc(' '); }
static inline int b_pos(void) { return b_col; }
	#if defined(TARGET_HOST)

static unsigned long long b_fps_prev_us = 0, b_fps_report_us = 0, b_fps_cnt = 0, b_fps_sum = 0, b_fps_max = 0, b_fps_late = 0;
static int b_fps_on = -1;
static const char* b_fps_path = 0;
static FILE* b_fps_fp = 0;
static void b_fps_tick(void)
{
	if (b_fps_on < 0) {
		b_fps_path = getenv("ABC_FPS");
		b_fps_on = b_fps_path != 0;
		if (b_fps_on && !(b_fps_path[0]=='1' && b_fps_path[1]==0)) {
			b_fps_fp = fopen(b_fps_path, "w");
		}
	}
	if (!b_fps_on) return;
	struct timeval tv;
	gettimeofday(&tv, 0);
	unsigned long long now = (unsigned long long)tv.tv_sec * 1000000ULL + tv.tv_usec;
	if (b_fps_prev_us) {
		unsigned long long dt = now - b_fps_prev_us;
		b_fps_cnt++;
		b_fps_sum += dt;
		if (dt > b_fps_max) b_fps_max = dt;
		if (dt > 16667ULL) b_fps_late++;
	}
	b_fps_prev_us = now;
	if (!b_fps_report_us) { b_fps_report_us = now; return; }
	if (now - b_fps_report_us >= 1000000ULL) {
		if (b_fps_cnt) {
			unsigned long long avg = b_fps_sum / b_fps_cnt;
			FILE* out = b_fps_fp ? b_fps_fp : stderr;
			fprintf(out, "fps=%llu avg_us=%llu max_us=%llu late=%llu/%llu\n",
			b_fps_cnt, avg, b_fps_max, b_fps_late, b_fps_cnt);
			if (b_fps_fp) fflush(b_fps_fp);
		}
		b_fps_cnt = 0;
		b_fps_sum = 0;
		b_fps_max = 0;
		b_fps_late = 0;
		b_fps_report_us = now;
	}
}
static void b_frame_sync(void)
{
	static int b_warp = -1;
	if (b_warp < 0) b_warp = getenv("ABC_WARP") != 0;
	b_fps_tick();
	if (b_warp) return;
	if (b_present_hook) b_present_hook();
}
static unsigned long long b_now_us(void)
{
	struct timeval tv;
	gettimeofday(&tv, 0);
	return (unsigned long long)tv.tv_sec * 1000000ULL + tv.tv_usec;
}
static void b_pace60(void)
{
}

static unsigned char b_held_scan = 64;
static int b_held_fresh = 0;
static unsigned long long b_held_until_us = 0;
static unsigned char b_getq[256];
static int b_getq_n = 0;

static int b_key_to_scan(int p)
{
	switch (p) {
		case 0x0d: return 1;
		case 0x91: return 9;
		case 0x11: return 13;
		case 0x9d: return 10;
		case 0x1d: return 18;
		case '3': return 8;
		case 'W': return 9;
		case 'A': return 10;
		case '4': return 11;
		case 'Z': return 12;
		case 'S': return 13;
		case 'E': return 14;
		case '5': return 16;
		case 'R': return 17;
		case 'D': return 18;
		case '6': return 19;
		case 'C': return 20;
		case 'F': return 21;
		case 'T': return 22;
		case 'X': return 23;
		case '7': return 24;
		case 'Y': return 25;
		case 'G': return 26;
		case '8': return 27;
		case 'B': return 28;
		case 'H': return 29;
		case 'U': return 30;
		case 'V': return 31;
		case '9': return 32;
		case 'I': return 33;
		case 'J': return 34;
		case '0': return 35;
		case 'M': return 36;
		case 'K': return 37;
		case 'O': return 38;
		case 'N': return 39;
		case '1': return 56;
		case '2': return 59;
		case ' ': return 60;
		case 'Q': return 62;
		default: return 64;
	}
}
	#endif

static int b_getin(void)
{
		#if defined(TARGET_AVR)
			#ifdef B_UART
	if (UCSR0A & (1 << RXC0)) return (int)UDR0;
			#endif
	return -1;
		#elif defined(TARGET_HOST)

	{ b_fps_tick(); }
	if (b_present_hook) b_present_hook();
	b_ptop = 0;
	int n = b_mem[198];
	if (n > 0) { n--; b_mem[198] = (unsigned char)n; return (int)b_mem[631 + n]; }
	int got = -1;
	if (b_getq_n > 0) { got = b_getq[0]; for (int i = 1; i < b_getq_n; i++) b_getq[i - 1] = b_getq[i]; b_getq_n--; }

	if (got < 0 && b_host_window) b_pace60();
	return got;
		#else
	int n = b_mem[198];
	if (n > 0) { n--; b_mem[198] = (unsigned char)n; return (int)b_mem[631 + n]; }
	return -1;
		#endif
}
static inline unsigned char b_peek(unsigned short addr)
{
		#if defined(__X16__)
	{ unsigned char _o; if (x16_peek_intercept(addr, &_o)) return _o; }
		#endif
		#if defined(TARGET_HOST)

	if (addr == 197) {
		b_frame_sync();
		if (b_now_us() < b_held_until_us) return b_held_scan;
		b_held_scan = 64;
		return 64;
	}
	if (addr == 653) {
		if (b_now_us() < b_held_until_us) { int f = b_held_fresh; b_held_fresh = 0; return (unsigned char)(f ? 0 : 1); }
		return 0;
	}
		#endif
	return b_mem[addr];
}
static void b_wait(unsigned int addr, unsigned char mask, unsigned char xormask) {
		#if defined(TARGET_HOST)

	(void)xormask;
	if (addr == 53265 && (mask & 0x80)) b_frame_sync();
		#else
	(void)addr;
	(void)mask;
	(void)xormask;
		#endif
}
static inline breal_t b_rnd(bint_t x) {
	if (x < 0) b_rndSeed = (unsigned long)(-x) * 2654435761u;
	b_rndSeed = b_rndSeed * 69069u + 12345u;
		#if defined(TARGET_AVR) && B_TIER <= B_TINY
	return (breal_t)((b_rndSeed >> 8) & 0xffffff);
		#else
	return (breal_t)((b_rndSeed >> 8) & 0xffffff) / (breal_t)16777216.0;
		#endif
}
	#if defined(__AVRCC__) && B_TIER >= B_FULL
static bint_t b_d2l(breal_t x){
	union { breal_t f; unsigned long long u; } p; p.f = x;
	unsigned long long u = p.u;
	int e = (int)((u >> 52) & 0x7FF);
	if (e < 1023) return 0;
	if (e >= 1023 + 31) return (u >> 63) ? (bint_t)0x80000000L : (bint_t)0x7FFFFFFFL;
	unsigned long long mant = (u & 0x000FFFFFFFFFFFFFULL) | (1ULL << 52);
	unsigned long long iv = mant >> (52 + 1023 - e);
	return (u >> 63) ? -(bint_t)iv : (bint_t)iv;
}
static breal_t b_int(breal_t x){
	union { breal_t f; unsigned long long u; } p; p.f = x;
	unsigned long long u = p.u;
	int e = (int)((u >> 52) & 0x7FF);
	if (e >= 1023 + 52) return x;
	if (e < 1023) return (u >> 63) ? (breal_t)-1.0 : (breal_t)0.0;
	int sh = 52 + 1023 - e;
	unsigned long long mask = (1ULL << sh) - 1;
	unsigned long long fi = u & ~mask;
	if ((u >> 63) && (u & mask)) fi += (1ULL << sh);
	p.u = fi;
	return p.f;
}
	#else
static inline bint_t b_int(breal_t x) { bint_t i = (bint_t)x; if (x < 0 && (breal_t)i != x) i--; return i; }
	#endif
static inline bint_t b_sgn(breal_t x) { return x < 0 ? -1 : (x > 0 ? 1 : 0); }
static inline breal_t b_abs(breal_t x) { return x < 0 ? -x : x; }
static bint_t b_fre(bint_t x) { (void)x; return 65535; }
static breal_t b_usr(bint_t x) { return (breal_t)x; }
static void b_defusr(int n, unsigned int addr) { (void)n; (void)addr; }
static void b_clr(void) { b_ptop = 0; b_dataPtr = 0; b_gosubSP = 0; }
	#if defined(TARGET_C64) || defined(TARGET_AVR)
static void b_open(int laf, int dev, int sa, bstr_t nm) { (void)laf; (void)dev; (void)sa; (void)nm; }
static void b_close(int laf) { (void)laf; }
static bint_t  b_geth(int laf) { (void)laf; return 0; }
static breal_t b_inputh_field(int laf) { (void)laf; return 0; }
static bstr_t  b_inputh_str(int laf) { bstr_t r; r.p = ""; r.len = 0; (void)laf; return r; }
static void b_loadf(bstr_t nm, int mode, int dev, int sa) { (void)nm; (void)mode; (void)dev; (void)sa; }
static void b_chkout(int laf) { (void)laf; }
static void b_clrch(void) {}
	#else

static char b_seqpool[4096];
static int b_seqptop = 0;
static void b1541_path(const char* fname, char* out, int outsz) {
	int i, n = 0;
	for (i = 0; fname[i] && n < outsz - 5; i++) {
		char c = fname[i];
		if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '-') out[n++] = c;
		else if (c != '.') out[n++] = '_';
	}
	out[n++] = '.';
	out[n++] = 'S';
	out[n++] = 'E';
	out[n++] = 'Q';
	out[n] = 0;
}
static void b1541_spill(int si) {
	char path[80];
	FILE* f;
	if (si < 0) return;
	b1541_path(b_store[si].name, path, sizeof path);
	f = fopen(path, "wb");
	if (!f) return;
	if (b_store[si].len > 0) fwrite(b_store[si].buf, 1, (size_t)b_store[si].len, f);
	fclose(f);
}
static int b1541_load(const char* fname) {
	char path[80];
	FILE* f;
	int si = -1, i;
	b1541_path(fname, path, sizeof path);
	f = fopen(path, "rb");
	if (!f) return -1;
	for (i = 0; i < B1541_STORE; i++) if (!b_store[i].used) { si = i; break; }
	if (si < 0) { fclose(f); return -1; }
	b_store[si].used = 1;
	strncpy(b_store[si].name, fname, 63);
	b_store[si].name[63] = 0;
	{ size_t n = fread(b_store[si].buf, 1, B1541_CAP, f); b_store[si].len = (int)n; }
	fclose(f);
	return si;
}
static void b_open(int laf, int dev, int sa, bstr_t nm)
{
	int i, slot = -1, si = -1, mode, rl, bl, j;
	char raw[80], fname[64], * col;
	for (i = 0; i < 79 && i < nm.len; i++) raw[i] = nm.p[i];
	raw[rl = i] = 0;
	mode = -1;
	for (i = rl - 1; i > 0; i--) if (raw[i - 1] == ',') { char m = raw[i]; if (m == 'R' || m == 'r') mode = 0; else if (m == 'W' || m == 'w') mode = 1; else if (m == 'A' || m == 'a') mode = 2; break; }
	if (mode < 0) mode = (sa >= 1) ? 1 : 0;
	col = strchr(raw, ':');
	{
		const char* base = col ? col + 1 : raw;
		bl = (int)strlen(base);
		for (j = 0; j < bl && base[j] != ',' && j < 63; j++) fname[j] = base[j];
		fname[j] = 0;
	}
	for (i = 0; i < B1541_SLOTS; i++) if (!b_slots[i].used) { slot = i; break; }
	if (slot < 0) { b_mem[144] |= 0x02; return; }
	if (dev == 3 || dev == 0) {
		b_slots[slot].used = 1;
		b_slots[slot].laf = laf;
		b_slots[slot].dev = dev;
		b_slots[slot].mode = mode;
		b_slots[slot].sidx = -1;
		b_slots[slot].pos = 0;
		b_mem[144] = 0;
		return;
	}
	for (i = 0; i < B1541_STORE; i++) if (b_store[i].used && strcmp(b_store[i].name, fname) == 0) { si = i; break; }
	if (mode == 0) {
		if (si < 0) si = b1541_load(fname);
		if (si < 0) { b_mem[144] |= 0x42; return; }
		b_slots[slot].pos = 0;
		b_seqptop = 0;
	}
	else if (mode == 1) {
		if (si < 0) { for (i = 0; i < B1541_STORE; i++) if (!b_store[i].used) { si = i; break; } }
		if (si < 0) { b_mem[144] |= 0x02; return; }
		b_store[si].used = 1;
		strncpy(b_store[si].name, fname, 63);
		b_store[si].name[63] = 0;
		b_store[si].len = 0;
		b_slots[slot].pos = 0;
	}
	else {
		if (si < 0) { for (i = 0; i < B1541_STORE; i++) if (!b_store[i].used) { si = i; break; } }
		if (si < 0) { b_mem[144] |= 0x02; return; }
		b_store[si].used = 1;
		strncpy(b_store[si].name, fname, 63);
		b_store[si].name[63] = 0;
		b_slots[slot].pos = b_store[si].len;
	}
	b_slots[slot].used = 1;
	b_slots[slot].laf = laf;
	b_slots[slot].dev = dev;
	b_slots[slot].mode = mode;
	b_slots[slot].sidx = si;
	b_mem[144] = 0;
	(void)sa;
}
static void b_close(int laf) { int s = b_slot(laf); if (s >= 0) { if (b_slots[s].sidx >= 0 && (b_slots[s].mode == 1 || b_slots[s].mode == 2)) b1541_spill(b_slots[s].sidx); if (b_out_laf == s) b_out_laf = -1; if (b_in_laf == s) b_in_laf = -1; b_slots[s].used = 0; } }
static bint_t b_geth(int laf)
{
	int s = b_slot(laf), si, c;
	if (s < 0 || b_slots[s].sidx < 0) { b_mem[144] |= 0x40; return 0; }
	if (b_slots[s].mode != 0) { b_mem[144] |= 0x02; return 0; }
	si = b_slots[s].sidx;
	if (b_slots[s].pos >= b_store[si].len) { b_mem[144] |= 0x40; return 0; }
	b_mem[144] = 0;
	c = b_store[si].buf[b_slots[s].pos++];
	if (b_slots[s].pos >= b_store[si].len) b_mem[144] |= 0x40;
	return (bint_t)c;
}
static breal_t b_inputh_field(int laf)
{
	int s = b_slot(laf), si, n = 0, c;
	char buf[64];
	if (s < 0 || b_slots[s].sidx < 0) { b_mem[144] |= 0x40; return 0; }
	si = b_slots[s].sidx;
	while (b_slots[s].pos < b_store[si].len) { c = b_store[si].buf[b_slots[s].pos]; if (c == ' ' || c == ',' || c == 0x0d || c == 0x0a) b_slots[s].pos++; else break; }
	while (b_slots[s].pos < b_store[si].len && n < 63) { c = b_store[si].buf[b_slots[s].pos]; if (c == ',' || c == 0x0d || c == 0x0a) break; buf[n++] = (char)c; b_slots[s].pos++; }
	buf[n] = 0;
	if (b_slots[s].pos >= b_store[si].len) b_mem[144] |= 0x40;
	return b_atof(buf);
}
static bstr_t b_inputh_str(int laf)
{
	int s = b_slot(laf), si, n = 0, c, i;
	char buf[256];
	bstr_t r;
	if (s < 0 || b_slots[s].sidx < 0) { r.p = ""; r.len = 0; b_mem[144] |= 0x40; return r; }
	si = b_slots[s].sidx;
	while (b_slots[s].pos < b_store[si].len) { c = b_store[si].buf[b_slots[s].pos]; if (c == ' ' || c == ',' || c == 0x0d || c == 0x0a) b_slots[s].pos++; else break; }
	while (b_slots[s].pos < b_store[si].len && n < 255) { c = b_store[si].buf[b_slots[s].pos]; if (c == ',' || c == 0x0d || c == 0x0a) break; buf[n++] = (char)c; b_slots[s].pos++; }
	if (b_slots[s].pos >= b_store[si].len) b_mem[144] |= 0x40;
	if (b_seqptop + n > (int)sizeof(b_seqpool)) n = (int)sizeof(b_seqpool) - b_seqptop;
	r.p = b_seqpool + b_seqptop;
	r.len = n;
	for (i = 0; i < n; i++) b_seqpool[b_seqptop + i] = buf[i];
	b_seqptop += n;
	return r;
}
static void b_loadf(bstr_t nm, int mode, int dev, int sa) { (void)nm; (void)mode; (void)dev; (void)sa; }
static void b_chkout(int laf) { int s = b_slot(laf); b_out_laf = (s >= 0) ? s : -1; }
static void b_clrch(void) { b_out_laf = -1; }
	#endif

	#if defined(TARGET_AVR) && !defined(__AVRCC__)
		#define b_lit(s, n) b_lit_fn(__COUNTER__, (s), (n))
		#define b_lit_fn(c, s, n) ({ static char _bl_##c[] = (s); bstr_t _r; _r.p = _bl_##c; _r.len = (n); _r; })
	#elif defined(__AVRCC__)
		#define b_lit(s, n) {(s), (n)}
	#else
static bstr_t b_lit(const char* s, int len) { bstr_t r; r.p = s; r.len = len; return r; }
	#endif
static bstr_t b_dup(const char* s, int len)
{
	bstr_t r;
	int i;
	if (len < 0) len = 0;
	if (b_ptop + len > (int)sizeof(b_pool)) len = (int)sizeof(b_pool) - b_ptop;
	r.p = b_pool + b_ptop;
	r.len = len;
	for (i = 0; i < len; i++) b_pool[b_ptop + i] = s[i];
	b_ptop += len;
	return r;
}

	#if defined(TARGET_AVR) && !defined(__AVRCC__)
static char b_keep_pool[1];
static int b_keep_top = 0;
	#elif defined(TARGET_AVR)
		#ifndef B_KEEP_SIZE
			#define B_KEEP_SIZE 256
		#endif
static char b_keep_pool[B_KEEP_SIZE];
static int b_keep_top = 0;
	#elif defined(TARGET_C64)
		#ifndef B_KEEP_SIZE
			#define B_KEEP_SIZE 512
		#endif
static char b_keep_pool[B_KEEP_SIZE];
static int b_keep_top = 0;
	#else
static char b_keep_pool[65536];
static int b_keep_top = 0;
	#endif
static bstr_t b_keep(bstr_t s)
{
	bstr_t r;
	int i;
	if (s.len <= 0) return s;
	if (!(s.p >= b_pool && s.p < b_pool + (int)sizeof(b_pool))) return s;
	if (b_keep_top + s.len > (int)sizeof(b_keep_pool)) return s;
	r.p = b_keep_pool + b_keep_top;
	r.len = s.len;
	for (i = 0; i < s.len; i++) b_keep_pool[b_keep_top + i] = s.p[i];
	b_keep_top += s.len;
	return r;
}
static bstr_t b_concat(bstr_t a, bstr_t b)
{
	bstr_t r;
	int i;
	if (b_ptop + a.len + b.len > (int)sizeof(b_pool)) { r.p = b_pool; r.len = 0; return r; }
	r.p = b_pool + b_ptop;
	r.len = a.len + b.len;
	for (i = 0; i < a.len; i++) b_pool[b_ptop + i] = a.p[i];
	for (i = 0; i < b.len; i++) b_pool[b_ptop + a.len + i] = b.p[i];
	b_ptop += a.len + b.len;
	return r;
}
static bstr_t b_chr(bint_t c) { char ch = (char)c; return b_dup(&ch, 1); }
static bint_t b_asc(bstr_t s) { return s.len > 0 ? (bint_t)(unsigned char)s.p[0] : 0; }
static bint_t b_len(bstr_t s) { return s.len; }
static bstr_t b_left(bstr_t s, bint_t n) { if (n < 0)n = 0; if (n > s.len)n = s.len; return b_dup(s.p, (int)n); }
static bstr_t b_right(bstr_t s, bint_t n) { if (n < 0)n = 0; if (n > s.len)n = s.len; return b_dup(s.p + (s.len - n), (int)n); }
static bstr_t b_mid(bstr_t s, bint_t start, bint_t n)
{
	int o = (int)start - 1;
	if (o < 0)o = 0;
	if (o > s.len)o = s.len;
	if (n < 0)n = 0;
	if (o + n > s.len)n = s.len - o;
	return b_dup(s.p + o, (int)n);
}
static bstr_t b_strs(breal_t v)
{
	char buf[64];
	int k;
	if (v >= 0) { buf[0] = ' '; b_ftoa(buf + 1, v); }
	else b_ftoa(buf, v);
	if (strchr(buf, '.')) { k = (int)strlen(buf); while (k > 1 && buf[k - 1] == '0') buf[--k] = 0; if (k > 1 && buf[k - 1] == '.') buf[--k] = 0; }
	return b_dup(buf, (int)strlen(buf));
}
static breal_t b_atof(const char* s)
{
	breal_t r = 0, frac = 0, m = 1, x;
	int neg = 0, eneg = 0, e = 0, got = 0, i;
	while (*s == ' ' || *s == '\t') s++;
	if (*s == '-') { neg = 1; s++; }
	else if (*s == '+') s++;
	while (*s >= '0' && *s <= '9') { r = r * (breal_t)10.0 + (breal_t)(*s - '0'); s++; got = 1; }
	if (*s == '.') { s++; while (*s >= '0' && *s <= '9') { frac = frac * (breal_t)10.0 + (breal_t)(*s - '0'); m *= (breal_t)10.0; s++; got = 1; } }
	if (*s == 'E' || *s == 'e') { s++; if (*s == '-') { eneg = 1; s++; } else if (*s == '+') s++; while (*s >= '0' && *s <= '9') { e = e * 10 + (*s - '0'); s++; } }
	if (!got) return 0;
	r = r + frac / m;
	if (e) { x = (breal_t)10.0; for (i = 1; i < e; i++) x *= (breal_t)10.0; r = eneg ? r / x : r * x; }
	return neg ? -r : r;
}
static breal_t b_val(bstr_t s)
{
	char buf[64];
	int i, n = s.len < 60 ? s.len : 60;
	breal_t r = 0;
	for (i = 0; i < n; i++) buf[i] = s.p[i];
	buf[n] = 0;
	r = b_atof(buf);
	return r;
}
static int b_strcmp(bstr_t a, bstr_t b)
{
	int i, m = a.len < b.len ? a.len : b.len;
	for (i = 0; i < m; i++)
	{
		if ((unsigned char)a.p[i] != (unsigned char)b.p[i])
		return (unsigned char)a.p[i] < (unsigned char)b.p[i] ? -1 : 1;
	}
	if (a.len == b.len) return 0;
	return a.len < b.len ? -1 : 1;
}
static void b_midstmt(bstr_t* d, bint_t start, bint_t n, bstr_t r)
{
	int o, k, i;
	if (start < 1) start = 1;
	o = (int)start - 1;
	if (o > (int)d->len) o = (int)d->len;
	k = (int)d->len - o;
	if (n >= 0 && n < k) k = (int)n;
	if (k > (int)r.len) k = (int)r.len;
	*d = b_dup(d->p, d->len);
	for (i = 0; i < k; i++) ((char*)d->p)[o + i] = r.p[i];
}

static bint_t b_read_int(void)
{
	if (b_dataPtr >= b_data_len) { b_dataPtr++; return 0; }
	b_datum d = b_data[b_dataPtr++];
	if (d.kind == 0) return d.i;
	if (d.kind == 1) return
		#if defined(__AVRCC__) && B_TIER >= B_FULL
	b_d2l(b_int(d.f));
		#else
	(bint_t)b_int(d.f);
		#endif
	{ bstr_t s; s.p = d.s; s.len = d.slen; return (bint_t)b_val(s); }
}
static breal_t b_read_real(void)
{
	if (b_dataPtr >= b_data_len) { b_dataPtr++; return 0; }
	b_datum d = b_data[b_dataPtr++];
	if (d.kind == 1) return d.f;
	if (d.kind == 0) return (breal_t)d.i;
	{ bstr_t s; s.p = d.s; s.len = d.slen; return b_val(s); }
}
static bstr_t b_read_str(void)
{
	if (b_dataPtr >= b_data_len) { b_dataPtr++; return b_strs((breal_t)0); }
	b_datum d = b_data[b_dataPtr++];
	if (d.kind == 2) { bstr_t r; r.p = d.s; r.len = d.slen; return r; }
	if (d.kind == 0) return b_strs((breal_t)d.i);
	return b_strs(d.f);
}

	#if defined(TARGET_AVR) && B_TIER <= B_TINY
static void b_input_num(breal_t* out, const char* prompt, int plen)
{
	(void)prompt;
	(void)plen;
	*out = (breal_t)0;
}
static void b_input_str(bstr_t* out, const char* prompt, int plen)
{
	(void)prompt;
	(void)plen;
	out->p = "";
	out->len = 0;
}
	#else
static void b_input_num(breal_t* out, const char* prompt, int plen)
{
	char buf[128];
	breal_t r = 0;
	if (plen > 0) b_str(prompt, plen);
	b_putc('?');
	b_putc(' ');
	b_getline(buf, (int)sizeof(buf));
	r = b_atof(buf);
	*out = r;
}
		#ifndef B_INPUT_BUF_SIZE
			#if defined(TARGET_AVR)
				#define B_INPUT_BUF_SIZE 64
			#else
				#define B_INPUT_BUF_SIZE 512
			#endif
		#endif
static void b_input_str(bstr_t* out, const char* prompt, int plen)
{
	static char buf[B_INPUT_BUF_SIZE];
	int n;
	if (plen > 0) b_str(prompt, plen);
	b_putc('?');
	b_putc(' ');
	n = b_getline(buf, (int)sizeof(buf));
	*out = b_keep(b_dup(buf, n));
}
	#endif

static int b_input_field(char* out, int max)
{
	int i = 0, c, col = b_col, row = b_row;
		#if defined(TARGET_HOST)
	if (b_need_cr) {
		b_need_cr = 0;
		for (;;) {
			int k = b_getin();
			if (k < 0) { struct timespec ts; ts.tv_sec = 0; ts.tv_nsec = 16 * 1000000L; nanosleep(&ts, 0); continue; }
			if (k == 0x0d) break;
			if (k == 0x14 || k == 0x7f) {
				if (b_col > col) {
					b_col--;
					b_scr[b_row * 40 + b_col] = (unsigned char)' ';
					b_colram[b_row * 40 + b_col] = b_mem[646];
					b_any_poke = 1;
					b_cnt_bump();
				}
				continue;
			}
			if (k >= 0x20 && k < 0x7f) b_putc((unsigned char)k);
		}
	}
		#endif
	while (col < 40) { c = b_scr[row * 40 + col]; if (c == (unsigned char)' ') col++; else break; }
	while (col < 40) {
		c = b_scr[row * 40 + col];
		if (c == (unsigned char)',') { col++; break; }
		if (i >= max - 1) { col++; break; }
		out[i++] = (char)c;
		col++;
	}
	while (i > 0 && out[i - 1] == ' ') i--;
	if (i < max) out[i] = 0;
	b_col = col;
	return i;
}

	#if defined(TARGET_AVR)

		#if !defined(__AVRCC__) && defined(F_CPU)
			#include <util/delay.h>
		#endif

		#ifdef __AVRCC__

			#define ADMUX    (*(volatile unsigned char*)0x7C)
			#define ADCSRA   (*(volatile unsigned char*)0x7A)
			#define ADCSRB   (*(volatile unsigned char*)0x7B)
			#define ADCL     (*(volatile unsigned char*)0x78)
			#define ADCH     (*(volatile unsigned char*)0x79)
			#define EECR     (*(volatile unsigned char*)0x3F)
			#define EEDR     (*(volatile unsigned char*)0x40)
			#define EEARL    (*(volatile unsigned char*)0x41)
			#define EEARH    (*(volatile unsigned char*)0x42)

			#define ADEN     7
			#define ADSC     6
			#define ADIF     4
			#define ADPS2    2
			#define ADPS1    1
			#define ADPS0    0

			#define REFS0    6
			#define REFS1    7

			#define EERE     0
			#define EEPE     1
			#define EEMPE    2

			#define WDTCSR   (*(volatile unsigned char*)0x60)
			#define WDIF     7
			#define WDIE     6
			#define WDP3     5
			#define WDCE     4
			#define WDE      3
			#define WDP2     2
			#define WDP1     1
			#define WDP0     0

			#define SPCR     (*(volatile unsigned char*)0x4C)
			#define SPSR     (*(volatile unsigned char*)0x4D)
			#define SPDR     (*(volatile unsigned char*)0x4E)
			#define SPIF     7
			#define SPE      6
			#define MSTR     4

			#define TWBR     (*(volatile unsigned char*)0xB8)
			#define TWSR     (*(volatile unsigned char*)0xB9)
			#define TWAR     (*(volatile unsigned char*)0xBA)
			#define TWDR     (*(volatile unsigned char*)0xBB)
			#define TWCR     (*(volatile unsigned char*)0xBC)
			#define TWAMR    (*(volatile unsigned char*)0xBD)
			#define TWINT    7
			#define TWEA     6
			#define TWSTA    5
			#define TWSTO    4
			#define TWEN     2

			#define UDR0     (*(volatile unsigned char*)0xC6)
			#define UCSR0A   (*(volatile unsigned char*)0xC0)
			#define UCSR0B   (*(volatile unsigned char*)0xC1)
			#define UCSR0C   (*(volatile unsigned char*)0xC2)
			#define UBRR0L   (*(volatile unsigned char*)0xC4)
			#define UBRR0H   (*(volatile unsigned char*)0xC5)
			#define RXC0     7
			#define TXC0     6
			#define UDRE0    5
			#define TXEN0    3
			#define RXEN0    4
			#define UCSZ00   1
			#define UCSZ01   2
		#endif

static void b_sleep_ms(bint_t ms)
{
	if (ms < 0) ms = 0;
			#if !defined(__AVRCC__) && defined(F_CPU)
	while (ms-- > 0) _delay_ms(1.0);
			#else
	while (ms-- > 0) {
		volatile unsigned long i = (F_CPU / 4000UL);
		if (!i) i = 2000;
		while (i--) __asm__ __volatile__("nop");
	}
			#endif
}

static bint_t b_adc_read(bint_t channel)
{
	unsigned char lo, hi;
	ADMUX = (unsigned char)((1 << REFS0) | (channel & 0x07));
	ADCSRA = (unsigned char)((1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0));
	ADCSRA |= (unsigned char)(1 << ADSC);
	while (ADCSRA & (1 << ADSC)) ;
	lo = ADCL;
	hi = ADCH;
	return (bint_t)(lo | (hi << 8));
}

static bint_t b_eeprom_read(bint_t addr)
{
	while (EECR & (1 << EEPE)) ;
	EEARL = (unsigned char)(addr & 0xFF);
	EEARH = (unsigned char)((addr >> 8) & 0xFF);
	EECR |= (unsigned char)(1 << EERE);
	return (bint_t)EEDR;
}

static void b_eeprom_write(bint_t addr, bint_t val)
{
	while (EECR & (1 << EEPE)) ;
	EEARL = (unsigned char)(addr & 0xFF);
	EEARH = (unsigned char)((addr >> 8) & 0xFF);
	EEDR  = (unsigned char)(val & 0xFF);
	EECR |= (unsigned char)(1 << EEMPE);
	EECR |= (unsigned char)(1 << EEPE);
	while (EECR & (1 << EEPE)) ;
}

static bint_t b_map(bint_t x, bint_t in_min, bint_t in_max, bint_t out_min, bint_t out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

static bint_t b_constrain(bint_t x, bint_t lo, bint_t hi)
{
	return (x < lo) ? lo : (x > hi) ? hi : x;
}

static bint_t b_random(bint_t max)
{
	if (max <= 0) return 0;
	b_rndSeed = b_rndSeed * 1103515245u + 12345u;
	return (bint_t)((b_rndSeed >> 16) % (unsigned long)max);
}

static void b_wdt_enable(bint_t ms)
{
	unsigned char wdp;
	if      (ms <    24) wdp = (0<<WDP3)|(0<<WDP2)|(0<<WDP1)|(0<<WDP0);
	else if (ms <    48) wdp = (0<<WDP3)|(0<<WDP2)|(0<<WDP1)|(1<<WDP0);
	else if (ms <    96) wdp = (0<<WDP3)|(0<<WDP2)|(1<<WDP1)|(0<<WDP0);
	else if (ms <   190) wdp = (0<<WDP3)|(0<<WDP2)|(1<<WDP1)|(1<<WDP0);
	else if (ms <   380) wdp = (0<<WDP3)|(1<<WDP2)|(0<<WDP1)|(0<<WDP0);
	else if (ms <   760) wdp = (0<<WDP3)|(1<<WDP2)|(0<<WDP1)|(1<<WDP0);
	else if (ms <  1500) wdp = (0<<WDP3)|(1<<WDP2)|(1<<WDP1)|(0<<WDP0);
	else if (ms <  3000) wdp = (0<<WDP3)|(1<<WDP2)|(1<<WDP1)|(1<<WDP0);
	else if (ms <  6000) wdp = (1<<WDP3)|(0<<WDP2)|(0<<WDP1)|(0<<WDP0);
	else                 wdp = (1<<WDP3)|(0<<WDP2)|(0<<WDP1)|(1<<WDP0);
	__asm__ __volatile__("wdr");
	WDTCSR |= (unsigned char)((1<<WDCE)|(1<<WDE));
	WDTCSR  = (unsigned char)((1<<WDE) | wdp);
}

static void b_wdt_disable(void)
{
	__asm__ __volatile__("wdr");
	WDTCSR |= (unsigned char)((1<<WDCE)|(1<<WDE));
	WDTCSR  = 0;
}

static void b_spi_init(void)
{
	SPCR = (unsigned char)((1<<SPE)|(1<<MSTR));
	(void)SPSR;
}

static bint_t b_spi_transfer(bint_t b)
{
	SPDR = (unsigned char)(b & 0xFF);
	while (!(SPSR & (1<<SPIF))) ;
	return (bint_t)SPDR;
}

static void b_twi_start(void)
{
	static unsigned char inited = 0;
	if (!inited) { TWBR = 72; TWSR = 0; TWCR = 0; inited = 1; }
	TWCR = (unsigned char)((1<<TWINT)|(1<<TWEN)|(1<<TWSTA));
	while (!(TWCR & (1<<TWINT))) ;
}

static void b_twi_stop(void)
{
	TWCR = (unsigned char)((1<<TWINT)|(1<<TWEN)|(1<<TWSTO));
}

static bint_t b_twi_write(bint_t b)
{
	TWDR = (unsigned char)(b & 0xFF);
	TWCR = (unsigned char)((1<<TWINT)|(1<<TWEN));
	while (!(TWCR & (1<<TWINT))) ;
	return (bint_t)(((TWSR & 0xF8) == 0x28) ? 1 : 0);
}

static bint_t b_twi_read(bint_t ack)
{
	TWCR = (unsigned char)((1<<TWINT)|(1<<TWEN)|(ack ? (1<<TWEA) : 0));
	while (!(TWCR & (1<<TWINT))) ;
	return (bint_t)TWDR;
}

		#if defined(__AVRCC__) || defined(__AVR_ATmega328P__) || defined(__AVR_ATmega328PB__)

static const unsigned char b_uno_port[3] = { 0x2B, 0x25, 0x28 };
static const unsigned char b_uno_ddr [3] = { 0x2A, 0x24, 0x27 };
static const unsigned char b_uno_pin [3] = { 0x29, 0x23, 0x26 };
static const unsigned char b_uno_base[3] = { 0, 8, 14 };

static int b_uno_lookup(bint_t pin, unsigned char* port_a,
unsigned char* ddr_a, unsigned char* pin_a, unsigned char* bit)
{
	int g;
	if (pin < 0) return -1;
	if (pin <= 7) g = 0;
	else if (pin <= 13) g = 1;
	else if (pin <= 19) g = 2;
	else return -1;
	*port_a = b_uno_port[g];
	*ddr_a = b_uno_ddr[g];
	*pin_a = b_uno_pin[g];
	*bit = (unsigned char)(pin - b_uno_base[g]);
	return g;
}

static void b_pin_mode(bint_t pin, bint_t mode)
{
	unsigned char port_a, ddr_a, pin_a, bit;
	if (b_uno_lookup(pin, &port_a, &ddr_a, &pin_a, &bit) < 0) return;
	if (mode == 1) {
		b_mem[ddr_a]  |= (unsigned char)(1 << bit);
		b_mem[port_a] &= (unsigned char)~(1 << bit);
	} else if (mode == 2) {
		b_mem[ddr_a]  &= (unsigned char)~(1 << bit);
		b_mem[port_a] |= (unsigned char)(1 << bit);
	} else {
		b_mem[ddr_a]  &= (unsigned char)~(1 << bit);
		b_mem[port_a] &= (unsigned char)~(1 << bit);
	}
}

static void b_dwrite(bint_t pin, bint_t val)
{
	unsigned char port_a, ddr_a, pin_a, bit;
	if (b_uno_lookup(pin, &port_a, &ddr_a, &pin_a, &bit) < 0) return;
	if (val) b_mem[port_a] |= (unsigned char)(1 << bit);
	else     b_mem[port_a] &= (unsigned char)~(1 << bit);
}

static bint_t b_dread(bint_t pin)
{
	unsigned char port_a, ddr_a, pin_a, bit;
	if (b_uno_lookup(pin, &port_a, &ddr_a, &pin_a, &bit) < 0) return 0;
	return (bint_t)((b_mem[pin_a] >> bit) & 1);
}

		#endif

	#endif

#endif
