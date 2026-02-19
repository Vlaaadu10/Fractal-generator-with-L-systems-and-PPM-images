/* 314CA Stefan Alexandru Vladut*/
#ifndef FONT_H
#define FONT_H

struct aplicatie_t;

//reprezentarea grafica a unui caracter
typedef struct glyph_t {
	int dwx;
	int dwy;
	int bbw;
	int bbh;
	int bbxoff;
	int bbyoff;
	unsigned char **bitmap;
	int exista;
} glyph_t;

typedef struct font_t {
	char *nume;
	glyph_t glyph[256];
} font_t;

font_t *incarca_font(char *nume_fisier);
font_t *copiaza_font(font_t *font);
void free_font(font_t *font);

void comanda_font(struct aplicatie_t *context,
				  const char *linie);
void comanda_type(struct aplicatie_t *context,
				  const char *linie);
#endif
