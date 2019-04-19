
    .global _RESOURCES_PACKAGE
    .global _RESOURCES_PACKAGE_SIZE
    .section __TEXT,__const
_RESOURCES_PACKAGE:
    .incbin "resources.bin"
1:
_RESOURCES_PACKAGE_SIZE:
    .int 1b - _RESOURCES_PACKAGE

