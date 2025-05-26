#!/bin/bash

mv /boot/stm32mp135f-dk.dtb /boot/stm32mp135f-dk.dtb.tmp
mv /boot/stm32mp135f-dk.dtb.bak /boot/stm32mp135f-dk.dtb
mv /boot/stm32mp135f-dk.dtb.tmp /boot/stm32mp135f-dk.dtb.bak


mv /boot/stm32mp135f-dk-a7-examples.dtb /boot/stm32mp135f-dk-a7-examples.dtb.tmp
mv /boot/stm32mp135f-dk-a7-examples.dtb.bak /boot/stm32mp135f-dk-a7-examples.dtb
mv /boot/stm32mp135f-dk-a7-examples.dtb.tmp /boot/stm32mp135f-dk-a7-examples.dtb.bak

reboot now
