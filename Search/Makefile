#
## $Id: Makefile,v 1.1 2010/01/14 22:58:09 rolf Exp $
## Build the avl search tree code
#
.PHONY.:	all clean realclean

MYFLAGS = -pg -g
MYFLAGS = 

WARN = -Wall -pedantic \
        -Wswitch-default \
	-Wswitch-enum \
	-Wunused-parameter \
	-Wextra \
	-Wfloat-equal \
	-Wshadow \
	-Wunreachable-code \
	-Wredundant-decls 

DEPS =	avl

all:	avl.o


#
## Dependencies
#
avl.o:		avl.h


#
## Build it
#
%.o:	%.c
	gcc $(MYFLAGS) $(WARN) $< -c


#
## Clean up
#
clean:
	@rm -f $(addsuffix .o, $(DEPS))

realclean:	clean
