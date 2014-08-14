TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    writeWav.c \
    support.c

OTHER_FILES += \
    Makefile \
    README.md \
    writeWav

HEADERS += \
    support.h

