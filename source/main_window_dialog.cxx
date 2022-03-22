#include "main_window_dialog.hxx"
#include "ui_main_window_dialog.h"

// Make a timer that periodically gets rid of closed windows from the list via IsWindow

void MainWindow::spawnProcessScannerDialog() {
    if(processScannerDialog == nullptr) {
        processScannerDialog = new ProcessScannerDialog { this, ProcessScanner::SCAN_SCOPE::WINDOW_MODE };
        processScannerDialog->show();

        ui->pushButtonSelectWindow->setEnabled(false);

        connect(processScannerDialog, &ProcessScannerDialog::treeSelectionMade, [this](QString window_title, HWND window_handle) -> void {
            delete processScannerDialog;
            processScannerDialog = nullptr;

            if(IsWindow(window_handle)) {
                ProcessWindowListItem* new_item { new ProcessWindowListItem { window_handle } };
                new_item->setText(window_title);

                ui->listWidgetInactiveWindows->addItem(new_item);
            }

            /*
            ui->pushButtonSelectWindow->setEnabled(true);

            if(IsWindow(window_handle)) {
                // ui->lineEditSelectedWindowTitle->setText(window_title);
                selectedWindowHandle = window_handle;

                setModificationControlsEnabled(originalWindowStates.contains(window_handle));

                if(hasTransparencyFlag(window_handle)) {
                    LWA lw_attributes { getLWAttributes(window_handle) };

                    if(lw_attributes.Flags & LWA_ALPHA) {
                        ui->horizontalSliderAlpha->setValue(lw_attributes.Alpha);
                    }
                }
            } else {
                // ui->lineEditSelectedWindowTitle->clear();
                selectedWindowHandle = nullptr;
            }

            */
        });

        connect(processScannerDialog, &ProcessScannerDialog::destroyed, [this]() -> void {
            ui->pushButtonSelectWindow->setEnabled(true);
            processScannerDialog = nullptr;
        });
    }
}

void MainWindow::startWindowGrabber() {

}

void MainWindow::selectedInactiveWindows_Activate() {
    for(QListWidgetItem* selected_item : ui->listWidgetInactiveWindows->selectedItems()) {
        ProcessWindowListItem* selected_window_item { reinterpret_cast<ProcessWindowListItem*>(selected_item) };
        selected_window_item->ApplyModifiedState();

        ui->listWidgetActiveWindows->addItem(
                    ui->listWidgetInactiveWindows->takeItem(
                        ui->listWidgetInactiveWindows->row(selected_item)));
    }
}

void MainWindow::selectedActiveWindows_Deactivate() {
    for(QListWidgetItem* selected_item : ui->listWidgetActiveWindows->selectedItems()) {
        ProcessWindowListItem* selected_window_item { reinterpret_cast<ProcessWindowListItem*>(selected_item) };
        selected_window_item->ApplyOriginalState();

        ui->listWidgetInactiveWindows->addItem(
                    ui->listWidgetActiveWindows->takeItem(
                        ui->listWidgetActiveWindows->row(selected_item)));
    }
}

void MainWindow::selectedActiveWindows_EnableClickthrough() {

}

void MainWindow::selectedActiveWindows_DisableClickthrough() {

}

void MainWindow::selectedActiveWindows_EnableTransparency() {

}

void MainWindow::selectedActiveWindows_DisableTransparency() {

}

void MainWindow::selectedActiveWindows_EnableTopmost() {

}

void MainWindow::selectedActiveWindows_DisableTopmost() {

}

void MainWindow::selectedActiveWindows_SyncAlphaToSlider() {

}

void MainWindow::on_spinBoxAlpha_valueChanged(int new_value) {
    ui->horizontalSliderAlpha->setValue(new_value);
    selectedActiveWindows_SyncAlphaToSlider();
}

void MainWindow::on_horizontalSliderAlpha_valueChanged(int new_value) {
    ui->spinBoxAlpha->setValue(new_value);
    selectedActiveWindows_SyncAlphaToSlider();
}

qsizetype MainWindow::LoadAndApplyStylesheet(const QString& style_sheet_file_path) {
    QFileInfo style_sheet_file_info { style_sheet_file_path };

    if(style_sheet_file_info.exists() && style_sheet_file_info.isFile()) {
        QFile style_sheet_file_stream { style_sheet_file_path };

        if(style_sheet_file_stream.open(QFile::ReadOnly)) {
            const QByteArray& style_sheet_data { style_sheet_file_stream.readAll() };
            style_sheet_file_stream.close();

            if(style_sheet_data.size()) {
                QFileInfo resource_file_info { QString { "%1/%2.rcc" }.arg(style_sheet_file_info.path(), style_sheet_file_info.completeBaseName()) };

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
      processScannerDialog    { nullptr               }
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

    connect(ui->pushButtonMakeActive,   SIGNAL(clicked()),
            this,                       SLOT(selectedInactiveWindows_Activate()));

    connect(ui->pushButtonMakeInactive, SIGNAL(clicked()),
            this,                       SLOT(selectedActiveWindows_Deactivate()));


    // setMaximumHeight(height());

    // setModificationControlsEnabled(false);

    connect(ui->pushButtonSelectWindow,           SIGNAL(clicked()),
            this,                                 SLOT(spawnProcessScannerDialog()));

}

MainWindow::~MainWindow() {
    // restoreOriginalWindowStates();
    delete ui;
}

