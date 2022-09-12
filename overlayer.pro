QT += core gui widgets

TARGET = overlayer
TEMPLATE = app

CONFIG += C++17
QMAKE_CXXFLAGS += /std:c++17


# SUBMODULE: Qt Hotkey Recorder Widget
# ==================================================
INCLUDEPATH += submodules/qt-hotkey-recorder-widget/
SOURCES     += submodules/qt-hotkey-recorder-widget/hotkey_recorder_widget.cpp
HEADERS     += submodules/qt-hotkey-recorder-widget/hotkey_recorder_widget.hpp
# ==================================================


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


# Resources
# ==================================================
RC_ICONS += resources/appicon.ico


# Main Source Code
# ==================================================
LIBS += -luser32 -ladvapi32 -lkernel32

SOURCES += \
    source/list_widget_window_item.cpp \
    source/main.cpp \
    source/main_window_dialog.cxx

HEADERS += \
    source/list_widget_window_item.hpp \
    source/main_window_dialog.hxx

FORMS += \
    source\main_window_dialog.ui
