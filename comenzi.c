/* 314CA Stefan Alexandru Vladut*/
#include "comenzi.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "font.h"
#include "istoric.h"
#include "sistem_l.h"

//tratarea cazului in care malloc esueaza
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

//crearea unei copii a sirului
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

//citire header fisier PPM format P6
int read_header(FILE *fisier, int *w, int *h)
{
	char magic[3];
	int maxi;
	if (fscanf(fisier, "%s", magic) != 1) {
		return 0;
	}

	if (strcmp(magic, "P6") != 0) {
		return 0;
	}

	if (fscanf(fisier, "%d %d", w, h) != 2) {
		return 0;
	}

	if (fscanf(fisier, "%d", &maxi) != 1) {
		return 0;
	}

	if (maxi != 255) {
		return 0;
	}
	fgetc(fisier);
	return 1;
}

//conversie pixeli de la sus in jos la jos in sus
int index_pixel(imagine_t *imagine, int x, int y)
{
	int y2;
	y2 = imagine->inaltime - 1 - y;
	return y2 * imagine->latime + x;
}

//incarcarea imaginii PPM din fisier
imagine_t *load_ppm(char *nume_fisier)
{
	FILE *fisier;
	imagine_t *imagine;
	int w, h;
	int n;
	fisier = fopen(nume_fisier, "rb");
	if (!fisier) {
		return NULL;
	}
	if (!read_header(fisier, &w, &h)) {
		fclose(fisier);
		return NULL;
	}

	imagine = my_malloc(sizeof(*imagine));
	imagine->latime = w;
	imagine->inaltime = h;

	n = w * h;
	imagine->pixeli = my_malloc(n * sizeof(pixel_t));
	//verificare daca toti pixelii au fost cititi din fisier
	if (fread(imagine->pixeli, sizeof(pixel_t), n, fisier) != (size_t)n) {
		free_imagine(imagine);
		fclose(fisier);
		return NULL;
	}
	fclose(fisier);
	return imagine;
}

//salvarea imaginii in format PPM
int save_ppm(imagine_t *imagine, char *nume_fisier)
{
	FILE *fisier;
	int n;
	fisier = fopen(nume_fisier, "wb");

	fprintf(fisier,
			"P6\n%d %d\n255\n",
			imagine->latime,
			imagine->inaltime);
	n = imagine->latime * imagine->inaltime;
	//verificare daca toti pixelii au fost scrisi in fisier
	if (fwrite(imagine->pixeli, sizeof(pixel_t), n, fisier) != (size_t)n) {
		fclose(fisier);
		return 0;
	}
	fclose(fisier);
	return 1;
}

//colorarea pixelului de la pozitia (x,y)
void colorare_pixel(imagine_t *imagine, int x, int y, pixel_t culoare)
{
	int poz;
	if (x < 0 || y < 0 ||
		x >= imagine->latime ||
		y >= imagine->inaltime) {
		return;
	}
	poz = index_pixel(imagine, x, y);
	imagine->pixeli[poz] = culoare;
}

//deep copy a unei imagini pentru a putea folosi UNDO/REDO
imagine_t *copiaza_imagine(imagine_t *imagine)
{
	imagine_t *copie;
	int n;
	if (!imagine) {
		return NULL;
	}
	copie = my_malloc(sizeof(*copie));
	copie->latime = imagine->latime;
	copie->inaltime = imagine->inaltime;

	n = imagine->latime * imagine->inaltime;
	copie->pixeli = my_malloc((size_t)n * sizeof(pixel_t));
	memcpy(copie->pixeli, imagine->pixeli, (size_t)n * sizeof(pixel_t));
	return copie;
}

void free_imagine(imagine_t *imagine)
{
	if (!imagine) {
		return;
	}
	free(imagine->pixeli);
	free(imagine);
}

void init_aplicatie(aplicatie_t *aplicatie)
{
	aplicatie->stare_curenta.imagine = NULL;
	aplicatie->stare_curenta.sistem_l = NULL;
	aplicatie->stare_curenta.font = NULL;

	aplicatie->undo = init_stiva_istoric();
	aplicatie->redo = init_stiva_istoric();
}

void free_aplicatie(aplicatie_t *aplicatie)
{
	free_stiva_istoric(aplicatie->undo);
	free_stiva_istoric(aplicatie->redo);

	free_imagine(aplicatie->stare_curenta.imagine);
	free_sistem_l(aplicatie->stare_curenta.sistem_l);
	free_font(aplicatie->stare_curenta.font);
}

stare_aplicatie_t *copy_stare(stare_aplicatie_t *stare)
{
	stare_aplicatie_t *copie;
	copie = my_malloc(sizeof(*copie));
	copie->imagine = copiaza_imagine(stare->imagine);
	copie->sistem_l = copiaza_sistem_l(stare->sistem_l);
	copie->font = copiaza_font(stare->font);

	return copie;
}

void free_stare(stare_aplicatie_t *stare)
{
	if (!stare) {
		return;
	}
	free_imagine(stare->imagine);
	free_sistem_l(stare->sistem_l);
	free_font(stare->font);
	free(stare);
}

void reset_stare(aplicatie_t *aplicatie,
				 stare_aplicatie_t *stare)
{
	free_imagine(aplicatie->stare_curenta.imagine);
	free_sistem_l(aplicatie->stare_curenta.sistem_l);
	free_font(aplicatie->stare_curenta.font);

	aplicatie->stare_curenta.imagine = copiaza_imagine(stare->imagine);
	aplicatie->stare_curenta.sistem_l = copiaza_sistem_l(stare->sistem_l);
	aplicatie->stare_curenta.font = copiaza_font(stare->font);
}

//parcurgerea bitilor din imagine
int read_bit(imagine_t *img, int bit_index)
{
	int pixel_index = bit_index / 24;
	int poz_pixel = bit_index % 24;
	int canal = poz_pixel / 8;
	//inversez pozitia pentru a citi corect bit-ul
	int poz_canal = 7 - (poz_pixel % 8);

	if (pixel_index >= img->latime * img->inaltime) {
		return -1;
	}
	pixel_t p = img->pixeli[pixel_index];
	unsigned char rez;
	if (canal == 0) {
		rez = p.r;
	} else if (canal == 1) {
		rez = p.g;
	} else {
		rez = p.b;
	}
	return (rez >> poz_canal) & 1;
}

//inversarea posibilor biti cititi gresit
void flip_bit(imagine_t *img, int bit_index)
{
	int pixel_index = bit_index / 24;
	int poz_pixel = bit_index % 24;
	int canal = poz_pixel / 8;
	int poz_canal = 7 - (poz_pixel % 8);

	int x = pixel_index % img->latime;
	int y2 = pixel_index / img->latime;
	int y = img->inaltime - 1 - y2;

	pixel_t p = img->pixeli[pixel_index];
	int r = p.r, g = p.g, b = p.b;

	//inversam bitul folosind XOR
	int masca = 1 << poz_canal;
	if (canal == 0) {
		r ^= masca;
	} else if (canal == 1) {
		g ^= masca;
	} else {
		b ^= masca;
	}
	printf("Warning: pixel at (%d, %d) may be read as (%d, %d, %d)\n",
		   x, y, r, g, b);
}

//parcurgerea bitilor din imaginie si gasirea unor tipare suspecte
void comanda_bitcheck(aplicatie_t *aplicatie)
{
	imagine_t *img = aplicatie->stare_curenta.imagine;
	if (!img) {
		printf("No image loaded\n");
		return;
	}

	long total_bits = (long)img->latime * img->inaltime * 24;
	for (long i = 0; i < total_bits; i++) {
		int b0 = read_bit(img, i);
		int b1 = read_bit(img, i + 1);
		int b2 = read_bit(img, i + 2);
		int b3 = read_bit(img, i + 3);

		if (b0 == 0 && b1 == 0 && b2 == 1 && b3 == 0)
			flip_bit(img, i + 2);
		else if (b0 == 1 && b1 == 1 && b2 == 0 && b3 == 1)
			flip_bit(img, i + 2);
	}
}

//parsarea parametrilor si crearea sistemului grafic valid
int read_parametri(const char *comanda,
				   parametri_turtle_t *p)
{
	int r, g, b;
	int citite = sscanf(comanda, "TURTLE %lf %lf %lf %lf %lf %d %d %d %d",
					   &p->x, &p->y, &p->pas, &p->orientare, &p->pas_unghiular,
					   &p->derivare, &r, &g, &b);
	if (citite != 9) {
		return 0;
	}
	if (r < 0) {
		r = 0;
	} else if (r > 255) {
		r = 255;
	}
	if (g < 0) {
		g = 0;
	} else if (g > 255) {
		g = 255;
	}
	if (b < 0) {
		b = 0;
	} else if (b > 255) {
		b = 255;
	}
	p->culoare.r = (unsigned char)r;
	p->culoare.g = (unsigned char)g;
	p->culoare.b = (unsigned char)b;
	return 1;
}

//executarea comenzii turtle si a desenarii efective pe imagine
void comanda_turtle(aplicatie_t *aplicatie, const char *comanda)
{
	parametri_turtle_t p;
	char *sir = NULL;
	char **mesaj = NULL;
	if (!aplicatie->stare_curenta.imagine) {
		printf("No image loaded\n");
		return;
	}
	if (!aplicatie->stare_curenta.sistem_l) {
		printf("No L-system loaded\n");
		return;
	}
	if (!read_parametri(comanda, &p)) {
		printf("Invalid command\n");
		return;
	}
	sir = derive_sistem_l(aplicatie->stare_curenta.sistem_l, p.derivare);
	if (!sir) {
		return;
	}
	start_modificare(aplicatie);
	draw_turtle(aplicatie->stare_curenta.imagine, sir, &p);
	mesaj = my_malloc(sizeof(char *));
	mesaj[0] = my_strdup("Drawing done");

	printf("%s\n", mesaj[0]);
	final_modificare(aplicatie, mesaj, 1);
	free(sir);
}

// printeaza mesajul de eroare pentru comenzi care nu pot fi interpretate
void comanda_invalida(void)
{
	printf("Invalid command\n");
}

//executarea comenzii LOAD = incarcare imagine PPM + salvare stare curenta
void comanda_load(aplicatie_t *aplicatie, const char *comanda)
{
	char fisier[1000];
	imagine_t *img;
	char **mesaj;

	if (sscanf(comanda, "LOAD %1023s", fisier) != 1) {
		comanda_invalida();
		return;
	}
	img = load_ppm(fisier);
	if (!img) {
		printf("Failed to load %s\n", fisier);
		return;
	}

	start_modificare(aplicatie);
	free_imagine(aplicatie->stare_curenta.imagine);
	aplicatie->stare_curenta.imagine = img;
	printf("Loaded %s (PPM image %dx%d)\n",
		   fisier, img->latime, img->inaltime);
	mesaj = my_malloc(sizeof(char *));
	mesaj[0] = my_strdup("Loaded image");
	final_modificare(aplicatie, mesaj, 1);
}

//executarea comenzii SAVE = salvea in format PPM a imaginii curente
void comanda_save(aplicatie_t *aplicatie, const char *comanda)
{
	char fisier[1000];
	if (sscanf(comanda, "SAVE %1023s", fisier) != 1) {
		comanda_invalida();
		return;
	}

	if (!aplicatie->stare_curenta.imagine) {
		printf("No image loaded\n");
		return;
	}

	if (!save_ppm(aplicatie->stare_curenta.imagine, fisier)) {
		printf("Failed to save %s\n", fisier);
		return;
	}
	printf("Saved %s\n", fisier);
}

//citeste axioma + regulile fisierului
void comanda_lsystem(aplicatie_t *aplicatie, const char *comanda)
{
	char fisier[1000];
	sistem_l_t *sys;
	char **mesaj;
	if (sscanf(comanda, "LSYSTEM %1023s", fisier) != 1) {
		comanda_invalida();
		return;
	}

	sys = incarca_sistem_l(fisier);
	if (!sys) {
		printf("Failed to load %s\n", fisier);
		return;
	}

	start_modificare(aplicatie);
	free_sistem_l(aplicatie->stare_curenta.sistem_l);
	aplicatie->stare_curenta.sistem_l = sys;
	printf("Loaded %s (L-system with %d rules)\n",
		   fisier, sys->numar_reguli);
	mesaj = my_malloc(sizeof(char *));
	mesaj[0] = my_strdup("Loaded L-system");
	final_modificare(aplicatie, mesaj, 1);
}

//calcularea si afisarea sirului dupa n derivari
void comanda_derive(aplicatie_t *aplicatie, const char *comanda)
{
	int n = 0;
	char *sir = NULL;
	if (sscanf(comanda, "DERIVE %d", &n) != 1) {
		comanda_invalida();
		return;
	}

	if (!aplicatie->stare_curenta.sistem_l) {
		printf("No L-system loaded\n");
		return;
	}

	sir = derive_sistem_l(aplicatie->stare_curenta.sistem_l, n);
	if (!sir) {
		return;
	}

	printf("%s\n", sir);
	free(sir);
}

//centralizatorul aplicatiei, apeleaza comanda introdusa
void executa_comanda(aplicatie_t *context, const char *linie)
{
	char comanda[100];
	if (sscanf(linie, "%s", comanda) != 1) {
		return;
	}

	if (strcmp(comanda, "LOAD") == 0) {
		comanda_load(context, linie);
	} else if (strcmp(comanda, "SAVE") == 0) {
		comanda_save(context, linie);
	} else if (strcmp(comanda, "LSYSTEM") == 0) {
		comanda_lsystem(context, linie);
	} else if (strcmp(comanda, "DERIVE") == 0) {
		comanda_derive(context, linie);
	} else if (strcmp(comanda, "TURTLE") == 0) {
		comanda_turtle(context, linie);
	} else if (strcmp(comanda, "FONT") == 0) {
		comanda_font(context, linie);
	} else if (strcmp(comanda, "TYPE") == 0) {
		comanda_type(context, linie);
	} else if (strcmp(comanda, "BITCHECK") == 0) {
		comanda_bitcheck(context);
	} else if (strcmp(comanda, "UNDO") == 0) {
		functie_undo(context);
	} else if (strcmp(comanda, "REDO") == 0) {
		functie_redo(context);
	} else if (strcmp(comanda, "EXIT") == 0) {
		/* in main este tratat acest caz */
	} else {
		comanda_invalida();
	}
}
