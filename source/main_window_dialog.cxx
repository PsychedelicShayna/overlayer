#include "main_window_dialog.hxx"
#include "ui_main_window_dialog.h"

bool MainWindow::nativeEvent(const QByteArray& event_type, void* message, qintptr* result) {
    Q_UNUSED(event_type)
    Q_UNUSED(result)

    MSG* msg { reinterpret_cast<MSG*>(message) };

    if (msg->message == WM_HOTKEY) {
        if(msg->wParam == clickthroughHotkeyId && ((msg->lParam >> 16) & 0xFF) == clickthroughHotkeyVkid) {
            emit clickthroughToggleHotkeyPressed();
            return true;
        }
    }

    return QMainWindow::nativeEvent(event_type, message, result);
}

void MainWindow::removeInvalidWindowsFromLists() {
    for(qint32 i { 0 }; i < ui->listWidgetActiveWindows->count(); ++i) {
        ListWidgetWindowItem* list_widget_window_item { dynamic_cast<ListWidgetWindowItem*>(ui->listWidgetActiveWindows->item(i)) };

        if(!list_widget_window_item->IsValidWindow()) {
            delete list_widget_window_item;
        }
    }

    for(qint32 i { 0 }; i < ui->listWidgetInactiveWindows->count(); ++i) {
        ListWidgetWindowItem* list_widget_window_item { dynamic_cast<ListWidgetWindowItem*>(ui->listWidgetInactiveWindows->item(i)) };

        if(!list_widget_window_item->IsValidWindow()) {
            delete list_widget_window_item;
        }
    }
}

void MainWindow::addWindowToInactiveList(const HWND& window_handle) {
    if(IsWindow(window_handle)) {
        for(qint32 i { 0 }; i < ui->listWidgetInactiveWindows->count(); ++i) {
            ListWidgetWindowItem* list_widget_window_item { dynamic_cast<ListWidgetWindowItem*>(ui->listWidgetInactiveWindows->item(i)) };

            if(list_widget_window_item->WindowHandle == window_handle) {
                return;
            }
        }

        for(qint32 i { 0 }; i < ui->listWidgetActiveWindows->count(); ++i) {
            ListWidgetWindowItem* list_widget_window_item { dynamic_cast<ListWidgetWindowItem*>(ui->listWidgetActiveWindows->item(i)) };

            if(list_widget_window_item->WindowHandle == window_handle) {
                return;
            }
        }

        wchar_t window_title_buffer[0xFF];
        GetWindowText(window_handle, window_title_buffer, sizeof(window_title_buffer));

        ListWidgetWindowItem* new_list_widget_window_item { new ListWidgetWindowItem { window_handle } };
        new_list_widget_window_item->setText(QString::fromWCharArray(window_title_buffer));

        ui->listWidgetInactiveWindows->addItem(new_list_widget_window_item);
    }
}

void MainWindow::registerClickthroughHotkey(HotkeyRecorderWidget::WindowsHotkey recorded_hotkey) {
    unregisterClickthroughHotkey();

    RegisterHotKey(HWND(winId()), clickthroughHotkeyId, MOD_NOREPEAT | recorded_hotkey.Modifiers, recorded_hotkey.Vkid);
    clickthroughHotkeyVkid = recorded_hotkey.Vkid;
}

void MainWindow::unregisterClickthroughHotkey() {
    UnregisterHotKey(HWND(winId()), clickthroughHotkeyId);
}

void MainWindow::spawnProcessScannerDialog() {
    if(processScannerDialog == nullptr) {
        processScannerDialog = new ProcessScannerDialog { this, ProcessScanner::SCAN_SCOPE::WINDOW_MODE };
        processScannerDialog->show();

        ui->pushButtonSelectWindow->setEnabled(false);

        connect(processScannerDialog, &ProcessScannerDialog::treeSelectionMade, [this](QString window_title, HWND window_handle) -> void {
            delete processScannerDialog;
            processScannerDialog = nullptr;

            addWindowToInactiveList(window_handle);
        });

        connect(processScannerDialog, &ProcessScannerDialog::destroyed, [this]() -> void {
            ui->pushButtonSelectWindow->setEnabled(true);
            processScannerDialog = nullptr;
        });
    }
}

void MainWindow::startWindowGrabber() {
    const HWND& initial_foregound_window { GetForegroundWindow() };

    QTimer* timer_window_grabber  { new QTimer { this } };
    qint32* timer_attempt_counter { new qint32          };

    QString* original_button_text { new QString { ui->pushButtonGrabWindow->text() } };

    *timer_attempt_counter = 0;

    const auto& timer_timeout_lambda {
        [=]() -> void {
            if(*timer_attempt_counter < 30) {
                const HWND& current_foreground_window { GetForegroundWindow() };
                ui->pushButtonGrabWindow->setText("Switch To Target Window..");

                if(current_foreground_window != initial_foregound_window) {
                    addWindowToInactiveList(current_foreground_window);
                } else {
                    (*timer_attempt_counter)++;
                    return;
                }
            }

            ui->pushButtonGrabWindow->setText(*original_button_text);

            delete timer_attempt_counter;
            delete original_button_text;

            timer_window_grabber->stop();
            timer_window_grabber->deleteLater();
        }
    };

    connect(timer_window_grabber, &QTimer::timeout, timer_timeout_lambda);

    timer_timeout_lambda();
    timer_window_grabber->start(300);
}

void MainWindow::selectedInactiveWindows_Activate() {
    for(QListWidgetItem* selected_item : ui->listWidgetInactiveWindows->selectedItems()) {
        ListWidgetWindowItem* selected_list_widget_window_item { reinterpret_cast<ListWidgetWindowItem*>(selected_item) };
        selected_list_widget_window_item->ModifiedState.EnableLayering();
        selected_list_widget_window_item->ApplyModifiedState();

        ui->listWidgetActiveWindows->addItem(
                    ui->listWidgetInactiveWindows->takeItem(
                        ui->listWidgetInactiveWindows->row(selected_item)));
    }
}

void MainWindow::selectedActiveWindows_Deactivate() {
    for(QListWidgetItem* selected_item : ui->listWidgetActiveWindows->selectedItems()) {
        ListWidgetWindowItem* selected_list_widget_window_item { reinterpret_cast<ListWidgetWindowItem*>(selected_item) };
        selected_list_widget_window_item->ApplyOriginalState();

        ui->listWidgetInactiveWindows->addItem(
                    ui->listWidgetActiveWindows->takeItem(
                        ui->listWidgetActiveWindows->row(selected_item)));
    }
}

void MainWindow::selectedActiveWindows_ResetModifications() {
    for(auto& list_widget_item : ui->listWidgetActiveWindows->selectedItems()) {
        ListWidgetWindowItem* list_widget_window_item { dynamic_cast<ListWidgetWindowItem*>(list_widget_item) };
        list_widget_window_item->ResetModifications();
    }
}

void MainWindow::selectedActiveWindows_EnableClickthrough() {
    for(auto& list_widget_item : ui->listWidgetActiveWindows->selectedItems()) {
        ListWidgetWindowItem* list_widget_window_item { dynamic_cast<ListWidgetWindowItem*>(list_widget_item) };
        list_widget_window_item->ModifiedState.EnableClickthrough();
        list_widget_window_item->ApplyModifiedState();
    }
}

void MainWindow::selectedActiveWindows_DisableClickthrough() {
    for(auto& list_widget_item : ui->listWidgetActiveWindows->selectedItems()) {
        ListWidgetWindowItem* list_widget_window_item { dynamic_cast<ListWidgetWindowItem*>(list_widget_item) };
        list_widget_window_item->ModifiedState.DisableClickthrough();
        list_widget_window_item->ApplyModifiedState();
    }
}

void MainWindow::selectedActiveWindows_ToggleClickthrough() {
    for(qsizetype i { 0 }; i < ui->listWidgetActiveWindows->count(); ++i) {
        ListWidgetWindowItem* list_widget_window_item { dynamic_cast<ListWidgetWindowItem*>(ui->listWidgetActiveWindows->item(i)) };

        if(list_widget_window_item->RespondToHotkey) {
            if(clickthroughToggleStateEnabled) {
                list_widget_window_item->ModifiedState.DisableClickthrough();
            } else {
                list_widget_window_item->ModifiedState.EnableClickthrough();
            }

            list_widget_window_item->ApplyModifiedState();
        }
    }

    clickthroughToggleStateEnabled ^= true;
}

void MainWindow::selectedActiveWindows_SetClickthroughToggleHotkeyEnabled(qint32 enabled) {
   for(auto& list_widget_item : ui->listWidgetActiveWindows->selectedItems()) {
        ListWidgetWindowItem* list_widget_window_item { dynamic_cast<ListWidgetWindowItem*>(list_widget_item) };
        list_widget_window_item->RespondToHotkey = enabled;
    }
}

void MainWindow::selectedActiveWindows_EnableTopmost() {
    for(auto& list_widget_item : ui->listWidgetActiveWindows->selectedItems()) {
        ListWidgetWindowItem* list_widget_window_item { dynamic_cast<ListWidgetWindowItem*>(list_widget_item) };
        list_widget_window_item->ModifiedState.EnableTopmost();
        list_widget_window_item->ApplyModifiedState();
    }
}

void MainWindow::selectedActiveWindows_DisableTopmost() {
    for(auto& list_widget_item : ui->listWidgetActiveWindows->selectedItems()) {
        ListWidgetWindowItem* list_widget_window_item { dynamic_cast<ListWidgetWindowItem*>(list_widget_item) };
        list_widget_window_item->ModifiedState.DisableTopmost();
        list_widget_window_item->ApplyModifiedState();
    }
}

void MainWindow::selectedActiveWindows_EnableTransparency() {
    for(auto& list_widget_item : ui->listWidgetActiveWindows->selectedItems()) {
        ListWidgetWindowItem* list_widget_window_item { dynamic_cast<ListWidgetWindowItem*>(list_widget_item) };
        list_widget_window_item->ModifiedState.EnableAlphaTransparencyMode();
        list_widget_window_item->ApplyModifiedState();
    }
}

void MainWindow::selectedActiveWindows_DisableTransparency() {
    for(auto& list_widget_item : ui->listWidgetActiveWindows->selectedItems()) {
        ListWidgetWindowItem* list_widget_window_item { dynamic_cast<ListWidgetWindowItem*>(list_widget_item) };
        list_widget_window_item->ModifiedState.DisableTransparency();
        list_widget_window_item->ApplyModifiedState();
    }
}

void MainWindow::selectedActiveWindows_WriteSliderAlphaToModifiedState() {
    for(auto& list_widget_item : ui->listWidgetActiveWindows->selectedItems()) {
        ListWidgetWindowItem* list_widget_window_item { dynamic_cast<ListWidgetWindowItem*>(list_widget_item) };
        list_widget_window_item->ModifiedState.LWAttributes.Alpha = static_cast<uint8_t>(ui->spinBoxAlpha->value());
        list_widget_window_item->ApplyModifiedState();
    }
}

void MainWindow::selectedActiveWindows_WriteModifiedStateToWidgets() {
    const QList<QListWidgetItem*> selected_items { ui->listWidgetActiveWindows->selectedItems() };

    if(selected_items.size() == 1) {
        const ListWidgetWindowItem* first_list_widget_window_item { reinterpret_cast<ListWidgetWindowItem*>(selected_items.first()) };
        ui->spinBoxAlpha->setValue(first_list_widget_window_item->ModifiedState.LWAttributes.Alpha);
        checkBoxEnableClickthrough->setChecked(first_list_widget_window_item->RespondToHotkey);
    }
}

void MainWindow::on_spinBoxAlpha_valueChanged(int new_value) {
    ui->horizontalSliderAlpha->setValue(new_value);
    selectedActiveWindows_WriteSliderAlphaToModifiedState();
}

void MainWindow::on_horizontalSliderAlpha_valueChanged(int new_value) {
    ui->spinBoxAlpha->setValue(new_value);
    selectedActiveWindows_WriteSliderAlphaToModifiedState();
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
      QMainWindow                                  { parent                          },
      ui                                           { new Ui::MainWindow              },
      styleSheetFilePath                           { "./styles/indigo.qss"           },
      processScannerDialog                         { nullptr                         },
      timerRemoveInvalidWindows                    { new QTimer      { this }        },
      horizontalLayoutClickthroughHotkeyWidgets    { new QHBoxLayout { this }        },
      hotkeyRecorderWidgetClickthrough             { new HotkeyRecorderWidget        },
      checkBoxEnableClickthrough                   { new QCheckBox   { this }        },
      clickthroughHotkeyId                         { 0x41414141                      },
      clickthroughToggleStateEnabled               { false                           }
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

    horizontalLayoutClickthroughHotkeyWidgets->addWidget(hotkeyRecorderWidgetClickthrough);
    horizontalLayoutClickthroughHotkeyWidgets->addWidget(checkBoxEnableClickthrough);
    ui->verticalLayoutClickthroughWidgets->addLayout(horizontalLayoutClickthroughHotkeyWidgets);

    checkBoxEnableClickthrough->setText("Enable Hotkey");
    hotkeyRecorderWidgetClickthrough->StartRecording();

    LoadAndApplyStylesheet(styleSheetFilePath);

    ui->centralwidget->layout()->setAlignment(Qt::AlignTop);

    timerRemoveInvalidWindows->start(1000);

    connect(timerRemoveInvalidWindows,            SIGNAL(timeout()),
            this,                                 SLOT(removeInvalidWindowsFromLists()));

    connect(ui->pushButtonSelectWindow,           SIGNAL(clicked()),
            this,                                 SLOT(spawnProcessScannerDialog()));

    connect(ui->pushButtonGrabWindow,             SIGNAL(clicked()),
            this,                                 SLOT(startWindowGrabber()));

    connect(ui->pushButtonMakeActive,             SIGNAL(clicked()),
            this,                                 SLOT(selectedInactiveWindows_Activate()));

    connect(ui->pushButtonMakeInactive,           SIGNAL(clicked()),
            this,                                 SLOT(selectedActiveWindows_Deactivate()));

    connect(ui->pushButtonResetSelectedWindows,   SIGNAL(clicked()),
            this,                                 SLOT(selectedActiveWindows_ResetModifications()));

    connect(ui->pushButtonEnableTopmost,          SIGNAL(clicked()),
            this,                                 SLOT(selectedActiveWindows_EnableTopmost()));

    connect(ui->pushButtonDisableTopmost,         SIGNAL(clicked()),
            this,                                 SLOT(selectedActiveWindows_DisableTopmost()));

    connect(ui->pushButtonEnableClickthrough,     SIGNAL(clicked()),
            this,                                 SLOT(selectedActiveWindows_EnableClickthrough()));

    connect(ui->pushButtonDisableClickthrough,    SIGNAL(clicked()),
            this,                                 SLOT(selectedActiveWindows_DisableClickthrough()));

    connect(this,                                 SIGNAL(clickthroughToggleHotkeyPressed()),
            this,                                 SLOT(selectedActiveWindows_ToggleClickthrough()));

    connect(hotkeyRecorderWidgetClickthrough,     SIGNAL(HotkeyRecorded(HotkeyRecorderWidget::WindowsHotkey)),
            this,                                 SLOT(registerClickthroughHotkey(HotkeyRecorderWidget::WindowsHotkey)));

    connect(checkBoxEnableClickthrough,           SIGNAL(stateChanged(int)),
            this,                                 SLOT(selectedActiveWindows_SetClickthroughToggleHotkeyEnabled(qint32)));

    connect(ui->pushButtonEnableTransparency,     SIGNAL(clicked()),
            this,                                 SLOT(selectedActiveWindows_EnableTransparency()));

    connect(ui->pushButtonDisableTransparency,    SIGNAL(clicked()),
            this,                                 SLOT(selectedActiveWindows_DisableTransparency()));

    connect(ui->listWidgetActiveWindows,          SIGNAL(itemSelectionChanged()),
            this,                                 SLOT(selectedActiveWindows_WriteModifiedStateToWidgets()));

}

MainWindow::~MainWindow() {
    delete ui;
}

