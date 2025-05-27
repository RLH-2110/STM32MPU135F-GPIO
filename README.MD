# LED Test Program and Library

This Repo contains a Library to work with the GPIO on the STM32MP135F-DK, and a test program that works with the library and can be used to control the LEDs.  

This is still in development, and I do not encourage serious use at the moment, there is still much to improve.  

> [!NOTE]
> LD6 and LD7 are connected with I2C, and Currently need modifications to the Device Tree to work, the modified Device Tree is currently not in the Repo.

# Compiling

This is meant to work with Yocto, and it has a Yocto Recipe, you should not worry about compiling it manually.  
Consult the Yocto Documentation if you want to know how to add a recipe.  
