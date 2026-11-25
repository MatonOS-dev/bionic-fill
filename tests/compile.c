/* SPDX-License-Identifier: MIT */
#define _GNU_SOURCE 1
#include <pthread.h>
#include <stdio.h>
#include "bionic-fill.h"
#include <libintl.h>
static ssize_t rd(void *c, char *b, size_t n) { (void)c; (void)b; return (ssize_t)n; }
static ssize_t wr(void *c, const char *b, size_t n) { (void)c; (void)b; return (ssize_t)n; }
static int sk(void *c, off64_t *o, int w) { (void)c; (void)w; *o = 0; return 0; }
static int cl(void *c) { (void)c; return 0; }
int main(void) {
 cpu_set_t set; CPU_ZERO(&set); CPU_SET(0, &set);
 int r = pthread_getaffinity_np(pthread_self(), sizeof(set), &set);
 r += pthread_setaffinity_np(pthread_self(), sizeof(set), &set);
 cookie_read_function_t *a = rd; cookie_write_function_t *b = wr;
 cookie_seek_function_t *c = sk; cookie_close_function_t *d = cl;
 cookie_io_functions_t io = {a,b,c,d};
 FILE *f = fopencookie(NULL, "w+", io); if(f) { fputs("x",f); fseek(f,0,SEEK_SET); r += fclose(f); }
 f = fopencookie(NULL, "r+", (cookie_io_functions_t){0}); if(f) r += fclose(f);
 (void)gettext("x"); (void)dgettext("d","x"); (void)dcgettext("d","x",0);
 (void)ngettext("x","xs",2); (void)dngettext("d","x","xs",2);
 (void)dcngettext("d","x","xs",2,0); (void)bindtextdomain("d","/");
 (void)bind_textdomain_codeset("d","UTF-8"); (void)textdomain("d"); return r;
}
