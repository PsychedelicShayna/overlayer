QT += core gui widgets

TARGET = overlayer
TEMPLATE = app

CONFIG += C++17
QMAKE_CXXFLAGS += /std:c++17

INCLUDEPATH += submodules/qt-hotkey-recorder-widget/source/
SOURCES     += submodules/qt-hotkey-recorder-widget/source/hotkey_recorder_widget.cpp
HEADERS     += submodules/qt-hotkey-recorder-widget/source/hotkey_recorder_widget.hpp

INCLUDEPATH += submodules/qt-process-scanner-widget/


# SUBMODULE: Qt Process Scanner Widget
# ==================================================
INCLUDEPATH += submodules/qt-process-scanner-widget/

SOURCES += \
    submodules/qt-process-scanner-widget/process_scanner.cpp \
    submodules/qt-process-scanner-widget/process_scanner_dialog.cxx

HEADERS += \
    submodules/qt-process-scanner-widget/process_scanner.hpp \
    submodules/qt-process-scanner-widget/process_scanner_dialog.hxx

FORMS += \
    submodules/qt-process-scanner-widget/process_scanner_dialog.ui
# ==================================================


SOURCES += \
    source/list_widget_window_item.cpp \
    source/main.cpp \
    source/main_window_dialog.cxx

HEADERS += \
    source/list_widget_window_item.hpp \
    source/main_window_dialog.hxx

FORMS += \
    source\main_window_dialog.ui

LIBS += -luser32 -ladvapi32 -lkernel32

