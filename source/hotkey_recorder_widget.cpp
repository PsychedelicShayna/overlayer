#include "hotkey_recorder_widget.hpp"

const QList<WINAPI_KEYBOARD_MODIFIER>& ALL_WINAPI_KEYBOARD_MODIFIERS {
    WINMOD_ALT,
    WINMOD_CONTROL,
    WINMOD_SHIFT,
    WINMOD_WIN
};

WINAPI_KEYBOARD_MODIFIER QStringToWinApiKbModifier(QString modifier_string) {
    for(auto& character : modifier_string) {
        character = character.toUpper();
    }

    static const QMap<QString, WINAPI_KEYBOARD_MODIFIER>& conversion_table {
        { "ALT",     WINMOD_ALT     },
        { "CTRL",    WINMOD_CONTROL },
        { "CONTROL", WINMOD_CONTROL },
        { "SHIFT",   WINMOD_SHIFT   },
        { "SHFT",    WINMOD_SHIFT   },
        { "WIN",     WINMOD_WIN     }
    };

    return conversion_table.contains(modifier_string) ? conversion_table[modifier_string] : WINMOD_NULLMOD;
}

WINAPI_KEYBOARD_MODIFIER QtKeyModifierToWinApiKbModifier(const Qt::Key& qtkey) {
    static const QMap<Qt::Key, WINAPI_KEYBOARD_MODIFIER>& conversion_table {
        { Qt::Key_Alt,        WINMOD_ALT     },
        { Qt::Key_Control,    WINMOD_CONTROL },
        { Qt::Key_Shift,      WINMOD_SHIFT   },
        { Qt::Key_Meta,       WINMOD_WIN     }
    };

    return conversion_table.contains(qtkey) ? conversion_table[qtkey] : WINMOD_NULLMOD;
}

QString WinApiKbModifierToQString(const WINAPI_KEYBOARD_MODIFIER& winapi_keyboard_modifier, const bool& capitalized) {
    static const QMap<WINAPI_KEYBOARD_MODIFIER, QString>& conversion_table {
        { WINMOD_CONTROL,    "Ctrl" },
        { WINMOD_SHIFT,      "Shift"   },
        { WINMOD_ALT,        "Alt"     },
        { WINMOD_WIN,        "Win"     }
    };

    QString converted_string {
        conversion_table.contains(winapi_keyboard_modifier) ? conversion_table[winapi_keyboard_modifier] : ""
    };

    if(capitalized && converted_string.size()) {
        for(QChar& character : converted_string) {
            character = character.toUpper();
        }
    }

    return converted_string;
}

QString WinApiKbModifierBitmaskToQString(const quint32& winapi_keyboard_modifier, const bool& capitalized) {
    QString string_sequence;

    if(winapi_keyboard_modifier & WINMOD_CONTROL)
        string_sequence += WinApiKbModifierToQString(WINMOD_CONTROL, capitalized) + " + ";

    if(winapi_keyboard_modifier & WINMOD_SHIFT)
        string_sequence += WinApiKbModifierToQString(WINMOD_SHIFT, capitalized) + " + ";

    if(winapi_keyboard_modifier & WINMOD_ALT)
        string_sequence += WinApiKbModifierToQString(WINMOD_ALT, capitalized) + " + ";

    if(winapi_keyboard_modifier & WINMOD_WIN)
        string_sequence += WinApiKbModifierToQString(WINMOD_WIN, capitalized) + " + ";

    return string_sequence;
}

QString QtKeyModifierToQString(const Qt::Key& modifier_qtkey, const bool& capitalize) {
    const WINAPI_KEYBOARD_MODIFIER& qtkey_modifier { QtKeyModifierToWinApiKbModifier(modifier_qtkey) };
    return WinApiKbModifierToQString(qtkey_modifier, capitalize);
}

bool HotkeyRecorderWidget::WindowsHotkey::operator==(const WindowsHotkey& other) const {
    return    QtModifiers      == other.QtModifiers
           && QtModifierKeys   == other.QtModifierKeys
           && QtKey            == other.QtKey
           && Modifiers        == other.Modifiers
           && ScanCode         == other.ScanCode
           && Vkid             == other.Vkid;
}

bool HotkeyRecorderWidget::WindowsHotkey::operator!=(const WindowsHotkey& other) const {
    return !(*this==other);
}

HotkeyRecorderWidget::WindowsHotkey::operator QString() const {
    QString modifier_sequence { WinApiKbModifierBitmaskToQString(Modifiers, false) };

    if(Vkid) {
        modifier_sequence += QKeySequence { QtKey }.toString();
    }

    return modifier_sequence;
}

HotkeyRecorderWidget::WindowsHotkey::operator bool() const {
    return    QtModifiers              != NULL
           || QtModifierKeys.size()    != NULL
           || QtKey                    != NULL
           || ScanCode                 != NULL
           || Vkid                     != NULL;
}

QString HotkeyRecorderWidget::WindowsHotkey::ToString() const {
    return QString(*this);
}

void HotkeyRecorderWidget::WindowsHotkey::Clear() {
    QtModifierKeys.clear();

    QtModifiers = Qt::KeyboardModifier::NoModifier;
    QtKey = static_cast<Qt::Key>(NULL);

    Modifiers    = NULL;
    ScanCode     = NULL;
    Vkid         = NULL;
}

bool HotkeyRecorderWidget::event(QEvent* event) {
    const auto& event_type { event->type() };

    if(event_type != QEvent::KeyPress && event_type != QEvent::KeyRelease) {
        return QLineEdit::event(event);
    }

    if(!isRecording) return false;

    QKeyEvent* key_event
        { reinterpret_cast<QKeyEvent*>(event) };

    const Qt::Key& qt_key
        { static_cast<Qt::Key>(key_event->key()) };

    const WINAPI_KEYBOARD_MODIFIER& winapi_keyboard_modifier
        { QtKeyModifierToWinApiKbModifier(qt_key) };

    if(event_type == QEvent::KeyRelease) {
        if(windowsHotkey.Vkid && !winapi_keyboard_modifier && windowsHotkey != lastWindowsHotkeyEmitted) {
            emit HotkeyRecorded(windowsHotkey);
            lastWindowsHotkeyEmitted = windowsHotkey;
            mainKeyEstablished = true;
        } else if(!windowsHotkey.Vkid && winapi_keyboard_modifier) {
            windowsHotkey.Modifiers &= ~winapi_keyboard_modifier;

            if(!windowsHotkey.Modifiers) {
                windowsHotkey.Clear();
            }

            setText(windowsHotkey.ToString());
        }
    }

    else if(event_type == QEvent::KeyPress && !key_event->isAutoRepeat() && key_event->key()) {
        if(mainKeyEstablished && winapi_keyboard_modifier) {
            windowsHotkey.Clear();
            mainKeyEstablished = false;
        }

        if(winapi_keyboard_modifier) {
            windowsHotkey.Modifiers |= winapi_keyboard_modifier;
            windowsHotkey.QtModifierKeys.append(qt_key);
        } else {
            windowsHotkey.Vkid        = key_event->nativeVirtualKey();
            windowsHotkey.ScanCode    = key_event->nativeScanCode();
            windowsHotkey.QtKey       = qt_key;
        }

        setText(windowsHotkey.ToString());
    }

    event->setAccepted(true);
    return true;
}

void HotkeyRecorderWidget::StartRecording() {
    if(!isRecording) {
        isRecording = true;
    }
}

void HotkeyRecorderWidget::StopRecording() {
    if(isRecording) {
        isRecording = false;
        ClearState();
    }
}

bool HotkeyRecorderWidget::IsRecording() const {
    return isRecording;
}

void HotkeyRecorderWidget::ClearState() {
    mainKeyEstablished = false;
    lastWindowsHotkeyEmitted.Clear();
    windowsHotkey.Clear();
    clear();
}

HotkeyRecorderWidget::HotkeyRecorderWidget(QWidget* parent)
    :
      QLineEdit             { parent      },
      mainKeyEstablished    { false       },
      isRecording           { false       }
{
    windowsHotkey.Clear();
}
