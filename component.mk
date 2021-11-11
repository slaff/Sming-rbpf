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

export RBPF_BLOBDIR	:= $(PROJECT_DIR)/out/rbpf/obj
export RBPF_INCDIR	:= $(PROJECT_DIR)/out/rbpf/include
COMPONENT_INCDIRS	+= $(RBPF_INCDIR)

RBPF_MAKE = $(MAKE) -C $(RBPF_CONTAINER_PATH) --no-print-directory -f $(RBPF_COMPONENT_PATH)/rbpf.inc.mk

.PHONY: rbpf-blobs
rbpf-blobs: | $(RBPF_CONTAINER_PATH) $(RBPF_BLOBDIR) ##Compile container objects
	$(Q) $(RBPF_MAKE) blobs

.PHONY: rbpf-blobs-clean
rbpf-blobs-clean: ##Remove generated rBPF files
	$(Q) rm -rf $(RBPF_BLOBDIR)
	$(Q) $(RBPF_MAKE) clean

.PHONY: rbpf-blobs-dump
rbpf-blobs-dump: ##Show container application
	$(Q) $(RBPF_MAKE) dump

$(RBPF_BLOBDIR):
	$(Q) mkdir -p $@

COMPONENT_PREREQUISITES := rbpf-blobs

ifndef MAKE_DOCS
clean: rbpf-blobs-clean
endif
