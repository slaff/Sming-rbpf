COMPONENT_INCDIRS := \
	bpf/include

COMPONENT_RELINK_VARS := BPF_USE_JUMPTABLE
BPF_USE_JUMPTABLE := 1

COMPONENT_SRCDIRS := \
	bpf

ifeq ($(BPF_USE_JUMPTABLE),1)
  COMPONENT_SRCFILES += bpf/option/jumptable.c
else
  COMPONENT_SRCFILES += bpf/option/instruction.c
endif

RBPF_COMPONENT_PATH := $(COMPONENT_PATH)
export RBPF_GENRBF := $(COMPONENT_PATH)/tools/gen_rbf.py

# The folder where the container application source code is stored.
RBPF_CONTAINER_PATH ?= $(PROJECT_DIR)/container

##@rBPF containers

export RBPF_OUTDIR	:= $(PROJECT_DIR)/out/rbpf
COMPONENT_INCDIRS	+= $(RBPF_OUTDIR)/include
COMPONENT_APPCODE	:= $(RBPF_OUTDIR)

RBPF_MAKE = $(MAKE) -C $(RBPF_CONTAINER_PATH) --no-print-directory -f $(RBPF_COMPONENT_PATH)/rbpf.inc.mk

.PHONY: rbpf-blobs
rbpf-blobs: | $(RBPF_CONTAINER_PATH) ##Compile container objects
	$(Q) $(RBPF_MAKE) blobs

.PHONY: rbpf-blobs-clean
rbpf-blobs-clean: ##Remove generated rBPF files
	$(Q) $(RBPF_MAKE) clean

.PHONY: rbpf-blobs-dump
rbpf-blobs-dump: ##Show container application
	$(Q) $(RBPF_MAKE) dump

COMPONENT_PREREQUISITES := rbpf-blobs

ifndef MAKE_DOCS
clean: rbpf-blobs-clean
endif
