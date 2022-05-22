QT += core gui widgets

TARGET = overlayer
TEMPLATE = app

CONFIG += C++17
QMAKE_CXXFLAGS += /std:c++17

INCLUDEPATH += submodules/qt-hotkey-recorder-widget/source/
SOURCES     += submodules/qt-hotkey-recorder-widget/source/hotkey_recorder_widget.cpp
HEADERS     += submodules/qt-hotkey-recorder-widget/source/hotkey_recorder_widget.hpp

SOURCES +=                              \
    source/list_widget_window_item.cpp  \
    source/main.cpp                     \
    source/main_window_dialog.cxx       \
    source/process_scanner_dialog.cxx   \
    source/process_scanner.cpp          \

HEADERS +=                              \
    source/list_widget_window_item.hpp  \
    source/main_window_dialog.hxx       \
    source/process_scanner_dialog.hxx   \
    source/process_scanner.hpp          \

FORMS +=                               \
    source\main_window_dialog.ui       \
    source\process_scanner_dialog.ui

LIBS += -luser32 -ladvapi32 -lkernel32

