# --- custom setup --------------------------------

#INCLUDES +=
LIBS += -lsndfile -lquadmath

PROGNAME = audio_histogram
SOURCES := $(wildcard *.cpp)

# --- generic setup --------------------------------

# debug
#CFLAGS += -g -O0 -fno-inline

# release
CFLAGS += -g -O2 -fvisibility-inlines-hidden
LFLAGS += -Wl,-O1 -Wl,-gc-sections

# common
CFLAGS += -m64 -pipe -fPIC
LDFLAGS += -m64

# C++11 support
#CPPFLAGS += -std=c++0x
#CPPFLAGS += -std=c++17
CPPFLAGS += -std=gnu++17

# warnings
CFLAGS += -Wall -Wextra -pedantic

# append CFLAGS to CPPFLAGS
CPPFLAGS += $(CFLAGS)

# --- execute --------------------------------

# $(call make-depend, source-file, object-file, depend-file)
define make-depend
$(CC) -MM -MF $(3) -MP -MT $(2) $(CPPFLAGS) $(1)
endef

.PHONY: all
all: $(PROGNAME)

$(PROGNAME): $(subst .cpp,.o,$(SOURCES))
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)
	objcopy --only-keep-debug $@ $@.sym
	objcopy --strip-debug --strip-unneeded $@
	objcopy --add-gnu-debuglink=$@.sym $@

%.o : %.cpp Makefile
	@$(call make-depend, $<, $@, $(subst .o,.d,$@))
	$(CC) -c $(CPPFLAGS) $(INCLUDES) -o $@ $<

-include $(subst .cpp,.d,$(SOURCES))

.PHONY: clean
clean:
	rm -f $(PROGNAME) $(PROGNAME).sym $(subst .cpp,.d,$(SOURCES)) $(subst .cpp,.o,$(SOURCES))
