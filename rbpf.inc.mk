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

CLANG ?= clang

all: blobs

INC_FLAGS = \
	-nostdinc \
	-isystem `$(CLANG) -print-file-name=include` \
	-I$(dir $(firstword $(MAKEFILE_LIST)))bpf/include

# Generate build targets
# $1 -> Source file
# $2 -> Blob file
define GenerateTarget
TARGET_OBJ := $(2:.bin=.obj)
$$(TARGET_OBJ): $1
	@echo "rBPF: CC $$<"
	$(Q) mkdir -p $$(@D)
	$(Q) $$(CLANG) -Wall -Wextra -g3 -Os $$(INC_FLAGS) -target bpf -c $$< -o $$@
$2: $$(TARGET_OBJ)
	$(Q) $$(RBPF_GENRBF) generate $$< $$@.tmp
	$(Q) mv -f $$@.tmp $$@
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
	@echo "rBPF: Create $(patsubst $(PROJECT_DIR)/%,%,$@)"
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
@printf "IMPORT_FSTR_ARRAY($(call GetSymbolName,$1), uint8_t, $(patsubst $(PROJECT_DIR)/%,PROJECT_DIR \"/%\",$(call BlobFile,$1)))\n" >> $@

endef

$(RBPF_SRCFILE): $(call BlobFile,$(RBPF_SOURCES))
	@echo "rBPF: Create $(patsubst $(PROJECT_DIR)/%,%,$@)"
	@echo "#include <FlashString/Array.hpp>" > $@
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

#
# $1 -> Path to blob
define DumpBlob
@echo Contents of \"$(patsubst $(RBPF_OBJDIR)/%,%,$1)\"
@echo -----------------------------
@$(RBPF_GENRBF) dump $1
@echo ""

endef

.PHONY: dump
dump: blobs
	$(foreach f,$(call BlobFile,$(RBPF_SOURCES)),$(call DumpBlob,$f))

.PHONY: clean
clean:
	$(Q) rm -rf $(RBPF_OUTDIR)
