#ifndef HOTKEY_RECORDER_WIDGET_HPP
#define HOTKEY_RECORDER_WIDGET_HPP

#include <QtCore/QString>
#include <QtCore/QEvent>
#include <QtCore/QMap>

#include <QtGui/QKeyEvent>

#include <QtWidgets/QLineEdit>

enum WINAPI_KEYBOARD_MODIFIER {
    WINMOD_NULLMOD    = NULL,
    WINMOD_ALT        = 0x01,
    WINMOD_CONTROL    = 0x02,
    WINMOD_SHIFT      = 0x04,
    WINMOD_WIN        = 0x08
};

extern const QList<WINAPI_KEYBOARD_MODIFIER>& ALL_WINAPI_KEYBOARD_MODIFIERS;

// Conversions from QStrings to keyboard modifier types.
// ----------------------------------------------------------------------------------------------------
extern WINAPI_KEYBOARD_MODIFIER QStringToWinApiKbModifier(QString);
extern WINAPI_KEYBOARD_MODIFIER QtKeyModifierToWinApiKbModifier(const Qt::Key&);

// Conversions from keyboard modifier types to QStrings.
// ----------------------------------------------------------------------------------------------------
extern QString WinApiKbModifierToQString(const WINAPI_KEYBOARD_MODIFIER& modifier,
                                         const bool& capitalize = false);

extern QString WinApiKbModifierBitmaskToQString(const quint32& modifiers,
                                                const bool& capitalize = false);

extern QString QtKeyModifierToQString(const Qt::Key& modifier_qtkey,
                                      const bool& capitalize = false);

class HotkeyRecorderWidget : public QLineEdit {
Q_OBJECT
public:
    struct WindowsHotkey {
        Qt::KeyboardModifiers QtModifiers;
        QList<Qt::Key> QtModifierKeys;
        Qt::Key QtKey;

        quint32 Modifiers;
        quint32 ScanCode;
        quint32 Vkid;

        bool operator==(const WindowsHotkey&) const;
        bool operator!=(const WindowsHotkey&) const;
        operator QString() const;
        operator bool() const;

        QString ToString() const;
        void Clear();
    };

signals:
    void HotkeyRecorded(HotkeyRecorderWidget::WindowsHotkey);

protected:
    virtual bool event(QEvent*) override;

private:
    WindowsHotkey lastWindowsHotkeyEmitted;
    WindowsHotkey windowsHotkey;
    bool mainKeyEstablished;
    bool isRecording;

public slots:
    void StartRecording();
    void StopRecording();

public:
    void UpdateWindowsHotkey(const WindowsHotkey& windows_hotkey);
    bool IsRecording() const;
    void ClearState();

    HotkeyRecorderWidget(QWidget* = nullptr);
};

#endif // HOTKEY_RECORDER_WIDGET_HPP
