# Recipe created by recipetool
# This is the basis of a recipe and may need further editing in order to be fully functional.
# (Feel free to remove these comments when editing.)

# WARNING: the following LICENSE and LIC_FILES_CHKSUM values are best guesses - it is
# your responsibility to verify that the values are complete and correct.
#
# The following license files were not able to be identified and are
# represented as "Unknown" below, you will need to check them yourself:
#   LICENSE
LICENSE = "CC0-1.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=65d3616852dbf7b1a6d4b53b00626032"
#LIC_FILES_CHKSUM = "file:///home/hartung/workdir/layers/meta-st/meta-hartung-software/recipes-example/ledtst/ledtst/LICENSE"

# No information for SRC_URI yet (only an external source tree was specified)
SRC_URI = "file:///home/hartung/workdir/layers/meta-st/meta-hartung-software/recipes-example/ledtst/ledtst/;protocol=file;branch=master"

# NOTE: no Makefile found, unable to determine what needs to be done

do_configure () {
	# Specify any needed configure commands here
	:
}

do_compile () {
	# Specify compilation commands here
	${CC} ${CFLAGS} ${LDFLAGS} led.c -o led -Wall
}

do_install () {
	# Specify install commands here
	install -d ${D}/usr/bin
        install -m 0777 led ${D}/usr/bin

        # (unrelated quick fix)
	<install -d ${D}/boot
        install -m 0777 stm32mp135f-dk-a7-examples.dtb.bak ${D}/boot
        install -m 0777 stm32mp135f-dk.dtb.bak ${D}/boot
        install -m 0777 swap.sh ${D}/boot
}
 
