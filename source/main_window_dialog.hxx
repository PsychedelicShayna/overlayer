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

signals:
    void clickthroughToggleHotkeyPressed();

protected slots:
    void removeInvalidWindowsFromLists();
    void addWindowToInactiveList(const HWND& window_handle);

    void registerClickthroughHotkey(HotkeyRecorderWidget::WindowsHotkey);
    void unregisterClickthroughHotkey();

    void spawnProcessScannerDialog();
    void startWindowGrabber();

    void selectedInactiveWindows_Activate();
    void selectedActiveWindows_Deactivate();

    void selectedActiveWindows_ResetModifications();

    // Enable / disable clickthrough.
    void selectedActiveWindows_EnableClickthrough();
    void selectedActiveWindows_DisableClickthrough();
    void selectedActiveWindows_ToggleClickthrough();
    void selectedActiveWindows_SetClickthroughToggleHotkeyEnabled(qint32 enabled);

    // Enable / disable topmost.
    void selectedActiveWindows_EnableTopmost();
    void selectedActiveWindows_DisableTopmost();

    // Enbable / disable alpha based transparency on layered windows.
    void selectedActiveWindows_EnableTransparency();
    void selectedActiveWindows_DisableTransparency();

    // Set the alpha of the selected windows to the value of the alpha slider.
    void selectedActiveWindows_WriteSliderAlphaToModifiedState();

    // Set the alpha slider's value to the alpha value of the selected window.
    // If more than one window is selected, the value is not written.
    void selectedActiveWindows_WriteModifiedStateToWidgets();

    void on_spinBoxAlpha_valueChanged(int);
    void on_horizontalSliderAlpha_valueChanged(int);

public:
    qsizetype LoadAndApplyStylesheet(const QString& file_path);

    MainWindow(QWidget* parent = nullptr);
    virtual ~MainWindow() override;
};

#endif // MAINWINDOW_HXX
