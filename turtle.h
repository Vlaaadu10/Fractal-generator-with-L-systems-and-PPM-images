/* 314CA Stefan Alexandru Vladut*/
#ifndef TURTLE_H
#define TURTLE_H

typedef struct pixel_t {
	unsigned char r;
	unsigned char g;
	unsigned char b;
} pixel_t;

//reprezentarea imaginii PPM in memorie
typedef struct imagine_t {
	int latime;
	int inaltime;
	pixel_t *pixeli;
} imagine_t;

imagine_t *load_ppm(char *nume_fisier);

int save_ppm(imagine_t *imagine, char *nume_fisier);

void free_imagine(imagine_t *imagine);

void colorare_pixel(imagine_t *imagine, int x, int y, pixel_t culoare);

typedef struct parametri_turtle_t {
	double x, y, pas, orientare, pas_unghiular;
	int derivare;
	pixel_t culoare;
} parametri_turtle_t;

int draw_turtle(imagine_t *imagine, const char *sir,
				parametri_turtle_t *param);
#endif
