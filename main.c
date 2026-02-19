/* 314CA Stefan Alexandru Vladut*/
#include <stdio.h>
#include <string.h>

#include "comenzi.h"
#define MAX_COMENZI 300000

//functie care elimina \n si \r de la final
static void del_newline(char *comanda)
{
	int n;
	n = strlen(comanda);
	while (n > 0 && (comanda[n - 1] == '\n' || comanda[n - 1] == '\r')) {
		comanda[n - 1] = '\0';
		n--;
	}
}

int main(void)
{
	char comanda[10000];
	aplicatie_t app;
	int cnt = 0;
	//initializarea aplicatiei
	init_aplicatie(&app);
	//citirea comenzilor pana cand se atinge un nr maxim, EOF sau EXIT
	while (cnt < MAX_COMENZI &&
		   fgets(comanda, (int)sizeof(comanda), stdin)) {
		//cazul in care comanda e prea mare pentru buffer
		if (!strchr(comanda, '\n')) {
			int i;
			while ((i = getchar()) != '\n' && i != EOF) {
				/* ignoram*/
			}
		}

		del_newline(comanda);

		if (comanda[0] == '\0') {
			cnt++;
			continue;
		}

		if (strcmp(comanda, "EXIT") == 0) {
			break;
		}

		executa_comanda(&app, comanda);
		cnt++;
	}
	free_aplicatie(&app);
	return 0;
}
