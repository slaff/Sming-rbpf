include $(SMING_HOME)/util.mk

ifeq (,$(RBPF_OUTDIR))
$(error RBPF_OUTDIR undefined)
endif

RBPF_OBJDIR := $(RBPF_OUTDIR)/obj
RBPF_INCDIR := $(RBPF_OUTDIR)/include

# Obtain blob file path
# $1 -> source file(s)
define BlobFile
$(addprefix $(RBPF_OBJDIR)/,$(patsubst %,%.bin,$(basename $1)))
endef

# List of relative paths to source files
RBPF_SOURCES	:= $(patsubst $(CURDIR)/%,%,$(call ListAllFiles,$(CURDIR),*.c *.cpp))
# Header file for all defined containers
RBPF_INCFILE	:= $(RBPF_INCDIR)/rbpf/containers.h
# Source file with actual blob imports
RBPF_SRCFILE	:= $(RBPF_OUTDIR)/containers.cpp

LLC ?= llc
CLANG ?= clang

all: blobs

# Generate build targets
# $1 -> Source file
# $2 -> Blob file
define GenerateTarget
TARGET_BC := $(2:.bin=.bc) # Clang bytecode
TARGET_OBJ := $(2:.bin=.obj) # llvm BPF object code
$$(TARGET_BC): $1
	$(Q) mkdir -p $$(@D)
	$(Q) $$(CLANG) -Wall -Wextra -Werror -g3 -Os -emit-llvm -c $$< -o $$@
$$(TARGET_OBJ): $$(TARGET_BC)
	$(Q) $$(LLC) -march=bpf -mcpu=v2 -filetype=obj -o $$@ $$<
$2: $$(TARGET_OBJ)
	$$(RBPF_GENRBF) generate $$< $$@
endef
$(foreach f,$(RBPF_SOURCES),$(eval $(call GenerateTarget,$f,$(call BlobFile,$f))))


# Get name to use for blob symbol
# $1 -> source file
define GetSymbolName
$(subst /,_,$(basename $1))
endef

# Generate code for header file
# $1 -> source file
define GenerateHeader
@printf "DECLARE_FSTR_ARRAY($(call GetSymbolName,$1), uint8_t)\n" >> $@

endef

$(RBPF_INCFILE): $(call BlobFile,$(RBPF_SOURCES))
	@mkdir -p $(@D)
	@echo "#pragma once" > $@
	@echo "#include <FlashString/Array.hpp>" >> $@
	@echo "" >> $@
	@echo "namespace rBPF {" >> $@
	@echo "namespace Container {" >> $@
	$(foreach f,$(RBPF_SOURCES),$(call GenerateHeader,$f))
	@echo "} // namespace Container" >> $@
	@echo "} // namespace rBPF" >> $@


# Generate code for BLOB source file
# $1 -> source file
define GenerateSource
@printf "IMPORT_FSTR_ARRAY($(call GetSymbolName,$1), uint8_t, \"$(call BlobFile,$1)\")\n" >> $@

endef

$(RBPF_SRCFILE): $(call BlobFile,$(RBPF_SOURCES))
	@echo "#include <FlashString/Array.hpp>" >> $@
	@echo "" >> $@
	@echo "namespace rBPF {" >> $@
	@echo "namespace Container {" >> $@
	$(foreach f,$(RBPF_SOURCES),$(call GenerateSource,$f))
	@echo "} // namespace Container" >> $@
	@echo "} // namespace rBPF" >> $@


.PHONY: blobs
blobs: $(RBPF_INCFILE) $(RBPF_SRCFILE) | $(RBPF_INCDIR) $(RBPF_OBJDIR)

$(RBPF_INCDIR) $(RBPF_OBJDIR):
	$(Q) mkdir -p $@

.PHONY: dump
dump: blobs
	$(RBPF_GENRBF) dump $< 

.PHONY: clean
clean:
	$(Q) rm -rf $(RBPF_OUTDIR)
