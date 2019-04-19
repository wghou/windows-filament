
    .global RESOURCES_PACKAGE
    .global RESOURCES_PACKAGE_SIZE
    .section .rodata
RESOURCES_PACKAGE:
    .incbin "resources.bin"
1:
RESOURCES_PACKAGE_SIZE:
    .int 1b - RESOURCES_PACKAGE

