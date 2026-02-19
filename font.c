/* 314CA Stefan Alexandru Vladut*/
#include "font.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "istoric.h"
#include "turtle.h"
#include "comenzi.h"
#define BUFFER_SIZE 30000
#define MAX_GLYPH_DIM 5000

static void del_newline(char *comanda)
{
	int n;
	n = strlen(comanda);
	while (n > 0 && (comanda[n - 1] == '\n' || comanda[n - 1] == '\r')) {
		comanda[n - 1] = '\0';
		n--;
	}
}

static void *my_malloc(int len)
{
	void *p;
	p = malloc(len);
	if (!p) {
		fprintf(stderr, "Eroare: malloc a esuat\n");
		return NULL;
	}
	return p;
}

static char *my_strdup(char *sir)
{
	int n;
	char *copie;
	if (!sir) {
		return NULL;
	}
	n = strlen(sir);
	copie = my_malloc(n + 1);
	memcpy(copie, sir, n + 1);
	return copie;
}

void init_font(font_t *font)
{
	int i;
	font->nume = NULL;
	for (i = 0; i < 256; i++) {
		font->glyph[i].exista = 0;
		font->glyph[i].bitmap = NULL;
		font->glyph[i].dwx = 0;
		font->glyph[i].dwy = 0;
		font->glyph[i].bbw = 0;
		font->glyph[i].bbh = 0;
		font->glyph[i].bbxoff = 0;
		font->glyph[i].bbyoff = 0;
	}
}

void free_glyph(glyph_t *g)
{
	int y;
	if (g->bitmap) {
		for (y = 0; y < g->bbh; y++) {
			if (g->bitmap[y]) {
				free(g->bitmap[y]);
			}
		}
		free(g->bitmap);
		g->bitmap = NULL;
	}
	g->exista = 0;
}

//transformarea din hexazecimal in sir de biti
unsigned char *parsare_bitmap(char *hex, int latime)
{
	int i, bit;
	int n = (int)strlen(hex);
	unsigned char *sir = my_malloc((size_t)latime);
	memset(sir, 0, (size_t)latime);

	for (i = 0; i < n; i++) {
		int rez;
		char c = hex[i];
		if (c >= '0' && c <= '9') {
			rez = c - '0';
		} else if (c >= 'A' && c <= 'F') {
			rez = c - 'A' + 10;
		} else {
			rez = 0;
		}
		for (bit = 0; bit < 4; bit++) {
			int poz = i * 4 + bit;
			if (poz < latime) {
				sir[poz] = (unsigned char)((rez >> (3 - bit)) & 1);
			}
		}
	}
	return sir;
}

//citirea fontului bitmap, salvarea structuii curente din font_t
font_t *incarca_font(char *nume_fisier)
{
	FILE *f;
	font_t *font;
	char *lin;
	int cod = -1;

	f = fopen(nume_fisier, "r");
	lin = (char *)malloc(BUFFER_SIZE * sizeof(char));
	if (!lin) {
		fclose(f);
		return NULL;
	}
	font = my_malloc(sizeof(*font));
	init_font(font);

	while (fgets(lin, BUFFER_SIZE, f)) {
		del_newline(lin);
		if (strncmp(lin, "FONT ", 5) == 0 && !font->nume) {
			font->nume = my_strdup(lin + 5);
			continue;
		}

		if (strncmp(lin, "ENCODING ", 9) == 0) {
			if (sscanf(lin, "ENCODING %d", &cod) != 1) {
				cod = -1;
			}
			continue;
		}

		if (strncmp(lin, "DWIDTH ", 7) == 0 && cod >= 0 && cod < 256) {
			if (sscanf(lin, "DWIDTH %d %d", &font->glyph[cod].dwx,
					   &font->glyph[cod].dwy) != 2) {
			}
		}

		if (strncmp(lin, "BBX ", 4) == 0 && cod >= 0 && cod < 256) {
			glyph_t *g = &font->glyph[cod];
			if (sscanf(lin, "BBX %d %d %d %d", &g->bbw, &g->bbh,
					   &g->bbxoff, &g->bbyoff) != 4) {
			}
		}

		if (strncmp(lin, "BITMAP", 6) == 0 && cod >= 0 && cod < 256) {
			glyph_t *g = &font->glyph[cod];
			int y;

			if (g->bbh <= 0 || g->bbw <= 0 ||
				g->bbh > MAX_GLYPH_DIM || g->bbw > MAX_GLYPH_DIM) {
				g->exista = 0;
				continue;
			}
			g->bitmap = my_malloc((size_t)g->bbh * sizeof(unsigned char *));
			for (y = 0; y < g->bbh; y++) {
				if (!fgets(lin, BUFFER_SIZE, f)) {
					break;
				}
				del_newline(lin);
				g->bitmap[y] = parsare_bitmap(lin, g->bbw);
			}
			g->exista = 1;
		}

		if (strncmp(lin, "ENDCHAR", 7) == 0) {
			cod = -1;
		}
	}
	free(lin);
	fclose(f);

	if (!font->nume) {
		font->nume = my_strdup("Unknown");
	}
	return font;
}

//creare deep copy a fontului
font_t *copiaza_font(font_t *font)
{
	font_t *sir;
	int ch, i;

	if (!font) {
		return NULL;
	}
	sir = my_malloc(sizeof(*sir));
	init_font(sir);

	if (font->nume) {
		sir->nume = my_strdup(font->nume);
	} else {
		sir->nume = my_strdup("Unknown");
	}

	for (ch = 0; ch < 256; ch++) {
		const glyph_t *glyph = &font->glyph[ch];
		glyph_t *cglyph = &sir->glyph[ch];
		*cglyph = *glyph;
		cglyph->bitmap = NULL;

		if (glyph->exista && glyph->bbh > 0 && glyph->bitmap) {
			cglyph->bitmap = my_malloc((size_t)glyph->bbh *
									   sizeof(unsigned char *));
			for (i = 0; i < glyph->bbh; i++) {
				cglyph->bitmap[i] = my_malloc((size_t)glyph->bbw);
				memcpy(cglyph->bitmap[i], glyph->bitmap[i], (size_t)glyph->bbw);
			}
		} else {
			if (!glyph->bitmap) {
				cglyph->exista = 0;
			}
		}
	}
	return sir;
}

void free_font(font_t *font)
{
	int i;
	if (!font) {
		return;
	}
	for (i = 0; i < 256; i++) {
		free_glyph(&font->glyph[i]);
	}
	free(font->nume);
	free(font);
}

//actualizarea fontului curent din bitmap
void comanda_font(aplicatie_t *aplicatie, const char *comanda)
{
	char fisier[256];
	char **mesaj;
	font_t *font;
	if (sscanf(comanda, "FONT %s", fisier) != 1) {
		printf("Invalid command\n");
		return;
	}

	font = incarca_font(fisier);
	if (!font) {
		printf("Failed to load %s\n", fisier);
		return;
	}

	start_modificare(aplicatie);
	free_font(aplicatie->stare_curenta.font);
	aplicatie->stare_curenta.font = font;
	printf("Loaded %s (bitmap font %s)\n", fisier, font->nume);
	mesaj = my_malloc(sizeof(char *));
	mesaj[0] = my_strdup("Loaded font");
	final_modificare(aplicatie, mesaj, 1);
}

//scrierea textului cu fontul ales pe imagine
void comanda_type(aplicatie_t *aplicatie, const char *comanda)
{
	char sir[256];
	int x, y, r, g, b;
	char **mesaj;
	int i;
	const char *st, *dr;
	imagine_t *img;

	if (!aplicatie->stare_curenta.imagine) {
		printf("No image loaded\n");
		return;
	}
	img = aplicatie->stare_curenta.imagine;

	if (!aplicatie->stare_curenta.font) {
		printf("No font loaded\n");
		return;
	}

	st = strchr(comanda, '"');
	if (st) {
		dr = strchr(st + 1, '"');
	} else {
		dr = NULL;
	}
	if (!st || !dr) {
		printf("Invalid command\n");
		return;
	}

	int len = (int)(dr - st - 1);
	if (len >= (int)sizeof(sir)) {
		len = (int)sizeof(sir) - 1;
	}
	if (len > 0) {
		strncpy(sir, st + 1, (size_t)len);
	}
	sir[len] = '\0';

	if (sscanf(dr + 1, "%d %d %d %d %d", &x, &y, &r, &g, &b) != 5) {
		printf("Invalid command\n");
		return;
	}

	start_modificare(aplicatie);

	for (i = 0; sir[i] != '\0'; i++) {
		unsigned char idx = (unsigned char)sir[i];
		glyph_t *glyph = &aplicatie->stare_curenta.font->glyph[idx];
		if (glyph->exista) {
			int px, py;
			for (py = 0; py < glyph->bbh; py++) {
				for (px = 0; px < glyph->bbw; px++) {
					int pos_x, pos_y;
					if (!glyph->bitmap[py][px]) {
						continue;
					}
					pos_x = x + glyph->bbxoff + px;
					pos_y = y + glyph->bbyoff + glyph->bbh - 1 - py;
					if (pos_x < 0 || pos_x >= img->latime ||
						pos_y < 0 || pos_y >= img->inaltime) {
						continue;
					}
					colorare_pixel(img, pos_x, pos_y,
								   (pixel_t){
									(unsigned char)r,
									(unsigned char)g,
									(unsigned char)b
								 });
				}
			}
			x += glyph->dwx;
		} else {
			x += glyph->dwx;
		}
	}
	mesaj = my_malloc(sizeof(char *));
	mesaj[0] = my_strdup("Text written");
	printf("%s\n", mesaj[0]);
	final_modificare(aplicatie, mesaj, 1);
}
