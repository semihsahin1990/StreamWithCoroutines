# Make Target:
# ------------
# The Makefile provides the following targets to make:
#   $ make           compile and link
#   $ make objs      compile only (no linking)
#   $ make docs     build documentation (requires doxygen)
#   $ make clean     clean objects and the executable file
#   $ make help      get the usage of the makefile

## Customizable Section: adapt those variables to suit your program.

OBJDIR  = obj
BINDIR  = bin
SRCDIR  = src
INCDIR  = include

LIBRARY   = libstreamc.so

CPPFLAGS  = -DBOOST_LOG_DYN_LINK
INCLUDES  = -isystem /opt/local/libcpp-software/include
LDFLAGS   = -L /opt/local/libcpp-software/lib -Wl,-rpath,/opt/local/libcpp-software/lib \
	-lboost_coroutine-mt -lboost_context-mt -lboost_log-mt -lboost_log_setup-mt \
	-lboost_filesystem-mt  -lboost_chrono-mt -lboost_system-mt -lboost_thread-mt 

## Implicit Section: change the following only when necessary.
CXX      = clang++ 
CXXFLAGS = -g -O2 -fPIC -Wall -std=c++11 -stdlib=libc++ 
SRCDIRS  = $(shell find $(SRCDIR) -type d)
INCDIRS  = $(shell find $(INCDIR) -type d)
INCFLAGS = $(addprefix -I ,$(SRCDIRS)) $(addprefix -I ,$(INCDIRS)) $(INCLUDES)

## Stable Section: usually no need to be changed. 
SHELL   = /bin/sh
SOURCES = $(wildcard $(addsuffix /*.cpp,$(SRCDIRS)))
OBJS    = $(patsubst $(SRCDIR)/%,$(OBJDIR)/%,$(SOURCES:.cpp=.o))
DEPS    = $(OBJS:.o=.d)

## Define some useful variables.
CDP	= clang++
DEP_OPT = -MM -MP
DEPEND  = $(CDP) $(CPPFLAGS) $(CXXFLAGS) $(INCFLAGS) $(DEP_OPT)
COMPILE = $(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INCFLAGS) 
LINK    = $(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS)

.PHONY: all objs clean distclean help show

# Delete the default suffixes
.SUFFIXES:

all: $(BINDIR)/$(LIBRARY)

$(OBJDIR)/%.d: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	@/bin/echo -n $(dir $@) > $@
	@$(DEPEND) $< >> $@ || rm $@

objs: $(OBJS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(COMPILE) -c $< -o $@

$(BINDIR)/$(LIBRARY): $(OBJS)
	@mkdir -p $(dir $@)
	$(LINK) $(OBJS) -shared -o "$(CURDIR)/$@"

docs:
	doxygen doc/doxy.conf

ifneq ($(DEPS),)
  sinclude $(DEPS)
endif

clean:
	$(RM) -r $(OBJDIR) $(BINDIR)
	$(RM) -r docs/html

help:
	@echo 'Usage: make [TARGET]'
	@echo 'TARGETS:'
	@echo '  all       (=make) compile and link.'
	@echo '  objs      compile only (no linking).'
	@echo '  tags      create tags for Emacs editor.'
	@echo '  clean     clean objects and the executable file.'
	@echo '  show      show variables (for debug use only).'
	@echo '  help      print this message.'

show:
	@echo 'LIBRARY     :' $(LIBRARY)
	@echo 'SRCDIRS     :' $(SRCDIRS)
	@echo 'INCDIRS     :' $(INCDIRS)
	@echo 'SOURCES     :' $(SOURCES)
	@echo 'OBJS        :' $(OBJS)
	@echo 'DEPS        :' $(DEPS)
	@echo 'DEPEND      :' $(DEPEND)
	@echo 'COMPILE     :' $(COMPILE)
	@echo 'link        :' $(LINK)



