KM_NAME := lcompat

override SRCFILES := $(shell find . -name "*.[Sc]" | LC_ALL=C sort)
MODULE_LINK_LIBS := $(PROJECT_ROOT)/libgcc_$(ARCH).a
MODULE_INCLUDE_DIRS += $(PROJECT_ROOT)/modules/lcompat/include

include $(PROJECT_ROOT)/modules/build/module.mk
