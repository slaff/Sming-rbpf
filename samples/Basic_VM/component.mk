COMPONENT_DEPENDS := rbpf
DISABLE_NETWORK := 1

# We require access to container header files which define shared structures
COMPONENT_INCDIRS := container/include
