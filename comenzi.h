/* 314CA Stefan Alexandru Vladut*/
#ifndef COMENZI_H
#define COMENZI_H

#include "turtle.h"
#include "sistem_l.h"
#include "font.h"
#include "istoric.h"

//starea aplicatiei la un anumit moment de timp
typedef struct stare_aplicatie_t {
	imagine_t *imagine;
	sistem_l_t *sistem_l;
	font_t *font;
} stare_aplicatie_t;

//starea aplicatiei in sine + UNDO/REDO
typedef struct aplicatie_t {
	stare_aplicatie_t stare_curenta;
	stiva_istoric_t *undo;
	stiva_istoric_t *redo;
} aplicatie_t;

//deep copy pentru o stare
stare_aplicatie_t *copy_stare(stare_aplicatie_t *stare);

void free_stare(stare_aplicatie_t *stare);
//revenirea la o stare anterioara
void reset_stare(aplicatie_t *context,
				 stare_aplicatie_t *stare);
//setarea starii curente la NULL
void init_aplicatie(aplicatie_t *context);
void free_aplicatie(aplicatie_t *context);

//apelarea functiei aferente comenzii respective
void executa_comanda(aplicatie_t *context,
					 const char *linie);

#endif
