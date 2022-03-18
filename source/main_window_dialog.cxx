#include "main_window_dialog.hxx"
#include "ui_main_window_dialog.h"

MainWindow::LWA MainWindow::getLWAttributes(const HWND& window_handle) {
    LWA attributes;
    GetLayeredWindowAttributes(window_handle, &attributes.ColorRef, &attributes.Alpha, &attributes.Flags);
    return attributes;
}

void MainWindow::setWindowAlpha(const HWND& window_handle, const uint8_t& alpha) {
    if(hasTransparencyFlag(window_handle)) {
        const int32_t& success {
            SetLayeredWindowAttributes(window_handle, NULL, alpha, LWA_ALPHA)
        };

        if(!success) {
            const uint32_t& error_code { GetLastError() };
        }
    }
}

uint8_t MainWindow::getWindowAlpha(const HWND& window_handle) {
    uint8_t alpha { NULL };
    GetLayeredWindowAttributes(window_handle, nullptr, &alpha, nullptr);
    return alpha;
}

bool MainWindow::hasTransparencyFlag(const HWND& window_handle)  {
    return GetWindowLong(window_handle, GWL_EXSTYLE) & WS_EX_LAYERED;
}

int32_t MainWindow::addTransparencyFlag(const HWND& window_handle) {
    return SetWindowLong(window_handle, GWL_EXSTYLE, GetWindowLong(window_handle, GWL_EXSTYLE) | WS_EX_LAYERED);
}

int32_t MainWindow::removeTransparencyFlag(const HWND& window_handle) {
    return SetWindowLong(window_handle, GWL_EXSTYLE, GetWindowLong(window_handle, GWL_EXSTYLE) & ~WS_EX_LAYERED);
}

bool MainWindow::hasClickthroughFlag(const HWND& window_handle) {
    return GetWindowLong(window_handle, GWL_EXSTYLE) & WS_EX_TRANSPARENT;
}

int32_t MainWindow::addClickthroughFlag(const HWND& window_handle) {
   return SetWindowLong(window_handle, GWL_EXSTYLE, GetWindowLong(window_handle, GWL_EXSTYLE) | WS_EX_TRANSPARENT);
}

int32_t MainWindow::removeClickthroughFlag(const HWND& window_handle) {
    return SetWindowLong(window_handle, GWL_EXSTYLE, GetWindowLong(window_handle, GWL_EXSTYLE) & ~WS_EX_TRANSPARENT);
}

void MainWindow::setSelectedWindowAlphaToSliderValue() {
    if(selectedWindowHandle != nullptr && IsWindow(selectedWindowHandle) && hasTransparencyFlag(selectedWindowHandle)) {
        setWindowAlpha(selectedWindowHandle, static_cast<uint8_t>(ui->spinBoxOpacityValue->value()));
    }
}

void MainWindow::addTransparencyFlagToSelectedWindow() {
    addTransparencyFlag(selectedWindowHandle);
}

void MainWindow::removeTransparencyFlagFromSelectedWindow() {
    removeTransparencyFlag(selectedWindowHandle);
}

void MainWindow::addClickthroughFlagToSelectedWindow() {
    addClickthroughFlag(selectedWindowHandle);

}

void MainWindow::removeClickthroughFlagFromSelectedWindow() {
    removeClickthroughFlag(selectedWindowHandle);
}

void MainWindow::setModificationControlsEnabled(bool enabled) {
    ui->pushButtonRemoveTransparencyFlag->setEnabled(enabled);
    ui->pushButtonAddTransparencyFlag->setEnabled(enabled);
    ui->pushButtonAddClickthroughFlag->setEnabled(enabled);
    ui->pushButtonRemoveClickthroughFlag->setEnabled(enabled);
    ui->horizontalSliderOpacity->setEnabled(enabled);
    ui->spinBoxOpacityValue->setEnabled(enabled);
}

void MainWindow::on_pushButtonEnableModifications_clicked() {
    if(selectedWindowHandle != nullptr && IsWindow(selectedWindowHandle) && !originalWindowStates.contains(selectedWindowHandle)) {
        WindowState original_state;
        ZeroMemory(&original_state, sizeof(original_state));

        original_state.ExStyle = GetWindowLong(selectedWindowHandle, GWL_EXSTYLE);

        if(hasTransparencyFlag(selectedWindowHandle)) {
            original_state.LWAttributes = getLWAttributes(selectedWindowHandle);
        }

        originalWindowStates[selectedWindowHandle] = original_state;

        setModificationControlsEnabled(true);
    } else if(originalWindowStates.contains(selectedWindowHandle)) {
        setModificationControlsEnabled(true);
    }
}

void MainWindow::on_spinBoxOpacityValue_valueChanged(int new_value) {
    ui->horizontalSliderOpacity->setValue(new_value);
    setSelectedWindowAlphaToSliderValue();
}

void MainWindow::on_horizontalSliderOpacity_valueChanged(int new_value) {
    ui->spinBoxOpacityValue->setValue(new_value);
    setSelectedWindowAlphaToSliderValue();
}

void MainWindow::on_pushButtonSelectWindow_clicked() {
    if(processScannerDialog == nullptr) {
        processScannerDialog = new ProcessScannerDialog { this, ProcessScanner::SCAN_SCOPE::WINDOW_MODE };
        processScannerDialog->show();

        ui->pushButtonSelectWindow->setEnabled(false);

        connect(processScannerDialog, &ProcessScannerDialog::treeSelectionMade, [this](QString window_title, HWND window_handle) -> void {
            delete processScannerDialog;
            processScannerDialog = nullptr;

            ui->pushButtonSelectWindow->setEnabled(true);

            if(IsWindow(window_handle)) {
                ui->lineEditSelectedWindowTitle->setText(window_title);
                selectedWindowHandle = window_handle;

                setModificationControlsEnabled(originalWindowStates.contains(window_handle));

                if(hasTransparencyFlag(window_handle)) {
                    LWA lw_attributes { getLWAttributes(window_handle) };

                    if(lw_attributes.Flags & LWA_ALPHA) {
                        ui->horizontalSliderOpacity->setValue(lw_attributes.Alpha);
                    }
                }
            } else {
                ui->lineEditSelectedWindowTitle->clear();
                selectedWindowHandle = nullptr;
            }
        });

        connect(processScannerDialog, &ProcessScannerDialog::destroyed, [this]() -> void {
            ui->pushButtonSelectWindow->setEnabled(true);
            processScannerDialog = nullptr;
        });
    }
}

void MainWindow::restoreOriginalWindowStates() {
    for(const HWND& window_handle : originalWindowStates.keys()) {
        const WindowState& original_state { originalWindowStates[window_handle] };

        if(IsWindow(window_handle)) {
            SetWindowLong(window_handle, GWL_EXSTYLE, original_state.ExStyle);

            if(original_state.ExStyle & WS_EX_LAYERED) {
                const LWA& lwa { original_state.LWAttributes };
                SetLayeredWindowAttributes(window_handle, lwa.ColorRef, lwa.Alpha, lwa.Flags);
            }
        }
    }
}

void MainWindow::on_pushButtonRestoreWindows_clicked() {
    restoreOriginalWindowStates();
}

qsizetype MainWindow::LoadAndApplyStylesheet(const QString& style_sheet_file_path) {
    QFileInfo style_sheet_file_info { style_sheet_file_path };

    if(style_sheet_file_info.exists() && style_sheet_file_info.isFile()) {
        QFile style_sheet_file_stream { style_sheet_file_path };

        if(style_sheet_file_stream.open(QFile::ReadOnly)) {
            const QByteArray& style_sheet_data { style_sheet_file_stream.readAll() };
            style_sheet_file_stream.close();

            if(style_sheet_data.size()) {
                QFileInfo resource_file_info { QString {"%1/%2.rcc" }.arg(style_sheet_file_info.path(), style_sheet_file_info.completeBaseName()) };

                if(resource_file_info.exists() && resource_file_info.isFile()) {
                    QResource::registerResource(resource_file_info.absoluteFilePath());
                }

                setStyleSheet(QString::fromLocal8Bit(style_sheet_data));
            }

            return style_sheet_data.size();
        }
    }

    return 0;
}

MainWindow::MainWindow(QWidget* parent)
    :
      QMainWindow             { parent                },
      ui                      { new Ui::MainWindow    },
      styleSheetFilePath      { "./styles/indigo.qss" },
      processScannerDialog    { nullptr               },
      selectedWindowHandle    { nullptr               }
{
    ui->setupUi(this);

    setWindowFlags(
                Qt::Dialog
                | Qt::CustomizeWindowHint
                | Qt::WindowTitleHint
                | Qt::WindowCloseButtonHint
                | Qt::WindowMinimizeButtonHint
                | Qt::WindowMaximizeButtonHint
                );

    LoadAndApplyStylesheet(styleSheetFilePath);

    ui->centralwidget->layout()->setAlignment(Qt::AlignTop);
    setMaximumHeight(height());

    setModificationControlsEnabled(false);

    connect(ui->pushButtonRestoreWindows,         SIGNAL(clicked()),
            this,                                 SLOT(restoreOriginalWindowStates()));

    connect(ui->pushButtonAddTransparencyFlag,    SIGNAL(clicked()),
            this,                                 SLOT(addTransparencyFlagToSelectedWindow()));

    connect(ui->pushButtonRemoveTransparencyFlag, SIGNAL(clicked()),
            this,                                 SLOT(removeTransparencyFlagFromSelectedWindow()));

    connect(ui->pushButtonAddClickthroughFlag,    SIGNAL(clicked()),
            this,                                 SLOT(addClickthroughFlagToSelectedWindow()));

    connect(ui->pushButtonRemoveClickthroughFlag, SIGNAL(clicked()),
            this,                                 SLOT(removeClickthroughFlagFromSelectedWindow()));
}

MainWindow::~MainWindow() {
    restoreOriginalWindowStates();
    delete ui;
}

