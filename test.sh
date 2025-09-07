#!/bin/bash

sudo insmod avm_kernel_logger.ko

echo "Test1 Test2 Test3"  > /proc/avm_kernel_logger

sleep 5

sudo rmmod avm_kernel_logger
sudo dmesg | tail -n 5
