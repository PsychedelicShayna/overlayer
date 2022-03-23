#ifndef MAINWINDOW_HXX
#define MAINWINDOW_HXX

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QCheckBox>

#include <QtCore/QResource>
#include <QtCore/QFileInfo>
#include <QtCore/QFile>

#ifndef WIN32_MEAN_AND_LEAN
#define WIN32_MEAN_AND_LEAN
#endif

#include <Windows.h>
#include <TlHelp32.h>

#include "process_scanner_dialog.hxx"
#include "list_widget_window_item.hpp"
#include "hotkey_recorder_widget.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
Q_OBJECT
protected:
    Ui::MainWindow* ui;

signals:
    void clickthroughToggleHotkeyPressed();

protected:
    const QString            styleSheetFilePath;
    ProcessScannerDialog*    processScannerDialog;
    QTimer*                  timerRemoveInvalidWindows;

    QHBoxLayout*             horizontalLayoutClickthroughHotkeyWidgets;
    HotkeyRecorderWidget*    hotkeyRecorderWidgetClickthrough;
    QCheckBox*               checkBoxEnableClickthrough;
    quint32                  clickthroughHotkeyVkid;
    quint32                  clickthroughHotkeyId;
    bool                     clickthroughToggleStateEnabled;

    bool nativeEvent(const QByteArray& event_type, void* message, qintptr* result);

    // Make a window grabber tiemr, because clicking multiple times breaks it.

protected:
    Q_SLOT void removeInvalidWindowsFromLists();
    Q_SLOT void addWindowToInactiveList(const HWND& window_handle);

    Q_SLOT void registerClickthroughHotkey(HotkeyRecorderWidget::WindowsHotkey);
    Q_SLOT void unregisterClickthroughHotkey();

    Q_SLOT void spawnProcessScannerDialog();

    QTimer* timerWindowGrabber;
    quint32 windowGrabAttemptCounter;
    Q_SLOT void startWindowGrabber();

    Q_SLOT void selectedInactiveWindows_Activate();
    Q_SLOT void selectedActiveWindows_Deactivate();

// Operations that act on active windows that are currently selected.
// ----------------------------------------------------------------------------------------------------
    // Resets all modifications that have been made to the selected windows,
    // restoring the modified settings to what they were initially.
    Q_SLOT void selectedActiveWindows_ResetModifications();

    // Enables/disables the clickthrough modification for the selected window(s).
    Q_SLOT void selectedActiveWindows_EnableClickthrough();
    Q_SLOT void selectedActiveWindows_DisableClickthrough();

    // Toggles the clickthrough modification for all windows that respond to the
    // clickthrough toggle hotkey.
    Q_SLOT void selectedActiveWindows_ToggleClickthrough();

    // Make the selected windows respond or not respond to the clickthrough toggle hotkey.
    Q_SLOT void selectedActiveWindows_SetClickthroughToggleHotkeyEnabled(qint32 enabled);

    // Enable/disable the topmost modification for the selected windows.
    Q_SLOT void selectedActiveWindows_EnableTopmost();
    Q_SLOT void selectedActiveWindows_DisableTopmost();

    // Enable/disable the transparency modification for the selected windows.
    Q_SLOT void selectedActiveWindows_EnableTransparency();
    Q_SLOT void selectedActiveWindows_DisableTransparency();

    // Writes the current alpha value of the alpha spinbox to the modified states
    // of the windows currently selected.
    Q_SLOT void selectedActiveWindows_WriteSliderAlphaToModifiedState();

    // Updates the modification control widgets with the current modification settings
    // stored inside of the selected ListWidgetWindowItem(s).
    Q_SLOT void selectedActiveWindows_WriteModifiedStateToWidgets();

    Q_SLOT void on_spinBoxAlpha_valueChanged(int);
    Q_SLOT void on_horizontalSliderAlpha_valueChanged(int);

public:
    qsizetype LoadAndApplyStylesheet(const QString& file_path);

    MainWindow(QWidget* parent = nullptr);
    virtual ~MainWindow() override;
};

#endif // MAINWINDOW_HXX
