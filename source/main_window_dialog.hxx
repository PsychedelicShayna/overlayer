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
#include "process_window_list_item.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
Q_OBJECT
protected:
    Ui::MainWindow* ui;

protected:
    const QString styleSheetFilePath;

    ProcessScannerDialog* processScannerDialog;

protected slots:
    void spawnProcessScannerDialog();
    void startWindowGrabber();

    void selectedInactiveWindows_Activate();
    void selectedActiveWindows_Deactivate();

    // Enable / disable clickthrough.
    void selectedActiveWindows_EnableClickthrough();
    void selectedActiveWindows_DisableClickthrough();

    // Enable / disable transparency.
    void selectedActiveWindows_EnableTransparency();
    void selectedActiveWindows_DisableTransparency();

    // Enable / disable topmost.
    void selectedActiveWindows_EnableTopmost();
    void selectedActiveWindows_DisableTopmost();

    // Set the alpha of the selected windows to the value of the alpha slider.
    void selectedActiveWindows_SyncAlphaToSlider();



    void on_spinBoxAlpha_valueChanged(int);
    void on_horizontalSliderAlpha_valueChanged(int);

public:
    qsizetype LoadAndApplyStylesheet(const QString& file_path);

    MainWindow(QWidget* parent = nullptr);
    virtual ~MainWindow() override;
};

#endif // MAINWINDOW_HXX
