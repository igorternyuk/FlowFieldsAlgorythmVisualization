TEMPLATE = app
CONFIG += c++1z
CONFIG -= app_bundle
CONFIG -= qt
LIBS += -lsfml-system -lsfml-window -lsfml-graphics
SOURCES += \
        main.cpp \
    game.cpp

HEADERS += \
    game.hpp
