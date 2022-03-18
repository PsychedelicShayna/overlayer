QT += core gui widgets

TARGET = opacity-chan
TEMPLATE = app

CONFIG += C++17
QMAKE_CXXFLAGS += /std:C++17

SOURCES +=                             \
    source\main.cpp                    \
    source\main_window_dialog.cxx      \
    source\process_scanner_dialog.cxx  \
    source\process_scanner.cpp

HEADERS +=                             \
    source\main_window_dialog.hxx      \
    source\process_scanner_dialog.hxx  \
    source\process_scanner.hpp

FORMS +=                               \
    source\main_window_dialog.ui       \
    source\process_scanner_dialog.ui

LIBS += -luser32 -ladvapi32 -lkernel32

