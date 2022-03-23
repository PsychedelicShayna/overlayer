#ifndef HOTKEY_RECORDER_WIDGET_HPP
#define HOTKEY_RECORDER_WIDGET_HPP

#include <QtWidgets/QLineEdit>
#include <QtGui/QKeyEvent>
#include <QtCore/QEvent>

#include "winapi_utilities.hpp"

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
