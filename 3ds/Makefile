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
INCLUDES    := include ../include
ROMFS       := romfs

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

CFLAGS  += $(INCLUDE) -D__3DS__ -DTARGET_3DS

# Match the PC port's defines for any shared code we pull in later.
# Commented out until we actually start linking decomp code.
# CFLAGS += -DVERSION=0 -DF3DEX_GBI_2 -DNDEBUG -DBUGFIXES -D_LANGUAGE_C

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
                   $(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR  := $(CURDIR)/$(BUILD)

CFILES   := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
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
export HFILES_PICA   := $(addsuffix .h,$(subst .shbin.o,.shbin,$(OFILES_PICA)))

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

$(OUTPUT).3dsx  : $(OUTPUT).elf $(_3DSXDEPS)
$(OFILES_SOURCES) : $(HFILES_PICA)
$(OUTPUT).elf   : $(OFILES)

-include $(DEPENDS)

endif
