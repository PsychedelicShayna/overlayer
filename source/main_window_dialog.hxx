#ifndef MAINWINDOW_HXX
#define MAINWINDOW_HXX

#include <QtWidgets/QMainWindow>

#include <QtCore/QResource>
#include <QtCore/QFileInfo>
#include <QtCore/QFile>

#ifndef WIN32_MEAN_AND_LEAN
#define WIN32_MEAN_AND_LEAN
#endif

#include <Windows.h>
#include <TlHelp32.h>

#include "process_scanner_dialog.hxx"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
Q_OBJECT
protected:
    Ui::MainWindow* ui;

protected:
    struct LWA {
        DWORD      ColorRef;
        uint8_t    Alpha;
        DWORD      Flags;
    };

    struct WindowState {
        int32_t    ExStyle;
        LWA        LWAttributes;
    };

    const QString styleSheetFilePath;

    ProcessScannerDialog* processScannerDialog;

    QMap<HWND, WindowState> originalWindowStates;
    HWND selectedWindowHandle;

    static LWA        getLWAttributes(const HWND& window_handle);
    static void       setWindowAlpha(const  HWND& window_handle, const uint8_t& alpha);
    static uint8_t    getWindowAlpha(const  HWND& window_handle);

    static bool       hasTransparencyFlag(const HWND&    window_handle);
    static int32_t    addTransparencyFlag(const HWND&    window_handle);
    static int32_t    removeTransparencyFlag(const HWND& window_handle);

    static bool       hasClickthroughFlag(const HWND&    window_handle);
    static int32_t    addClickthroughFlag(const HWND&    window_handle);
    static int32_t    removeClickthroughFlag(const HWND& window_handle);

protected slots:
    void setSelectedWindowAlphaToSliderValue();

    void addTransparencyFlagToSelectedWindow();
    void removeTransparencyFlagFromSelectedWindow();

    void addClickthroughFlagToSelectedWindow();
    void removeClickthroughFlagFromSelectedWindow();

    void setModificationControlsEnabled(bool enabled);
    void on_pushButtonEnableModifications_clicked();

    // Opacity modifier widgets/
    void on_spinBoxOpacityValue_valueChanged(int);
    void on_horizontalSliderOpacity_valueChanged(int);

    void on_pushButtonSelectWindow_clicked();

    void restoreOriginalWindowStates();
    void on_pushButtonRestoreWindows_clicked();

public:
    qsizetype LoadAndApplyStylesheet(const QString& file_path);

    MainWindow(QWidget* parent = nullptr);
    virtual ~MainWindow() override;
};

#endif // MAINWINDOW_HXX
