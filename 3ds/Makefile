#---------------------------------------------------------------------------------
# 3DS port of the Animal Crossing GameCube port.
#
# This is a SCAFFOLD Makefile based on the standard devkitARM 3DS template.
# In its current form it builds a hello-world that boots on a 3DS, initializes
# libctru + citro3d, clears both screens, polls input, and exits cleanly on
# START. None of the decomp game code is wired in yet.
#
# Requirements:
#   - devkitPro / devkitARM    (https://devkitpro.org/wiki/Getting_Started)
#   - libctru, citro3d, citro2d (installed via `pacman -S 3ds-dev`)
#
# Environment vars expected:
#   DEVKITPRO  (e.g. /opt/devkitpro)
#   DEVKITARM  (e.g. /opt/devkitpro/devkitARM)
#
# Build:
#   make                # produces AnimalCrossing.3dsx (Homebrew Launcher)
#   make AnimalCrossing.cia   (optional, requires makerom)
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITARM)),)
$(error "DEVKITARM is not set. Install devkitPro and source /etc/profile.d/devkit-env.sh")
endif

TOPDIR ?= $(CURDIR)
include $(DEVKITARM)/3ds_rules

#---------------------------------------------------------------------------------
# Target / sources / output
#---------------------------------------------------------------------------------
TARGET      := AnimalCrossing
BUILD       := build
SOURCES     := src
DATA        := data
INCLUDES    := include ../include ../src
ROMFS       := romfs

#---------------------------------------------------------------------------------
# Decomp wiring (opt-in).
#
# `make WIRE_DECOMP=1` pulls the decomp's `src/main.c` and `src/static/boot.c`
# into the build and renames their `main()` functions to `ac_entry` and
# `boot_main` respectively, the same trick `pc/CMakeLists.txt` uses.
#
# This WILL fail (compile errors first, then link errors) until all the
# transitively-included subsystems are stubbed or ported — boot.c alone pulls
# in OS, JSystem, JUT, GX, DVD, sound, padmgr, etc. The flag exists so the
# wiring is in place for the M1/M2 work; default build stays as the scaffold
# hello-world.
#---------------------------------------------------------------------------------
WIRE_DECOMP ?= 0

ifeq ($(WIRE_DECOMP),1)
    DECOMP_DIRS   := ../src ../src/static
    DECOMP_CFILES := main.c boot.c
    CFLAGS_EXTRA  := -DN3DS_WIRE_DECOMP \
                     -DVERSION=0 -DF3DEX_GBI_2 -DNDEBUG -DBUGFIXES -D_LANGUAGE_C
else
    DECOMP_DIRS   :=
    DECOMP_CFILES :=
    CFLAGS_EXTRA  :=
endif

APP_TITLE      := Animal Crossing
APP_DESCRIPTION:= 3DS port scaffold
APP_AUTHOR     := ACGC-3ds-port
ICON           := $(DEVKITPRO)/libctru/default_icon.png

#---------------------------------------------------------------------------------
# Compiler flags
#---------------------------------------------------------------------------------
ARCH    := -march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft

# -O0 mirrors the PC port: the decomp code has undefined behavior that breaks
# under any optimization level. Re-evaluate per subsystem when wiring real code.
CFLAGS  := -g -Wall -O0 -mword-relocations \
           -ffunction-sections \
           $(ARCH)

CFLAGS  += $(INCLUDE) -D__3DS__ -DTARGET_3DS $(CFLAGS_EXTRA)

CXXFLAGS:= $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++17
CFLAGS  += -std=gnu11

ASFLAGS := -g $(ARCH)
LDFLAGS := -specs=3dsx.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map)

LIBS    := -lcitro3d -lctru -lm
LIBDIRS := $(CTRULIB)

#---------------------------------------------------------------------------------
# devkitARM template plumbing (unchanged from the standard 3ds template)
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT   := $(CURDIR)/$(TARGET)
export TOPDIR   := $(CURDIR)

export VPATH    := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
                   $(foreach dir,$(DATA),$(CURDIR)/$(dir)) \
                   $(foreach dir,$(DECOMP_DIRS),$(CURDIR)/$(dir))

export DEPSDIR  := $(CURDIR)/$(BUILD)

CFILES   := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c))) \
            $(DECOMP_CFILES)
CPPFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES   := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
PICAFILES:= $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.v.pica)))
SHLISTFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.shlist)))
BINFILES := $(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

ifeq ($(strip $(CPPFILES)),)
    export LD := $(CC)
else
    export LD := $(CXX)
endif

export OFILES_BIN    := $(addsuffix .o,$(BINFILES))
export OFILES_SOURCES := $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
export OFILES_PICA   := $(PICAFILES:.v.pica=.shbin.o) $(SHLISTFILES:.shlist=.shbin.o)
export OFILES        := $(OFILES_BIN) $(OFILES_SOURCES) $(OFILES_PICA)
export HFILES_PICA   := $(addsuffix .h,$(subst .shbin.o,_shbin,$(OFILES_PICA)))

export INCLUDE  := $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
                   $(foreach dir,$(LIBDIRS),-I$(dir)/include) \
                   -I$(CURDIR)/$(BUILD)

export LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib)

export _3DSXDEPS := $(if $(NO_SMDH),,$(OUTPUT).smdh)

ifeq ($(strip $(ICON)),)
    icons := $(wildcard *.png)
    ifneq (,$(findstring $(TARGET).png,$(icons)))
        export APP_ICON := $(TOPDIR)/$(TARGET).png
    else
        ifneq (,$(findstring icon.png,$(icons)))
            export APP_ICON := $(TOPDIR)/icon.png
        endif
    endif
else
    export APP_ICON := $(ICON)
endif

ifeq ($(strip $(NO_SMDH)),)
    export _3DSXFLAGS += --smdh=$(CURDIR)/$(TARGET).smdh
endif

ifneq ($(ROMFS),)
    export _3DSXFLAGS += --romfs=$(CURDIR)/$(ROMFS)
endif

.PHONY: $(BUILD) clean all

all: $(BUILD)

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	@echo clean ...
	@rm -fr $(BUILD) $(TARGET).3dsx $(OUTPUT).smdh $(TARGET).elf

#---------------------------------------------------------------------------------
else

DEPENDS := $(OFILES:.o=.d)

# --- decomp main() rename trick (mirrors pc/CMakeLists.txt:342-352) ---
# src/main.c        : `void main(void)` -> ac_entry  (sets HotStartEntry)
# src/static/boot.c : `int main(int,const char**)` -> boot_main  (HotStartEntry loop)
# Per-target CFLAGS += only applies to these two objects.
main.o: CFLAGS += -Dmain=ac_entry
boot.o: CFLAGS += -Dmain=boot_main

$(OUTPUT).3dsx  : $(OUTPUT).elf $(_3DSXDEPS)
# bin2o emits both `.shbin.o` and `_shbin.h` from one recipe, but some
# devkitPro versions don't expose a pattern rule for the header on its
# own. Depend on the .o instead -- the .h is produced as a side effect of
# building it, so by the time any source compiles, the header is on disk.
$(OFILES_SOURCES) : $(OFILES_PICA)
$(OUTPUT).elf   : $(OFILES)

-include $(DEPENDS)

endif
