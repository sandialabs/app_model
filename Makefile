#
## $Id: Makefile,v 1.15 2010/03/03 02:27:01 rolf Exp $
## Makefile to build the application checkpoint/restart model
#
.PHONY.:	all clean realclean tags

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

INCLUDES =	-ISearch

DEPS =	app phases report rMPI_model rnd data_structs \
	globals timing input

all:	two_step


#
## Dependencies
#
two_step:	$(addsuffix .o, $(DEPS)) main.o
main.o:		globals.h app.h report.h rnd.h
app.o:		globals.h app.h phases.h rMPI_model.h
phases.o:	globals.h phases.h
report.o:	globals.h report.h
rMPI_model.o:	globals.h rMPI_model.h rnd.h data_structs.h
rnd.o:		globals.h rnd.h
data_structs.o:		data_structs.h
globals.o:	globals.h
timing.o:	globals.h timing.h
input.o:	input.h


#
## Build it
#
%.o:	%.c
	gcc $(MYFLAGS) $(INCLUDES) $(WARN) $< -c

two_step: Search/avl.o
	gcc $(MYFLAGS) $(WARN) $(addsuffix .o, $(DEPS)) main.o -o $@ -lgsl -lgslcblas -lm $< -lrt

Search/avl.o:
	$(MAKE) -C Search

tags:	$(addsuffix .c, $(DEPS)) main.c
	ctags $(addsuffix .c, $(DEPS)) Search/avl.c main.c

dist:
	cd .. ; \
	tar -zcf app_model/app_model_v1_0.tar.gz \
	    $(addprefix app_model/, $(addsuffix .c, $(DEPS))) \
	    $(addprefix app_model/, $(addsuffix .h, $(DEPS))) \
	    app_model/main.c \
	    app_model/README \
	    app_model/LICENSE \
	    app_model/Makefile \
	    app_model/Search/avl.c \
	    app_model/Search/avl.h \
	    app_model/Search/Makefile \
	    app_model/Search/README


#
## Clean up
#
clean:
	$(MAKE) -C Search $@
	@rm -f $(addsuffix .o, $(DEPS)) main.o
	@rm -f gmon.out

realclean:	clean
	$(MAKE) -C Search $@
	@rm -f two_step
	@rm -f tags
	@rm -f app_model_v1_0.tar.gz
