#
# Robot v1 C - Makefile principal.
# 
# @author Matthias Brun, adapté par Jérôme Delatour
#

# Répertoire d'installation de la librairie Infox
# pour la compilation avec le simulateur Intox 
# TODO (à adapter) :
# INTOXDIR = $(realpath /<path>/<to>/infox_prose-<arch>-v0.3
# Exemple :
# Attention : pas d'espace ou d'accent dans votre chemin
# sinon passer par realpath ne fonctionne pas

#CHEMIN ABSOLU
#INTOXDIR = $(realpath /home/ludovic/Documents/ESEO/I2/S8/projet_robot/infox_prose-x86_64-v0.3)

#CHEMIN RELATIF (librairie dans le répertoire parent du projet)
INTOXDIR = $(realpath ../infox_prose-x86_64-v0.3)

# Organisation des sources.
#

export SRCDIR = src
export BINDIR = bin

SUBDIRS = $(SRCDIR)

#
# Définitions des outils.
#

# Compilateur.

export CC = gcc

# options de compilation
export CCFLAGS += -O0 
# avec debuggage : -g -DDEBUG
# sans debuggage : -DNDEBUG
export CCFLAGS += -DNDEBUG
 # gestion automatique des dépendances
export CCFLAGS += -MMD -MP
export CCFLAGS += -D_BSD_SOURCE -D_XOPEN_SOURCE_EXTENDED -D_XOPEN_SOURCE -D_DEFAULT_SOURCE -D_GNU_SOURCE
export CCFLAGS += -D_REENTRANT -D_POSIX_C_SOURCE=199309L
#On compile en utilisant les options de gcc
export CCFLAGS += -std=c99 -Wall
# -pedantic retiré car génère des warnings pour mes TRACE
export LDFLAGS += -lrt -pthread

# options de compilation pour l'utilisation de Intox/Infox
export CCFLAGS += -DINTOX
export CCFLAGS += -I$(INTOXDIR)/include/infox/prose/
export LDFLAGS += -L$(INTOXDIR)/lib/ -linfox

#
# Définitions du binaire à générer.
#

#export PROG = $(BINDIR)/robot_pc
export PROG_SERVER = $(BINDIR)/server
export PROG_CLIENT = $(BINDIR)/client


#
# Règles du Makefile.
#

# Compilation récursive.
all: 
	@for i in $(SUBDIRS); do (cd $$i; make $@); done

# Nettoyage.
.PHONY: clean

clean:
	@for i in $(SUBDIRS); do (cd $$i; make $@); done
	@rm -f $(PROG_SERVER) core* $(BINDIR)/core*
	@rm -f $(PROG_CLIENT) core* $(BINDIR)/core*

