
PHONY = clean
TARGET_NAME = xtest

include $(BARDYDIR)/make/macros.mk

DIRS := $(GIPCYINC) $(BARDYDIR)/BRDINC $(BARDYDIR)/BRDINC/ctrladmpro $(BARDYDIR)/IcrInc \
$(BARDYDIR)/BRDLIBS $(BARDYDIR)/AMBPLX/bambpex
INC := $(addprefix  -I, $(DIRS))

CFLAGS += $(INC)
CFLAGS += -std=c++14

SRC := $(wildcard *.cpp)
SOURCE += $(SRC)
OBJ_FILES := $(SOURCE:.cpp=.o)

ROOTFS_SO_PATH := $(BARDYDIR)/../rootfs.fmc141v.rc/rootfs-overlay/home/embedded/bardy
ROOTFS_EX_PATH := $(BARDYDIR)/../rootfs.fmc141v.rc/rootfs-overlay/home/embedded/examples
ROOTFS_KO_PATH := $(BARDYDIR)/../rootfs.fmc141v.rc/rootfs-overlay/home/embedded/drivers

all: $(TARGET_NAME)
	cp $(TARGET_NAME) $(BARDYDIR)/bin
	cp $(TARGET_NAME) $(ROOTFS_EX_PATH)

include $(BARDYDIR)/make/rules.mk
include $(BARDYDIR)/make/clean.mk
include $(BARDYDIR)/make/app.mk