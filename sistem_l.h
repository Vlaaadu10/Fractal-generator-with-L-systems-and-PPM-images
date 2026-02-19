/* 314CA Stefan Alexandru Vladut*/
#ifndef SISTEM_L_H
#define SISTEM_L_H

typedef struct sistem_l_t {
	char *axioma;
	char *reguli[256];
	int numar_reguli;
} sistem_l_t;

sistem_l_t *incarca_sistem_l(char *nume_fisier);
void free_sistem_l(sistem_l_t *sistem);

sistem_l_t *copiaza_sistem_l(sistem_l_t *sistem);

char *derive_sistem_l(sistem_l_t *sistem, int n);
#endif
