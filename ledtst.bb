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

FILESEXTRAPATHS:prepend := "${THISDIR}:"
SRC_URI = "file://ledtst/"
S = "${WORKDIR}/ledtst"

# NOTE: no Makefile found, unable to determine what needs to be done

do_configure () {
	# Specify any needed configure commands here
	:
}

do_compile () {
	# Specify compilation commands here
	
	${CC} ${CFLAGS} -c riolib.c -o riolib.o -Wall
        ${AR} rcs libriolib.a riolib.o
	${CC} led.c -L. -lriolib -o led ${LDFLAGS}

}


do_install () {
	# Specify install commands here
	install -d ${D}/usr/bin
        install -m 0777 led ${D}${bindir}
}
 
