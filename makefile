# ----------------------------
# Makefile Options
# ----------------------------

NAME = BOING
ICON = icon.png
DESCRIPTION = "port of AMIGA 'boing' demo"
COMPRESSED = YES

CFLAGS = -Wall -Wextra -Oz
CXXFLAGS = -Wall -Wextra -Oz

# ----------------------------

include $(shell cedev-config --makefile)
