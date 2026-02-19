# Copyright PCLP Team, 2025

# Compiler setup.
CC=gcc
CFLAGS=-Wall -Wextra -std=c99

# Define targets, e.g., ninel, codeinvim, vectsecv, nomogram.
TARGETS=runic

SOURCES=main.c sistem_l.c turtle.c font.c istoric.c comenzi.c

# Manually define all targets.
build: $(TARGETS)

runic: $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o runic -lm

# Pack the solution into a zip file.
pack:
	zip -FSr 314CA_StefanAlexandruVladut_Tema3.zip README Makefile *.c *.h

# Clean the solution.
clean:
	rm -f $(TARGETS)

.PHONY: build pack clean
