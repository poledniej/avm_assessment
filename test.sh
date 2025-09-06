#!/bin/bash

make install

sudo insmod build/avm_kernel_logger.ko
sudo rmmod avm_kernel_logger
sudo dmesg | tail -n 20
