#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

TOPDIR ?= $(CURDIR)
include $(DEVKITARM)/3ds_rules

TARGET		:= 	$(notdir $(CURDIR))
BUILD		:= 	Build
INCLUDES	:= 	Includes \
				Includes\ctrulib \
				Includes\ctrulib\allocator \
				Includes\ctrulib\gpu \
				Includes\ctrulib\services \
				Includes\ctrulib\util
SOURCES 	:= 	Sources \
				Sources\CTRPluginFramework \
				Sources\CTRPluginFramework\Graphics \
				Sources\CTRPluginFramework\Menu \
				Sources\CTRPluginFramework\System \
				Sources\CTRPluginFramework\Utils \
				Sources\CTRPluginFrameworkImpl \
				Sources\CTRPluginFrameworkImpl\Graphics \
				Sources\CTRPluginFrameworkImpl\Graphics\Icons \
				Sources\CTRPluginFrameworkImpl\Menu \
				Sources\CTRPluginFrameworkImpl\Search \
				Sources\CTRPluginFrameworkImpl\System \
				Sources\CTRPluginFrameworkImpl\ActionReplay \
				Sources\ctrulib \
				Sources\ctrulib\allocator \
				Sources\ctrulib\gpu \
				Sources\ctrulib\services \
				Sources\ctrulib\system \
				Sources\ctrulib\util\utf \
				Sources\ctrulib\util\rbtree

IP			:=  5
FTP_HOST 	:=	192.168.1.
FTP_PORT	:=	"5000"
FTP_PATH	:=	"0004000000033600/" #Zelda OOT

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ARCH	:=	-march=armv6k -mlittle-endian -mtune=mpcore -mfloat-abi=hard 

CFLAGS	:=	-g -Os -mword-relocations \
 			-fomit-frame-pointer -ffunction-sections -fno-strict-aliasing \
			$(ARCH)

CFLAGS		+=	$(INCLUDE) -DARM11 -D_3DS 

CXXFLAGS	:= $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++11

ASFLAGS		:= -g $(ARCH)
LDFLAGS		:= -pie -T $(TOPDIR)/3ds.ld $(ARCH) -Os -Wl,-Map,$(notdir $*.map),--gc-sections,--strip-discarded,--strip-debug

LIBS 		:= 	-lctru -lm
LIBDIRS		:= 	$(CTRULIB)

#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUT	:=	$(CURDIR)/$(TARGET)
export LIBOUT	:=  $(CURDIR)/lib$(TARGET).a
export TOPDIR	:=	$(CURDIR)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
					$(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

CFILES			:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES			:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))

export LD 		:= 	$(CXX)
export OFILES	:=	$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					-I$(CURDIR)/$(BUILD)

export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L $(dir)/lib)

.PHONY: $(BUILD) clean re all

#---------------------------------------------------------------------------------
all: $(BUILD)

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
clean:
	@echo clean ... 
	@rm -fr $(BUILD) $(TARGET).3dsx $(OUTPUT).smdh $(TARGET).elf

re: clean all
	
send:
	@echo "Sending the plugin over FTP"
	@$(TOPDIR)/sendfile.py $(TARGET).plg $(FTP_PATH) "$(FTP_HOST)$(IP)" $(FTP_PORT)

ACNL:
	make send FTP_PATH="0004000000086400/"
FL:
	make send FTP_PATH="0004000000113100/"

#---------------------------------------------------------------------------------

else

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------

DEPENDS	:=	$(OFILES:.o=.d)
EXCLUDE := main.o cheats.o ActionReplayTest.o OSDManager.o PointerTesting.o Speedometer.o


$(OUTPUT).plg : $(OFILES) $(LIBOUT)
$(LIBOUT):	$(filter-out $(EXCLUDE), $(OFILES))

#---------------------------------------------------------------------------------
# you need a rule like this for each extension you use as binary data
#---------------------------------------------------------------------------------
%.bin.o	:	%.bin
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)

#---------------------------------------------------------------------------------
%.plg: %.elf
#---------------------------------------------------------------------------------
	@echo creating $(notdir $@)
	@$(OBJCOPY) -O binary $(OUTPUT).elf $(TOPDIR)/$(notdir $@) -S
	@echo Done !


-include $(DEPENDS)

#---------------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------------

