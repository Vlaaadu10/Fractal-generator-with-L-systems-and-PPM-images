/* 314CA Stefan Alexandru Vladut*/
#include "turtle.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static void *my_realloc(void *ptr, int len)
{
	void *p;
	p = realloc(ptr, len);
	if (!p) {
		fprintf(stderr, "Eroare: realloc a esuat\n");
		return NULL;
	}
	return p;
}

typedef struct stare_turtle_t {
	double x, y, unghi;
} stare_turtle_t;

typedef struct stiva_turtle_t {
	stare_turtle_t *v;
	int n, vf;
} stiva_turtle_t;

void stiva_init(stiva_turtle_t *s)
{
	s->v = NULL;
	s->n = 0;
	s->vf = 0;
}

void stiva_free(stiva_turtle_t *s)
{
	free(s->v);
	s->v = NULL;
	s->n = 0;
	s->vf = 0;
}

void stiva_push(stiva_turtle_t *s, stare_turtle_t q)
{
	if (s->n == s->vf) {
		int vf_nou;
		if (s->vf == 0) {
			vf_nou = 8;
		} else {
			vf_nou = s->vf * 2;
		}
		s->v = my_realloc(s->v, (size_t)vf_nou * sizeof(stare_turtle_t));
		s->vf = vf_nou;
	}
	s->v[s->n++] = q;
}

int stiva_pop(stiva_turtle_t *s, stare_turtle_t *q)
{
	if (s->n == 0) {
		return 0;
	}
	s->n--;
	*q = s->v[s->n];
	return 1;
}

//trasarea liniei intre doua puncte folosind algoritmul Bresenham
void bresenham(imagine_t *img, int x0, int y0, int x1, int y1,
			   pixel_t c)
{
	int dx = abs(x1 - x0);
	int sx = x0 < x1 ? 1 : -1;
	int dy = -abs(y1 - y0);
	int sy = y0 < y1 ? 1 : -1;
	int err = dx + dy;
	int e2;
	while (1) {
		colorare_pixel(img, x0, y0, c);
		if (x0 == x1 && y0 == y1) {
			break;
		}
		e2 = 2 * err;
		if (e2 >= dy) {
			err += dy;
			x0 += sx;
		}
		if (e2 <= dx) {
			err += dx;
			y0 += sy;
		}
	}
}

//realizeaza conversia de la double la int
int round_int(double v)
{
	return (int)round(v);
}

//deseneaza imaginea in functie de sirul de comenzi
int draw_turtle(imagine_t *img, const char *sir,
				parametri_turtle_t *p)
{
	double x, y, unghi;
	stiva_turtle_t stiva;
	int i;
	x = p->x;
	y = p->y;
	unghi = p->orientare;
	stiva_init(&stiva);

	for (i = 0; sir[i] != '\0'; i++) {
		char c = sir[i];
		if (c == 'F') {
			double rad = unghi * M_PI / 180.0;
			double xn = x + p->pas * cos(rad);
			double yn = y + p->pas * sin(rad);
			bresenham(img, round_int(x), round_int(y),
					  round_int(xn), round_int(yn), p->culoare);
			x = xn;
			y = yn;
		} else if (c == '+') {
			unghi += p->pas_unghiular;
		} else if (c == '-') {
			unghi -= p->pas_unghiular;
		} else if (c == '[') {
			stare_turtle_t st = {x, y, unghi};
			stiva_push(&stiva, st);
		} else if (c == ']') {
			stare_turtle_t st;
			if (stiva_pop(&stiva, &st)) {
				x = st.x;
				y = st.y;
				unghi = st.unghi;
			}
		}
	}
	stiva_free(&stiva);
	return 1;
}
