.SUFFIXES:

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

TOPDIR ?= $(CURDIR)
include $(TOPDIR)/3ds_rules

LIB 		:=  CTRPluginFramework.a
TARGET		:= 	$(notdir $(CURDIR))
BUILD		:= 	Build
INCLUDES	:= 	Includes \
				Includes\ctrulib \
				Includes\ctrulib\allocator \
				Includes\ctrulib\gpu \
				Includes\ctrulib\services \
				Includes\ctrulib\util \
				Includes\libntrplg \
				Includes\libntrplg\ns \
				C:\devkitPro\devkitARM\arm-none-eabi\include\c++\5.3.0
LIBDIRS		:= 	$(TOPDIR)\Lib
SOURCES 	:= 	Sources \
				Sources\CTRPluginFramework \
				Sources\CTRPluginFramework\Graphics \
				Sources\CTRPluginFramework\Menu \
				Sources\CTRPluginFramework\System \
				Sources\CTRPluginFrameworkImpl \
				Sources\CTRPluginFrameworkImpl\Graphics \
				Sources\CTRPluginFrameworkImpl\Graphics\Icons \
				Sources\CTRPluginFrameworkImpl\Menu \
				Sources\CTRPluginFrameworkImpl\System \
				Sources\ctrulib \
				Sources\ctrulib\allocator \
				Sources\ctrulib\gpu \
				Sources\ctrulib\services \
				Sources\ctrulib\system \
				Sources\ctrulib\util\utf \
				Sources\ctrulib\util\rbtree \
				Sources\NTR
IP			:=  190
FTP_HOST 	:=	192.168.137.
FTP_PORT	:=	"5000"
FTP_PATH	:=	"plugin/0004000000033600/"

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ARCH	:=	-march=armv6k -mlittle-endian -mtune=mpcore -mfloat-abi=hard 

CFLAGS	:=	-g -O2 -mword-relocations \
 			-fomit-frame-pointer -ffunction-sections -fno-strict-aliasing \
			$(ARCH)

CFLAGS		+=	$(INCLUDE) -DARM11 -D_3DS 

CXXFLAGS	:= $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++11

ASFLAGS		:=	-g $(ARCH)
LDFLAGS		:= -pie -T $(TOPDIR)/3ds.ld $(ARCH) -O2 -Wl,-Map,$(notdir $*.map),--gc-sections 
# ,-d,--emit-relocs,--use-blx,--print-gc-sections
# --gc-sections -Map=$(TARGET).map 

#LIBS	:= -lntr -lctr -lg -lsysbase  -lc -lm -lgcc -lgcov

#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUT	:=	$(CURDIR)/$(TARGET)
export TOPDIR	:=	$(CURDIR)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
					$(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

CFILES			:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES			:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
#	BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

export LD 		:= $(CXX)
export OFILES	:=	$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					-I$(CURDIR)/$(BUILD)

export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L $(dir)/lib) -L $(LIBDIRS)

.PHONY: $(BUILD) clean all

#---------------------------------------------------------------------------------
all: $(BUILD)

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
clean:
	@echo clean ... 
	@rm -fr $(BUILD) $(TARGET).3dsx $(OUTPUT).smdh $(TARGET).elf

re:
	@rm $(OUTPUT).plg $(OUTPUT).elf
	make

send:
	@echo "Sending the plugin over FTP"
	@$(TOPDIR)/sendfile.py $(TARGET).plg $(FTP_PATH) "$(FTP_HOST)$(IP)" $(FTP_PORT)
#---------------------------------------------------------------------------------


else



#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------

DEPENDS	:=	$(OFILES:.o=.d)
EXCLUDE := main.o cheats.o


$(OUTPUT).plg : $(OUTPUT).elf
$(OUTPUT).a	:	$(filter-out $(EXCLUDE), $(OFILES))


$(OUTPUT).elf :	$(OFILES)



#---------------------------------------------------------------------------------
# you need a rule like this for each extension you use as binary data
#---------------------------------------------------------------------------------
%.bin.o	:	%.bin
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)

-include $(DEPENDS)

#---------------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------------
