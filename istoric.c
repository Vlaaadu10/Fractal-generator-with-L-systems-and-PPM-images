/* 314CA Stefan Alexandru Vladut*/
#include "istoric.h"
#include <stdio.h>
#include <stdlib.h>
#include "comenzi.h"
#include <string.h>

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

struct modificare_t {
	stare_aplicatie_t *inainte;
	stare_aplicatie_t *dupa;
	char **output;
	int numar_linii;
};

//creaza si initializeaza o stiva goala pentru istoric
stiva_istoric_t *init_stiva_istoric(void)
{
	stiva_istoric_t *stiva;
	stiva = my_malloc(sizeof(*stiva));
	stiva->elemente = NULL;
	stiva->nr = 0;
	stiva->size = 0;
	return stiva;
}

void free_modificare(modificare_t *aplicatie)
{
	int i;
	free_stare(aplicatie->inainte);
	free_stare(aplicatie->dupa);
	for (i = 0; i < aplicatie->numar_linii; i++) {
		free(aplicatie->output[i]);
	}
	free(aplicatie->output);
	free(aplicatie);
}

void free_stiva_istoric(stiva_istoric_t *stiva)
{
	while (stiva->nr > 0) {
		stiva->nr--;
		free_modificare(stiva->elemente[stiva->nr]);
	}
	free(stiva->elemente);
	free(stiva);
}

static void stiva_push(stiva_istoric_t *stiva, modificare_t *aplicatie)
{
	if (stiva->nr == stiva->size) {
		int cap;
		if (stiva->size == 0) {
			cap = 8;
		} else {
			cap = stiva->size * 2;
		}
		stiva->elemente =
			my_realloc(stiva->elemente, (size_t)cap * sizeof(modificare_t *));
		stiva->size = cap;
	}
	stiva->elemente[stiva->nr++] = aplicatie;
}

static modificare_t *stiva_pop(stiva_istoric_t *stiva)
{
	if (stiva->nr == 0) {
		return NULL;
	}
	stiva->nr--;
	return stiva->elemente[stiva->nr];
}

//inceperea unei noi modificari a aplicatiei
void start_modificare(aplicatie_t *aplicatie)
{
	modificare_t *modificare;
	modificare = my_malloc(sizeof(*modificare));
	modificare->inainte = copy_stare(&aplicatie->stare_curenta);
	modificare->dupa = NULL;
	modificare->output = NULL;
	modificare->numar_linii = 0;

	stiva_push(aplicatie->undo, modificare);
	while (aplicatie->redo->nr > 0) {
		free_modificare(stiva_pop(aplicatie->redo));
	}
}

//finalizarea unei modificari si salvarea starii curente ca fiind cea de "dupa"
void final_modificare(aplicatie_t *aplicatie, char **mesaj,
					  int nr_linii)
{
	modificare_t *modificare;
	modificare = aplicatie->undo->elemente[aplicatie->undo->nr - 1];
	modificare->dupa = copy_stare(&aplicatie->stare_curenta);
	modificare->output = mesaj;
	modificare->numar_linii = nr_linii;
}

//anuleaza ultima modificare si revine la starea anterioara
int functie_undo(aplicatie_t *aplicatie)
{
	modificare_t *modificare;
	modificare = stiva_pop(aplicatie->undo);
	if (!modificare) {
		printf("Nothing to undo\n");
		return 0;
	}
	stiva_push(aplicatie->redo, modificare);
	reset_stare(aplicatie, modificare->inainte);
	return 1;
}

//reface ultima modificare anulata si afiseaza mesajele aferente
int functie_redo(aplicatie_t *aplicatie)
{
	int i;
	modificare_t *modificare;
	modificare = stiva_pop(aplicatie->redo);
	if (!modificare) {
		printf("Nothing to redo\n");
		return 0;
	}
	reset_stare(aplicatie, modificare->dupa);
	for (i = 0; i < modificare->numar_linii; i++) {
		printf("%s\n", modificare->output[i]);
	}
	stiva_push(aplicatie->undo, modificare);
	return 1;
}
