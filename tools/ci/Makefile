# Don't bother with implicit checks
.SUFFIXES:

.NOTPARALLEL:

include $(SMING_HOME)/util.mk

SAMPLES_ROOT	:= $(abspath ../../samples)
SAMPLE_DIRS		:= $(call ListSubDirs,$(SAMPLES_ROOT))
SAMPLE_NAMES	:= $(notdir $(SAMPLE_DIRS))
SAMPLE_RUN		:= $(addsuffix -run,$(SAMPLE_NAMES))

all: $(AVAILABLE_SOCS)

samples: $(SAMPLE_NAMES)

.PHONY: $(AVAILABLE_SOCS)
$(AVAILABLE_SOCS):
	$(Q) $(MAKE) --no-print-directory samples SMING_SOC=$@ STRICT=1

.PHONY: $(SAMPLE_NAMES)
$(SAMPLE_NAMES):
	$(info )
	$(info )
	$(info ===================== )
	$(info Building $@ for $(SMING_SOC))
	$(info ===================== )
	$(info )
	$(Q) $(MAKE) -C $(SAMPLES_ROOT)/$@ -r -R --no-print-directory \
		$(if $(filter host,$(SMING_SOC)),flash run HOST_UART_OPTIONS=--loopcount=50)

clean:
	$(Q) cd $(SAMPLES_ROOT) && rm -rf $(addsuffix /out,$(SAMPLE_NAMES))
