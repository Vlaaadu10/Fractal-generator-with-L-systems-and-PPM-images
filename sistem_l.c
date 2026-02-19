/* 314CA Stefan Alexandru Vladut*/
#include "sistem_l.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void init_sistem(sistem_l_t *s)
{
	int i;
	s->axioma = NULL;
	for (i = 0; i < 256; i++) {
		s->reguli[i] = NULL;
	}
}

void free_reguli(sistem_l_t *s)
{
	int i;
	for (i = 0; i < 256; i++) {
		free(s->reguli[i]);
		s->reguli[i] = NULL;
	}
}

//construirea unui L-system pe baza axiomei si a setului de reguli
sistem_l_t *incarca_sistem_l(char *nume_fisier)
{
	FILE *f;
	sistem_l_t *s;
	char comanda[256];
	int numar_reguli;
	int i;

	f = fopen(nume_fisier, "r");
	if (!f) {
		return NULL;
	}
	s = my_malloc(sizeof(*s));
	init_sistem(s);
	//extrage de pe prima linie axioma
	if (!fgets(comanda, (int)sizeof(comanda), f)) {
		free_sistem_l(s);
		fclose(f);
		return NULL;
	}
	del_newline(comanda);
	s->axioma = my_strdup(comanda);
	//extrage de pe a doua linie numarul de reguli
	if (!fgets(comanda, (int)sizeof(comanda), f)) {
		free_sistem_l(s);
		fclose(f);
		return NULL;
	}
	del_newline(comanda);
	numar_reguli = atoi(comanda);
	s->numar_reguli = numar_reguli;
	//cele numar_reguli linii care contin regulile
	for (i = 0; i < numar_reguli; i++) {
		char st;//simbolul din partea stanga
		char dr[256];//sirul din partea dreapta
		if (!fgets(comanda, sizeof(comanda), f)) {
			free_sistem_l(s);
			fclose(f);
			return NULL;
		}
		del_newline(comanda);
		if (sscanf(comanda, " %c %s", &st, dr) != 2) {
			free_sistem_l(s);
			fclose(f);
			return NULL;
		}
		char *copie = my_strdup(dr);
		if (!copie) {
			free_sistem_l(s);
			fclose(f);
			return NULL;
		}
		if (s->reguli[(unsigned char)st]) {
			free(s->reguli[(unsigned char)st]);
		}
		s->reguli[(unsigned char)st] = copie;
	}
	fclose(f);
	return s;
}

void free_sistem_l(sistem_l_t *s)
{
	if (!s) {
		return;
	}
	free(s->axioma);
	free_reguli(s);
	free(s);
}

//creare deep-copy a unui L-system pentru a nu afecat starile vechi
sistem_l_t *copiaza_sistem_l(sistem_l_t *s)
{
	sistem_l_t *d;
	int i;
	if (!s) {
		return NULL;
	}
	d = my_malloc(sizeof(*d));
	init_sistem(d);
	d->axioma = my_strdup(s->axioma);
	d->numar_reguli = s->numar_reguli;
	for (i = 0; i < 256; i++) {
		if (s->reguli[i]) {
			d->reguli[i] = my_strdup(s->reguli[i]);
		}
	}
	return d;
}

//calculeaza lungimea sirului dupa derivare
static int len_derivata(const sistem_l_t *s, const char *sir)
{
	int total;
	int i;
	total = 0;
	for (i = 0; sir[i] != '\0'; i++) {
		const char *rez = s->reguli[(unsigned char)sir[i]];
		if (rez) {
			total += (int)strlen(rez);
		} else {
			total += 1;
		}
	}
	return total;
}

//aplica regulile de derivare pentru o singura data
static char *transforma_sir(const sistem_l_t *s, const char *sir)
{
	int i;
	int poz;
	int n;
	char *mesaj;
	n = len_derivata(s, sir);
	mesaj = my_malloc(n + 1);
	poz = 0;
	for (i = 0; sir[i] != '\0'; i++) {
		const char *rez = s->reguli[(unsigned char)sir[i]];
		if (rez) {
			int k = strlen(rez);
			memcpy(mesaj + poz, rez, (size_t)k);
			poz += k;
		} else {
			mesaj[poz++] = sir[i];
		}
	}
	mesaj[poz] = '\0';
	return mesaj;
}

//generarea sirului dupa derivare de n ori
char *derive_sistem_l(sistem_l_t *s, int n)
{
	int i;
	char *curent = NULL;
	char *urmator = NULL;

	curent = my_strdup(s->axioma);
	if (!curent) {
		return NULL;
	}

	for (i = 0; i < n; i++) {
		urmator = transforma_sir(s, curent);
		free(curent);
		curent = urmator;
	}
	return curent;
}
