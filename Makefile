obj-m := avm_kernel_logger.o

# Name des Moduls (ohne .c und .o)
MODULE_NAME := avm_kernel_logger

# Unterordner, in dem gebaut werden soll
BUILD_DIR := build

# Kernel Build-Verzeichnis (läuft automatisch gegen das aktuell laufende Kernel)
KERNEL_DIR := /lib/modules/$(shell uname -r)/build

# Pfad zum aktuellen Verzeichnis
PWD := $(shell pwd)

# Standardziel
all:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) MO=$(PWD)/$(BUILD_DIR) modules

# Aufräumen im Build-Verzeichnis
clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) MO=$(PWD)/$(BUILD_DIR) clean
	@rm -rf $(BUILD_DIR)

# Installiert das Modul ins System
install:
	sudo cp $(BUILD_DIR)/$(MODULE_NAME).ko /lib/modules/$(shell uname -r)/extra/
	sudo depmod -a
