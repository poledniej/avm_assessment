obj-m := avm_kernel_logger.o

# Name des Moduls
MODULE_NAME := avm_kernel_logger

# Kernel Build-Verzeichnis
KERNEL_DIR := /lib/modules/$(shell uname -r)/build

# Pfad zum aktuellen Verzeichnis
PWD := $(shell pwd)

# Standardziel
all:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules

# Aufräumen im Build-Verzeichnis
clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean
	@rm -rf $(BUILD_DIR)

