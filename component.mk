# The folder where the container application source code is stored.
RBPF_CONTAINER_PATH ?= container
RBPF_CONTAINER_PATH := $(call AbsoluteSourcePath,$(PROJECT_DIR),$(RBPF_CONTAINER_PATH))

COMPONENT_INCDIRS := \
	$(RBPF_CONTAINER_PATH)/include \
	src/include \
	bpf/include

COMPONENT_RELINK_VARS := BPF_USE_JUMPTABLE BPF_STORE_NUM_VALUES
BPF_USE_JUMPTABLE ?= 1
BPF_STORE_NUM_VALUES ?= 16
COMPONENT_CFLAGS := -DCONFIG_BPF_STORE_NUM_VALUES=$(BPF_STORE_NUM_VALUES)

COMPONENT_SRCDIRS := \
	src \
	bpf

COMPONENT_APPCODE := src/appcode

ifeq ($(BPF_USE_JUMPTABLE),1)
  COMPONENT_SRCFILES += bpf/option/jumptable.c
else
  COMPONENT_SRCFILES += bpf/option/instruction.c
endif

COMPONENT_DOXYGEN_INPUT := src/include

RBPF_COMPONENT_PATH := $(COMPONENT_PATH)
export RBPF_GENRBF := $(PYTHON) $(COMPONENT_PATH)/tools/gen_rbf.py $(if $(V),--verbose)

##@rBPF containers

export RBPF_OUTDIR	:= $(PROJECT_DIR)/out/rbpf
COMPONENT_INCDIRS	+= $(RBPF_OUTDIR)/include
COMPONENT_APPCODE	+= $(RBPF_OUTDIR)

RBPF_MAKE = $(MAKE) -C $(RBPF_CONTAINER_PATH) --no-print-directory -f $(RBPF_COMPONENT_PATH)/rbpf.inc.mk

.PHONY: rbpf-blobs
rbpf-blobs: | $(RBPF_CONTAINER_PATH) ##Compile container objects
	+$(Q) $(RBPF_MAKE) blobs

.PHONY: rbpf-blobs-clean
rbpf-blobs-clean: ##Remove generated rBPF files
	+$(Q) $(RBPF_MAKE) clean

.PHONY: rbpf-dump
rbpf-dump: ##Dump contents of compiled container applications
	+$(Q) $(RBPF_MAKE) dump

COMPONENT_PREREQUISITES := rbpf-blobs

ifndef MAKE_DOCS
clean: rbpf-blobs-clean
endif
