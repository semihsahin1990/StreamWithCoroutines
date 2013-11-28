# Make Target:
# ------------
# The Makefile provides the following targets to make:
#   $ make           compile and link
#   $ make objs      compile only (no linking)
#   $ make clean     clean objects and the executable file
#   $ make help      get the usage of the makefile

## Customizable Section: adapt those variables to suit your program.

OBJDIR  = obj
SRCDIR  = src
BINDIR  = bin

PROGRAM   = main

SRCDIRS   = streamc \
	    streamc/runtime \
	    streamc/operators 
CPPFLAGS  = -Wall -std=c++11 -stdlib=libc++
INCLUDES  = -I/opt/local/include -I/opt/boost/include
LDFLAGS   = -L/opt/local/lib -Wl,-rpath,/opt/local/lib -L/opt/boost/lib -Wl,-rpath,/opt/boost/lib -lboost_coroutine-mt -lboost_context-mt -lboost_system-mt 

## Implicit Section: change the following only when necessary.

CFLAGS   = -g -O2 -I $(SRCDIR) $(INCLUDES)
CXXFLAGS = -g -O2 -I $(SRCDIR) $(INCLUDES)
CC       = clang
CXX      = clang++

## Stable Section: usually no need to be changed. 

SHELL   = /bin/sh
SOURCES = $(wildcard $(addsuffix /*.cpp,$(addprefix $(SRCDIR)/,$(SRCDIRS))))
HEADERS = $(wildcard $(addsuffix /*.h,$(addprefix $(SRCDIR)/,$(SRCDIRS))))
OBJS    = $(patsubst $(SRCDIR)/%,$(OBJDIR)/%,$(addsuffix .o, $(basename $(SOURCES))))
DEPS    = $(OBJS:.o=.d)

## Define some useful variables.
DEP_OPT = $(shell if `$(CC) --version | grep "GCC" >/dev/null`; then \
                  echo "-MM -MP"; else echo "-M"; fi )
DEPEND  = $(CC)  $(DEP_OPT) $(CFLAGS) $(CPPFLAGS)
COMPILE = $(CXX) $(CXXFLAGS) $(CPPFLAGS) -c
LINK    = $(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS)

.PHONY: all objs clean distclean help show

# Delete the default suffixes
.SUFFIXES:

all: $(BINDIR)/$(PROGRAM)

# Rules for creating dependency files (.d).

$(OBJDIR)/%.d:$(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	@/bin/echo -n $(dir $@) > $@
	@$(DEPEND) $< >> $@ || rm $@

# Rules for generating object files (.o).
objs:$(OBJS)

$(OBJDIR)/%.o:$(SRCDIR)/%.cpp
	$(COMPILE) $< -o $@

# Rules for generating the executable.
$(BINDIR)/$(PROGRAM):$(OBJS)
	@mkdir -p $(dir $@)
	$(LINK) $(OBJS) -o $@

ifneq ($(DEPS),)
  sinclude $(DEPS)
endif

clean:
	$(RM) -fr $(OBJDIR) $(BINDIR)


# Show help.
help:
	@echo 'Usage: make [TARGET]'
	@echo 'TARGETS:'
	@echo '  all       (=make) compile and link.'
	@echo '  objs      compile only (no linking).'
	@echo '  tags      create tags for Emacs editor.'
	@echo '  clean     clean objects and the executable file.'
	@echo '  show      show variables (for debug use only).'
	@echo '  help      print this message.'

# Show variables (for debug use only.)
show:
	@echo 'PROGRAM     :' $(PROGRAM)
	@echo 'SRCDIRS     :' $(SRCDIRS)
	@echo 'HEADERS     :' $(HEADERS)
	@echo 'SOURCES     :' $(SOURCES)
	@echo 'OBJS        :' $(OBJS)
	@echo 'DEPS        :' $(DEPS)
	@echo 'DEPEND      :' $(DEPEND)
	@echo 'COMPILE     :' $(COMPILE)
	@echo 'link        :' $(LINK)



