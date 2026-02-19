/* 314CA Stefan Alexandru Vladut*/
#ifndef ISTORIC_H
#define ISTORIC_H

struct aplicatie_t;
typedef struct modificare_t modificare_t;

typedef struct stiva_istoric_t {
	modificare_t **elemente;
	int nr;
	int size;
} stiva_istoric_t;

stiva_istoric_t *init_stiva_istoric(void);
void free_stiva_istoric(stiva_istoric_t *stiva);

void start_modificare(struct aplicatie_t *aplicatie);
void final_modificare(struct aplicatie_t *aplicatie,
					  char **mesaj,
					  int nr_linii);

int functie_undo(struct aplicatie_t *aplicatie);
int functie_redo(struct aplicatie_t *aplicatie);
#endif
