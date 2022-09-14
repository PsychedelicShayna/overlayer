#include "main_window_dialog.hxx"
#include "ui_main_window_dialog.h"

bool MainWindow::nativeEvent(const QByteArray& event_type,
                             void*             message,
                             qintptr*          result)
{
    const auto* msg = reinterpret_cast<MSG*>(message);

    if(msg->message == WM_HOTKEY) {
        if(msg->wParam == clickthroughHotkeyId
           && ((msg->lParam >> 16) & 0xFF) == clickthroughHotkeyVkid) {
            emit clickthroughToggleHotkeyPressed();
            return true;
        }
    }

    return QMainWindow::nativeEvent(event_type, message, result);
}

void MainWindow::removeInvalidWindowsFromLists()
{
    for(qint32 i = 0; i < ui->lwActiveWindows->count(); ++i) {
        auto*       item = ui->lwActiveWindows->item(i);
        const auto* lwwi = dynamic_cast<ListWidgetWindowItem*>(item);

        if(!lwwi->IsValidWindow()) {
            const auto& row = ui->lwActiveWindows->row(lwwi);
            delete ui->lwActiveWindows->takeItem(row);
        }
    }

    for(qint32 i = 0; i < ui->lwInactiveWindows->count(); ++i) {
        auto*       item = ui->lwInactiveWindows->item(i);
        const auto* lwwi = dynamic_cast<ListWidgetWindowItem*>(item);

        if(!lwwi->IsValidWindow()) {
            const auto& row = ui->lwInactiveWindows->row(lwwi);
            delete ui->lwInactiveWindows->takeItem(row);
        }
    }
}

void MainWindow::addWindowToInactiveList(const HWND& window_handle)
{
    if(IsWindow(window_handle)) {
        // Check if the window is already in the inactive window list, and
        // return early / ignore if it is.
        for(quint32 i = 0; i < ui->lwInactiveWindows->count(); ++i) {
            const auto* item = ui->lwInactiveWindows->item(i);
            const auto* lwwi = dynamic_cast<const ListWidgetWindowItem*>(item);

            if(lwwi->WindowHandle == window_handle) {
                return;
            }
        }

        // Check if the window is already in the active window list, and
        // return early / ignore if it is.
        for(quint32 i = 0; i < ui->lwActiveWindows->count(); ++i) {
            const auto* item = ui->lwActiveWindows->item(i);
            const auto* lwwi = dynamic_cast<const ListWidgetWindowItem*>(item);

            if(lwwi->WindowHandle == window_handle) {
                return;
            }
        }

        wchar_t window_title_buffer[1024];

        GetWindowText(
            window_handle, window_title_buffer, sizeof(window_title_buffer));

        auto* new_lwwi = new ListWidgetWindowItem {window_handle};

        new_lwwi->setText(QString::fromWCharArray(window_title_buffer));
        ui->lwInactiveWindows->addItem(new_lwwi);
    }
}

void MainWindow::registerClickthroughHotkey(
    HotkeyRecorderWidget::Hotkey recorded_hotkey)
{
    unregisterClickthroughHotkey();

    RegisterHotKey(HWND(winId()),
                   clickthroughHotkeyId,
                   MOD_NOREPEAT | recorded_hotkey.Modifiers,
                   recorded_hotkey.Vkid);

    clickthroughHotkeyVkid = recorded_hotkey.Vkid;
}

void MainWindow::unregisterClickthroughHotkey()
{
    UnregisterHotKey(HWND(winId()), clickthroughHotkeyId);
}

void MainWindow::spawnProcessScannerDialog()
{
    if(processScannerDialog == nullptr) {
        processScannerDialog = new ProcessScannerDialog {
            this, ProcessScanner::SCAN_SCOPE::WINDOW_MODE};

        processScannerDialog->setWindowFlags(processScannerDialog->windowFlags()
                                             & ~Qt::CustomizeWindowHint);

        processScannerDialog->show();

        ui->pbSelectWindow->setEnabled(false);

        connect(processScannerDialog,
                &ProcessScannerDialog::treeSelectionMade,
                [this](QString window_title, HWND window_handle) -> void {
                    delete processScannerDialog;
                    processScannerDialog = nullptr;

                    addWindowToInactiveList(window_handle);
                });

        connect(processScannerDialog,
                &ProcessScannerDialog::destroyed,
                [this]() -> void {
                    ui->pbSelectWindow->setEnabled(true);
                    processScannerDialog = nullptr;
                });
    }
}

void MainWindow::selectedWindows_Delete()
{
    for(auto item : ui->lwInactiveWindows->selectedItems()) {
        const int& item_row = ui->lwInactiveWindows->row(item);
        delete ui->lwInactiveWindows->takeItem(item_row);
    }

    for(auto item : ui->lwActiveWindows->selectedItems()) {
        const int& item_row = ui->lwActiveWindows->row(item);
        delete ui->lwActiveWindows->takeItem(item_row);
    }
}

void MainWindow::startWindowGrabber()
{
    if(timerWindowGrabber->isActive()) {
        windowGrabAttempts = 30;
        return;
    } else {
        windowGrabAttempts = 0;

        const HWND&    initial_foregound_window = GetForegroundWindow();
        const QString& original_button_text     = ui->pbGrabWindow->text();

        const auto& timer_timeout_lambda = [=]() -> void {
            if(windowGrabAttempts < 30) {
                const HWND& current_foreground_window = GetForegroundWindow();

                ui->pbGrabWindow->setText("Switch To Target Window..");

                if(current_foreground_window != initial_foregound_window) {
                    addWindowToInactiveList(current_foreground_window);
                } else {
                    ++windowGrabAttempts;
                    return;
                }
            }

            ui->pbGrabWindow->setText(original_button_text);
            timerWindowGrabber->stop();
        };

        connect(timerWindowGrabber, &QTimer::timeout, timer_timeout_lambda);

        timer_timeout_lambda();
        timerWindowGrabber->start(300);
    }
}

void MainWindow::selectedInactiveWindows_Activate()
{
    for(auto& item : ui->lwInactiveWindows->selectedItems()) {
        auto* lwwi = reinterpret_cast<ListWidgetWindowItem*>(item);

        lwwi->ModifiedState.EnableLayering();
        lwwi->ApplyModifiedState();

        ui->lwActiveWindows->addItem(
            ui->lwInactiveWindows->takeItem(ui->lwInactiveWindows->row(item)));
    }
}

void MainWindow::selectedActiveWindows_Deactivate()
{
    for(auto& item : ui->lwActiveWindows->selectedItems()) {
        auto* lwwi = reinterpret_cast<ListWidgetWindowItem*>(item);

        lwwi->ApplyOriginalState();

        ui->lwInactiveWindows->addItem(
            ui->lwActiveWindows->takeItem(ui->lwActiveWindows->row(item)));
    }
}

void MainWindow::selectedActiveWindows_ResetModifications()
{
    for(auto& item : ui->lwActiveWindows->selectedItems()) {
        auto* lwwi = dynamic_cast<ListWidgetWindowItem*>(item);
        lwwi->ResetModifications();
    }
}

void MainWindow::selectedActiveWindows_EnableClickthrough()
{
    for(auto& item : ui->lwActiveWindows->selectedItems()) {
        auto* lwwi = dynamic_cast<ListWidgetWindowItem*>(item);

        lwwi->ModifiedState.EnableClickthrough();
        lwwi->ApplyModifiedState();
    }
}

void MainWindow::selectedActiveWindows_DisableClickthrough()
{
    for(auto& item : ui->lwActiveWindows->selectedItems()) {
        auto* lwwi = dynamic_cast<ListWidgetWindowItem*>(item);

        lwwi->ModifiedState.DisableClickthrough();
        lwwi->ApplyModifiedState();
    }
}

void MainWindow::selectedActiveWindows_ToggleClickthrough()
{
    for(auto& item : ui->lwActiveWindows->selectedItems()) {
        auto* lwwi = dynamic_cast<ListWidgetWindowItem*>(item);

        if(lwwi->RespondToHotkey) {
            if(clickthroughToggle) {
                lwwi->ModifiedState.DisableClickthrough();
            } else {
                lwwi->ModifiedState.EnableClickthrough();
            }

            lwwi->ApplyModifiedState();
        }
    }

    clickthroughToggle ^= true;
}

void MainWindow::selectedActiveWin_setClickthroughHotkeyEnabled(qint32 enabled)
{
    for(auto& item : ui->lwActiveWindows->selectedItems()) {
        auto* lwwi            = dynamic_cast<ListWidgetWindowItem*>(item);
        lwwi->RespondToHotkey = enabled;
    }
}

void MainWindow::selectedActiveWindows_EnableTopmost()
{
    for(auto& item : ui->lwActiveWindows->selectedItems()) {
        auto* lwwi = dynamic_cast<ListWidgetWindowItem*>(item);

        lwwi->ModifiedState.EnableTopmost();
        lwwi->ApplyModifiedState();
    }
}

void MainWindow::selectedActiveWindows_DisableTopmost()
{
    for(auto& item : ui->lwActiveWindows->selectedItems()) {
        auto* lwwi = dynamic_cast<ListWidgetWindowItem*>(item);

        lwwi->ModifiedState.DisableTopmost();
        lwwi->ApplyModifiedState();
    }
}

void MainWindow::selectedActiveWindows_EnableTransparency()
{
    for(auto& item : ui->lwActiveWindows->selectedItems()) {
        auto* lwwi = dynamic_cast<ListWidgetWindowItem*>(item);

        lwwi->ModifiedState.EnableAlphaTransparencyMode();
        lwwi->ApplyModifiedState();
    }
}

void MainWindow::selectedActiveWindows_DisableTransparency()
{
    for(auto& item : ui->lwActiveWindows->selectedItems()) {
        auto* lwwi = dynamic_cast<ListWidgetWindowItem*>(item);

        lwwi->ModifiedState.DisableTransparency();
        lwwi->ApplyModifiedState();
    }
}

void MainWindow::selectedActiveWindows_WriteSliderAlphaToModifiedState()
{
    for(auto& item : ui->lwActiveWindows->selectedItems()) {
        auto* lwwi = dynamic_cast<ListWidgetWindowItem*>(item);

        lwwi->ModifiedState.LWAttributes.Alpha =
            static_cast<uint8_t>(ui->spinBoxAlpha->value());

        lwwi->ApplyModifiedState();
    }
}

void MainWindow::selectedActiveWindows_WriteModifiedStateToWidgets()
{
    const QList<QListWidgetItem*>& selected_items =
        ui->lwActiveWindows->selectedItems();

    if(selected_items.size() == 1) {
        const auto* first_item =
            dynamic_cast<ListWidgetWindowItem*>(selected_items.first());

        quint8 alpha = first_item->ModifiedState.LWAttributes.Alpha;
        ui->spinBoxAlpha->setValue(alpha);

        cbEnableClickthrough->setChecked(first_item->RespondToHotkey);
    }
}

void MainWindow::on_spinBoxAlpha_valueChanged(int new_value)
{
    ui->hslAlpha->setValue(new_value);
    selectedActiveWindows_WriteSliderAlphaToModifiedState();
}

void MainWindow::on_hslAlpha_valueChanged(int new_value)
{
    ui->spinBoxAlpha->setValue(new_value);
    selectedActiveWindows_WriteSliderAlphaToModifiedState();
}

qsizetype MainWindow::LoadAndApplyStylesheet(
    const QString& style_sheet_file_path)
{
    QFileInfo style_sheet_file_info {style_sheet_file_path};

    if(style_sheet_file_info.exists() && style_sheet_file_info.isFile()) {
        QFile style_sheet_file_stream {style_sheet_file_path};

        if(style_sheet_file_stream.open(QFile::ReadOnly)) {
            const QByteArray& style_sheet_data =
                style_sheet_file_stream.readAll();

            style_sheet_file_stream.close();

            if(style_sheet_data.size()) {
                QString resource_file_path = QString {"%1/%2.rcc"}.arg(
                    style_sheet_file_info.path(),
                    style_sheet_file_info.completeBaseName());

                QFileInfo resource_file_info {resource_file_path};

                if(resource_file_info.exists() && resource_file_info.isFile()) {
                    QResource::registerResource(
                        resource_file_info.absoluteFilePath());
                }

                setStyleSheet(QString::fromLocal8Bit(style_sheet_data));
            }

            return style_sheet_data.size();
        }
    }

    return 0;
}

MainWindow::MainWindow(QWidget* parent)
  : QMainWindow {parent},
    ui {new Ui::MainWindow},
    styleSheetFilePath {"./styles/indigo.qss"},
    processScannerDialog {nullptr},
    timerRemoveInvalidWindows {new QTimer {this}},
    hblClickthroughHotkeyWidgets {new QHBoxLayout {this}},
    hkrClickthrough {new HotkeyRecorderWidget},
    cbEnableClickthrough {new QCheckBox {this}},
    clickthroughHotkeyId {0x41414141},
    clickthroughToggle {false},
    timerWindowGrabber {new QTimer {this}}
{
    ui->setupUi(this);

    hblClickthroughHotkeyWidgets->addWidget(hkrClickthrough);
    hblClickthroughHotkeyWidgets->addWidget(cbEnableClickthrough);

    ui->vblClickthroughWidgets->addLayout(hblClickthroughHotkeyWidgets);

    cbEnableClickthrough->setText("Enable Hotkey");
    hkrClickthrough->StartRecording();

    LoadAndApplyStylesheet(styleSheetFilePath);

    ui->centralwidget->layout()->setAlignment(Qt::AlignTop);

    timerRemoveInvalidWindows->start(1000);

    const auto* shrt_del_inactive =
        new QShortcut {Qt::Key_Delete, ui->lwInactiveWindows};

    const auto* shrt_del_active =
        new QShortcut {Qt::Key_Delete, ui->lwActiveWindows};

    connect(shrt_del_inactive,
            &QShortcut::activated,
            this,
            &MainWindow::selectedWindows_Delete);

    connect(shrt_del_active,
            &QShortcut::activated,
            this,
            &MainWindow::selectedWindows_Delete);

    connect(shrt_del_inactive,
            &QShortcut::activatedAmbiguously,
            this,
            &MainWindow::selectedWindows_Delete);

    connect(shrt_del_active,
            &QShortcut::activatedAmbiguously,
            this,
            &MainWindow::selectedWindows_Delete);

    connect(ui->pbDeleteSelectedWindows,
            &QPushButton::clicked,
            this,
            &MainWindow::selectedWindows_Delete);

    connect(timerRemoveInvalidWindows,
            &QTimer::timeout,
            this,
            &MainWindow::removeInvalidWindowsFromLists);

    connect(ui->pbSelectWindow,
            &QPushButton::clicked,
            this,
            &MainWindow::spawnProcessScannerDialog);

    connect(ui->pbGrabWindow,
            &QPushButton::clicked,
            this,
            &MainWindow::startWindowGrabber);

    connect(ui->pbMakeActive,
            &QPushButton::clicked,
            this,
            &MainWindow::selectedInactiveWindows_Activate);

    connect(ui->pbMakeInactive,
            &QPushButton::clicked,
            this,
            &MainWindow::selectedActiveWindows_Deactivate);

    connect(ui->pbResetSelectedWindows,
            &QPushButton::clicked,
            this,
            &MainWindow::selectedActiveWindows_ResetModifications);

    connect(ui->pbEnableTopmost,
            &QPushButton::clicked,
            this,
            &MainWindow::selectedActiveWindows_EnableTopmost);

    connect(ui->pbDisableTopmost,
            &QPushButton::clicked,
            this,
            &MainWindow::selectedActiveWindows_DisableTopmost);

    connect(ui->pbEnableClickthrough,
            &QPushButton::clicked,
            this,
            &MainWindow::selectedActiveWindows_EnableClickthrough);

    connect(ui->pbDisableClickthrough,
            &QPushButton::clicked,
            this,
            &MainWindow::selectedActiveWindows_DisableClickthrough);

    connect(this,
            &MainWindow::clickthroughToggleHotkeyPressed,
            this,
            &MainWindow::selectedActiveWindows_ToggleClickthrough);

    connect(hkrClickthrough,
            &HotkeyRecorderWidget::HotkeyRecorded,
            this,
            &MainWindow::registerClickthroughHotkey);

    connect(cbEnableClickthrough,
            &QCheckBox::stateChanged,
            this,
            &MainWindow::selectedActiveWin_setClickthroughHotkeyEnabled);

    connect(ui->pbEnableTransparency,
            &QPushButton::clicked,
            this,
            &MainWindow::selectedActiveWindows_EnableTransparency);

    connect(ui->pbDisableTransparency,
            &QPushButton::clicked,
            this,
            &MainWindow::selectedActiveWindows_DisableTransparency);

    connect(ui->lwActiveWindows,
            &QListWidget::itemSelectionChanged,
            this,
            &MainWindow::selectedActiveWindows_WriteModifiedStateToWidgets);
}

MainWindow::~MainWindow()
{
    for(qint32 i = 0; i < ui->lwInactiveWindows->count(); ++i) {
        auto* item = ui->lwInactiveWindows->item(i);
        auto* lwwi = dynamic_cast<ListWidgetWindowItem*>(item);
        lwwi->ApplyOriginalState();
    }

    for(qint32 i = 0; i < ui->lwActiveWindows->count(); ++i) {
        auto* item = ui->lwActiveWindows->item(i);
        auto* lwwi = dynamic_cast<ListWidgetWindowItem*>(item);
        lwwi->ApplyOriginalState();
    }

    delete ui;
}
