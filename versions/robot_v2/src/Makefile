#
# Hello Robot C - Makefile des sources du projet.
#
# @author Matthias Brun
#

#
# Organisation des sources.
#

# Packages du projet (à compléter si besoin est).
PACKAGES = commando telco

# Un niveau de package est accessible.
SRC  = $(wildcard */*.c)

# Pour ajouter un second niveau :		
# SRC += $(wildcard */*/*.c)

OBJ = $(SRC:.c=.o)

# Point d'entrée du programme.
MAIN_SERVER = main_server.c
MAIN_CLIENT = main_client.c

# Gestion automatique des dépendances.
DEP_SERVER = $(MAIN_SERVER:.c=.d)
DEP_CLIENT = $(MAIN_CLIENT:.c=.d)

# Exécutable à générer.
EXEC_SERVER = ../$(PROG_SERVER)
EXEC_CLIENT = ../$(PROG_CLIENT)


# Inclusion depuis le niveau du package.
CCFLAGS += -I.

#
# Règles du Makefile.
#

# Compilation.
all:
	@for p in $(PACKAGES); do (cd $$p; $(MAKE) $@); done
	@$(MAKE) CCFLAGS="$(CCFLAGS)" LDFLAGS="$(LDFLAGS)" $(EXEC_SERVER)
	@$(MAKE) CCFLAGS="$(CCFLAGS)" LDFLAGS="$(LDFLAGS)" $(EXEC_CLIENT)
	
$(EXEC_SERVER): $(OBJ) $(MAIN_SERVER)
	$(CC) $(CCFLAGS) $(OBJ) $(MAIN_SERVER) -MF $(DEP_SERVER) -o $(EXEC_SERVER) $(LDFLAGS)

$(EXEC_CLIENT): $(OBJ) $(MAIN_CLIENT)
	$(CC) $(CCFLAGS) $(OBJ) $(MAIN_CLIENT) -MF $(DEP_CLIENT) -o $(EXEC_CLIENT) $(LDFLAGS)


# Nettoyage.
.PHONY: clean

clean:
	@for p in $(PACKAGES); do (cd $$p; $(MAKE) $@); done
	@rm -f $(DEP_SERVER)
	@rm -f $(DEP_CLIENT)


-include $(DEP_SERVER)
-include $(DEP_CLIENT)