#include "hotkey_recorder_widget.hpp"

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

    QKeyEvent*                key_event          { reinterpret_cast<QKeyEvent*>(event)    };
    const Qt::Key&            qtkey              { static_cast<Qt::Key>(key_event->key()) };
    const WINAPI_MODIFIER&    winapi_modifier    { QtKeyModifierToWinApiKbModifier(qtkey) };

    if(event_type == QEvent::KeyRelease) {
        if(windowsHotkey.Vkid && !winapi_modifier && windowsHotkey != lastWindowsHotkeyEmitted) {
            emit HotkeyRecorded(windowsHotkey);
            lastWindowsHotkeyEmitted = windowsHotkey;
            mainKeyEstablished = true;
        } else if(!windowsHotkey.Vkid && winapi_modifier) {
            windowsHotkey.Modifiers &= ~winapi_modifier;

            if(!windowsHotkey.Modifiers) {
                windowsHotkey.Clear();
            }

            setText(windowsHotkey.ToString());
        }
    }

    else if(event_type == QEvent::KeyPress && !key_event->isAutoRepeat() && key_event->key()) {
        if(mainKeyEstablished && winapi_modifier) {
            windowsHotkey.Clear();
            mainKeyEstablished = false;
        }

        if(winapi_modifier) {
            windowsHotkey.Modifiers |= winapi_modifier;
            windowsHotkey.QtModifierKeys.append(qtkey);
        } else {
            windowsHotkey.Vkid        = key_event->nativeVirtualKey();
            windowsHotkey.ScanCode    = key_event->nativeScanCode();
            windowsHotkey.QtKey       = qtkey;
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
