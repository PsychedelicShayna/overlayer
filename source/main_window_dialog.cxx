#include "main_window_dialog.hxx"
#include "ui_main_window_dialog.h"

bool MainWindow::nativeEvent(const QByteArray& event_type, void* message, qintptr* result) {
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
            delete ui->listWidgetActiveWindows->takeItem(ui->listWidgetActiveWindows->row(list_widget_window_item));
        }
    }

    for(qint32 i { 0 }; i < ui->listWidgetInactiveWindows->count(); ++i) {
        ListWidgetWindowItem* list_widget_window_item { dynamic_cast<ListWidgetWindowItem*>(ui->listWidgetInactiveWindows->item(i)) };

        if(!list_widget_window_item->IsValidWindow()) {
            delete ui->listWidgetInactiveWindows->takeItem(ui->listWidgetInactiveWindows->row(list_widget_window_item));
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

void MainWindow::registerClickthroughHotkey(HotkeyRecorderWidget::Hotkey recorded_hotkey) {
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
        processScannerDialog->setWindowFlags(processScannerDialog->windowFlags() & ~Qt::CustomizeWindowHint);
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

void MainWindow::selectedWindows_Delete() {
    for(QListWidgetItem* list_widget_item : ui->listWidgetInactiveWindows->selectedItems()) {
        delete ui->listWidgetInactiveWindows->takeItem(ui->listWidgetInactiveWindows->row(list_widget_item));
    }

    for(QListWidgetItem* list_widget_item : ui->listWidgetActiveWindows->selectedItems()) {
        delete ui->listWidgetActiveWindows->takeItem(ui->listWidgetActiveWindows->row(list_widget_item));
    }
}

void MainWindow::startWindowGrabber() {
    if(timerWindowGrabber->isActive()) {
        windowGrabAttemptCounter = 30;
        return;
    } else {
        windowGrabAttemptCounter = 0;

        const HWND&       initial_foregound_window    { GetForegroundWindow()            };
        const QString&    original_button_text        { ui->pushButtonGrabWindow->text() };

        const auto& timer_timeout_lambda {
            [=]() -> void {
                if(windowGrabAttemptCounter < 30) {
                    const HWND& current_foreground_window { GetForegroundWindow() };
                    ui->pushButtonGrabWindow->setText("Switch To Target Window..");

                    if(current_foreground_window != initial_foregound_window) {
                        addWindowToInactiveList(current_foreground_window);
                    } else {
                        ++windowGrabAttemptCounter;
                        return;
                    }
                }

                ui->pushButtonGrabWindow->setText(original_button_text);
                timerWindowGrabber->stop();
            }
        };

        connect(timerWindowGrabber, &QTimer::timeout, timer_timeout_lambda);

        timer_timeout_lambda();
        timerWindowGrabber->start(300);
    }
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
      clickthroughToggleStateEnabled               { false                           },
      timerWindowGrabber                           { new QTimer { this }             }
{
    ui->setupUi(this);


    horizontalLayoutClickthroughHotkeyWidgets->addWidget(hotkeyRecorderWidgetClickthrough);
    horizontalLayoutClickthroughHotkeyWidgets->addWidget(checkBoxEnableClickthrough);
    ui->verticalLayoutClickthroughWidgets->addLayout(horizontalLayoutClickthroughHotkeyWidgets);

    checkBoxEnableClickthrough->setText("Enable Hotkey");
    hotkeyRecorderWidgetClickthrough->StartRecording();

    LoadAndApplyStylesheet(styleSheetFilePath);

    ui->centralwidget->layout()->setAlignment(Qt::AlignTop);

    timerRemoveInvalidWindows->start(1000);

    QShortcut* inactive_list_widget_del_shortcut { new QShortcut { Qt::Key_Delete, ui->listWidgetInactiveWindows } };
    QShortcut* active_list_widget_del_shortcut   { new QShortcut { Qt::Key_Delete, ui->listWidgetActiveWindows   } };

    connect(inactive_list_widget_del_shortcut,    &QShortcut::activated,
            this,                                 &MainWindow::selectedWindows_Delete);

    connect(active_list_widget_del_shortcut,      &QShortcut::activated,
            this,                                 &MainWindow::selectedWindows_Delete);

    connect(inactive_list_widget_del_shortcut,    &QShortcut::activatedAmbiguously,
            this,                                 &MainWindow::selectedWindows_Delete);

    connect(active_list_widget_del_shortcut,      &QShortcut::activatedAmbiguously,
            this,                                 &MainWindow::selectedWindows_Delete);

    connect(ui->pushButtonDeleteSelectedWindows,  &QPushButton::clicked,
            this,                                 &MainWindow::selectedWindows_Delete);

    connect(timerRemoveInvalidWindows,            &QTimer::timeout,
            this,                                 &MainWindow::removeInvalidWindowsFromLists);

    connect(ui->pushButtonSelectWindow,           &QPushButton::clicked,
            this,                                 &MainWindow::spawnProcessScannerDialog);

    connect(ui->pushButtonGrabWindow,             &QPushButton::clicked,
            this,                                 &MainWindow::startWindowGrabber);

    connect(ui->pushButtonMakeActive,             &QPushButton::clicked,
            this,                                 &MainWindow::selectedInactiveWindows_Activate);

    connect(ui->pushButtonMakeInactive,           &QPushButton::clicked,
            this,                                 &MainWindow::selectedActiveWindows_Deactivate);

    connect(ui->pushButtonResetSelectedWindows,   &QPushButton::clicked,
            this,                                 &MainWindow::selectedActiveWindows_ResetModifications);

    connect(ui->pushButtonEnableTopmost,          &QPushButton::clicked,
            this,                                 &MainWindow::selectedActiveWindows_EnableTopmost);

    connect(ui->pushButtonDisableTopmost,         &QPushButton::clicked,
            this,                                 &MainWindow::selectedActiveWindows_DisableTopmost);

    connect(ui->pushButtonEnableClickthrough,     &QPushButton::clicked,
            this,                                 &MainWindow::selectedActiveWindows_EnableClickthrough);

    connect(ui->pushButtonDisableClickthrough,    &QPushButton::clicked,
            this,                                 &MainWindow::selectedActiveWindows_DisableClickthrough);

    connect(this,                                 &MainWindow::clickthroughToggleHotkeyPressed,
            this,                                 &MainWindow::selectedActiveWindows_ToggleClickthrough);

    connect(hotkeyRecorderWidgetClickthrough,     &HotkeyRecorderWidget::HotkeyRecorded,
            this,                                 &MainWindow::registerClickthroughHotkey);

    connect(checkBoxEnableClickthrough,           &QCheckBox::stateChanged,
            this,                                 &MainWindow::selectedActiveWindows_SetClickthroughToggleHotkeyEnabled);

    connect(ui->pushButtonEnableTransparency,     &QPushButton::clicked,
            this,                                 &MainWindow::selectedActiveWindows_EnableTransparency);

    connect(ui->pushButtonDisableTransparency,    &QPushButton::clicked,
            this,                                 &MainWindow::selectedActiveWindows_DisableTransparency);

    connect(ui->listWidgetActiveWindows,          &QListWidget::itemSelectionChanged,
            this,                                 &MainWindow::selectedActiveWindows_WriteModifiedStateToWidgets);

}

MainWindow::~MainWindow() {
    for(qint32 i { 0 }; i < ui->listWidgetInactiveWindows->count(); ++i) {
        dynamic_cast<ListWidgetWindowItem*>(ui->listWidgetInactiveWindows->item(i))->ApplyOriginalState();
    }

    for(qint32 i { 0 }; i < ui->listWidgetActiveWindows->count(); ++i) {
        dynamic_cast<ListWidgetWindowItem*>(ui->listWidgetActiveWindows->item(i))->ApplyOriginalState();
    }

    delete ui;
}
