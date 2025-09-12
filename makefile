# ----------------------------
# Makefile Options
# ----------------------------

NAME = BOING
ICON = icon.png
DESCRIPTION = "port of AMIGA 'boing' demo"
COMPRESSED = YES

CFLAGS = -Wall -Wextra -Ofast
CXXFLAGS = -Wall -Wextra -Ofast

# ----------------------------

include $(shell cedev-config --makefile)
