QT += core gui widgets

TARGET = opacity-chan
TEMPLATE = app

CONFIG += C++17
QMAKE_CXXFLAGS += /std:c++17

SOURCES +=                              \
    source/list_widget_window_item.cpp  \
    source/main.cpp                     \
    source/main_window_dialog.cxx       \
    source/process_scanner_dialog.cxx   \
    source/process_scanner.cpp          \
    source/hotkey_recorder_widget.cpp   \
    # source/winapi_utilities.cpp

HEADERS +=                              \
    source/list_widget_window_item.hpp  \
    source/main_window_dialog.hxx       \
    source/process_scanner_dialog.hxx   \
    source/process_scanner.hpp          \
    source/hotkey_recorder_widget.hpp   \
    # source/winapi_utilities.hpp

FORMS +=                               \
    source\main_window_dialog.ui       \
    source\process_scanner_dialog.ui

LIBS += -luser32 -ladvapi32 -lkernel32

