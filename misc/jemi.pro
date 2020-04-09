CONFIG -= qt
CONFIG += exceptions c++11 c11

INCLUDEPATH += /usr/include
INCLUDEPATH += /usr/include/c++/4.7
INCLUDEPATH += ../include
INCLUDEPATH += ../src
INCLUDEPATH += ../deps/mutest

QMAKE_CXXFLAGS += -std=c++11
QMAKE_CFLAGS += -std=c11
QMAKE_LFLAGS += -std=c++11 -std=c11

HEADERS += \ 
    ../include/jemi.h \
    ../include/jemi++.h
SOURCES += \ 
    ../src/jemi.c \
    ../tests/test_jemi.cc

OTHER_FILES += \
    ../CMakeLists.txt \
    ../src/CMakeLists.txt \
    ../test/CMakeLists.txt

DISTFILES += \
    ../tests/CMakeLists.txt \
    ../README.md
