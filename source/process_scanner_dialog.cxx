#include "process_scanner_dialog.hxx"
#include "ui_process_scanner_dialog.h"


QList<QTreeWidgetItem*> ProcessScannerDialog::collectTreeRoot(QTreeWidget* tree) {
    QList<QTreeWidgetItem*> root_items_list;
    root_items_list.reserve(tree->topLevelItemCount());

    for(int32_t i { 0 }; i < tree->topLevelItemCount(); ++i) {
        root_items_list.append(tree->topLevelItem(i));
    }

    return root_items_list;
}

QList<QTreeWidgetItem*> ProcessScannerDialog::collectTreeChildren(QTreeWidgetItem* root_item) {
    QList<QTreeWidgetItem*> child_items_list;
    child_items_list.reserve(root_item->childCount());

    for(int32_t i { 0 }; i < root_item->childCount(); ++i) {
        child_items_list.append(root_item->child(i));
    }

    return child_items_list;
}

void ProcessScannerDialog::setAllTopLevelItemsHidden(bool hidden) {
    auto* tree { ui->twProcessTree };

    for(auto iter_root_item : collectTreeRoot(tree)) {
        iter_root_item->setHidden(hidden);

        for(auto iter_child_item : collectTreeChildren(iter_root_item)) {
            iter_child_item->setHidden(hidden);
        }
    }
}

void ProcessScannerDialog::applySearchFilterToTree() {
    auto search_filter  { ui->linSearchFilter->text() };
    auto tree           { ui->twProcessTree };

    setAllTopLevelItemsHidden(search_filter.size());

    if(search_filter.size()) {
        /* A recursive lambda that walks down the tree of QTreeWidgetItem*'s to search for items that match the search_filter. It does this by iterating
         * over an initial list of QTreeWidgetItem*'s, and if an item matches the filter, then it is expanded, unhidden, its children are unhidden (if any),
         * and its parent (if it has one) is also unhidden and expanded. Regardless of if an item matches, a list of its children will be recursively passed
         * to this function using collectTreeChildren() after all checks have been performed, and if no children are returned by the function, the loop will
         * never iterate, and the recursive call will never be reached, ending the recursion.
         *
         * Every item expanded as a result of this function will be being appended to the searchExpansions member variable, which tracks all items that have
         * been expanded when searching, so that they can later be unexpanded when the search term is cleared, preserving the original state of the expansions.
         * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

        std::function<void(QList<QTreeWidgetItem*>)> apply_search_filter_recursively;

        apply_search_filter_recursively = [&](QList<QTreeWidgetItem*> items) {
            std::function<void(QTreeWidgetItem*)>
            // Simply unhide the given item.
            f_unhide_item {
                [](auto i) {
                    i->setHidden(false);
                }
            },

            // Expand the item if it isn't already expanded, and append the expanded item to searchExpansions, marking it for later unexpansion.
            f_expand_item {
                [&](auto i) -> void {
                    if(!i->isExpanded()) {
                        i->setExpanded(true);
                        searchExpansions.append(i);
                    }
                }
            },

            // If the item has a parent, unhide and expand the parent.
            f_expand_and_unhide_parent {
                [&](auto i) -> void {
                    if(i->parent()) {
                        f_unhide_item(i->parent());
                        f_expand_item(i->parent());
                    }
                }
            },

            // If item contains search_filter, call f_expand_and_unhide_parent, f_unhide_item, f_expand_item, and f_unhide_item over children.
            // Regardless of if the item contains search_filter, call apply_search_filter_recurisvely with the children of item.
            f_recursive_item_eval {
                [&](auto iter_item) -> void {
                    auto iter_item_children { collectTreeChildren(iter_item) };

                    if(iter_item->text(0).contains(search_filter, Qt::CaseInsensitive)) {
                        f_expand_and_unhide_parent(iter_item);
                        f_unhide_item(iter_item);
                        f_expand_item(iter_item);

                        std::for_each(iter_item_children.begin(), iter_item_children.end(), f_unhide_item);
                    }

                    apply_search_filter_recursively(iter_item_children);
                }
            };

            // Apply f_recursive_item_eval over initial sequence of items, starting the recursion.
            std::for_each(items.begin(), items.end(), f_recursive_item_eval);
        };

        apply_search_filter_recursively(collectTreeRoot(tree));
    }

    else if(searchExpansions.size()) {
        setAllTopLevelItemsHidden(false);

        for(auto expanded_tree_item : searchExpansions) {
            expanded_tree_item->setExpanded(false);
        }

        searchExpansions.clear();
    }
}

void ProcessScannerDialog::integrateProcessInfoIntoTree(ProcessScanner::ProcessInfo process_info) {
    for(const ProcessScanner::ProcessInfo::WindowInfo& window : process_info.ProcessWindows) {
        if(processWindowHandleMap.contains(window.WindowHash)) {
            qWarning() << "Window hash collision, hash already exists inside of map: " << window.WindowHash;
        }

        processWindowHandleMap[window.WindowHash] = window.WindowHandle;
    }

    auto tree        { ui->twProcessTree };
    auto root_items  { collectTreeRoot(tree) };

    auto existing_root_item {
        std::find_if(root_items.begin(), root_items.end(), [&](QTreeWidgetItem* item) -> bool {
            return item->text(1) == process_info.ProcessId;
        })
    };

    if(existing_root_item == root_items.end()) {
        auto new_root_item { process_info.MakeRootItem() };

        rootItemRemovalWhitelist.append(new_root_item);
        tree->addTopLevelItem(new_root_item);

        childItemRemovalWhitelist += collectTreeChildren(new_root_item);
    } else {
        rootItemRemovalWhitelist.append(*existing_root_item);

        if(processScannerScope & ProcessScanner::WINDOW_MODE)  {
            auto existing_children  { collectTreeChildren(*existing_root_item)  };

            for(auto& process_window : process_info.ProcessWindows) {
                auto existing_child_item {
                    std::find_if(existing_children.begin(), existing_children.end(), [&](auto iter_e) -> bool {
                        return iter_e->text(1) == process_window.WindowHash;
                    })
                };

                if(existing_child_item == existing_children.end()) {
                    auto new_child_item { process_window.MakeChildItem() };
                    childItemRemovalWhitelist.append(new_child_item);
                    (*existing_root_item)->addChild(new_child_item);
                } else {
                    childItemRemovalWhitelist.append(*existing_child_item);

                    if((*existing_child_item)->text(2) != process_window.WindowVisibility) {
                        (*existing_child_item)->setText(2, process_window.WindowVisibility);
                    }

                    if((*existing_child_item)->text(0) != process_window.WindowTitle) {
                        (*existing_child_item)->setText(0, process_window.WindowTitle);
                    }
                }
            }
        }
    }
}

void ProcessScannerDialog::applyWhitelistToTree() {
    auto tree { ui->twProcessTree };

    for(auto iter_root_item : collectTreeRoot(tree)) {
        if(!rootItemRemovalWhitelist.contains(iter_root_item)) {
            qInfo() << "Deleting root item: " << iter_root_item->text(0);

            for(auto iter_child_item : collectTreeChildren(iter_root_item)) {
                const QString& window_hash { iter_child_item->text(1) };

                if(processWindowHandleMap.contains(window_hash)) {
                    processWindowHandleMap.remove(window_hash);
                }
            }

            delete iter_root_item;
        } else {
            for(auto iter_child_item : collectTreeChildren(iter_root_item)) {
                if(!childItemRemovalWhitelist.contains(iter_child_item)) {
                    qInfo() << "Deleting child item: " << iter_child_item->text(0);
                    const QString& window_hash { iter_child_item->text(1) };

                    if(processWindowHandleMap.contains(window_hash)) {
                        processWindowHandleMap.remove(window_hash);
                    }

                    delete iter_child_item;
                }
            }
        }
    }
}

void ProcessScannerDialog::onProcessScannerScanStarted() {
    ui->twProcessTree->setSortingEnabled(false);

    ui->btnScan->setText("Scanning...");
    ui->btnScan->setEnabled(false);
    scannerCurrentlyScanning = true;

    rootItemRemovalWhitelist.clear();
    childItemRemovalWhitelist.clear();
}

void ProcessScannerDialog::onProcessScannerScanFinished() {
    applyWhitelistToTree();
    applySearchFilterToTree();

    ui->btnScan->setText("Scan Processes");
    ui->btnScan->setEnabled(true);
    scannerCurrentlyScanning = false;

    ui->twProcessTree->setSortingEnabled(true);
    ui->twProcessTree->header()->setSortIndicatorShown(false);
    ui->twProcessTree->sortByColumn(0, Qt::SortOrder::AscendingOrder);
}

void ProcessScannerDialog::emitFilteredProcessScannerScanRequest() {
    if(scannerCurrentlyScanning) return;

    qInfo() << "Filtered process scan has been requested.";

    int32_t scan_filters { (processScannerScope & ProcessScanner::PROCESS_MODE) ? NULL : ProcessScanner::FILTER_WINDOWLESS_PROCESSES };

    if(ui->cbFilterInvisibleWindows->isChecked()) {
        scan_filters |= ProcessScanner::FILTER_INVISIBLE_WINDOWS;
        qInfo() << "Added FILTER_INVISIBLE_WINDOWS to filter bitmask.";
    }

    if(ui->cbFilterDuplicateWindows->isChecked()) {
        scan_filters |= ProcessScanner::FILTER_DUPLICATE_WINDOWS;
        qInfo() << "Added FILTER_DUPLICATE_WINDOWS to filter bitmask.";
    }

    if(ui->cbFilterDuplicateProcesses->isChecked()) {
        scan_filters |= ProcessScanner::FILTER_DUPLICATE_PROCESSES;
        qInfo() << "Added FILTER_DUPLICATE_PROCESSES to filter bitmask.";
    }

    if(scan_filters == NULL) {
        qInfo() << "No scan filters have been provided.";
    }

    emit requestProcessScannerScan(processScannerScope, static_cast<ProcessScanner::SCAN_FILTERS>(scan_filters));
}

void ProcessScannerDialog::onAutoScannerTimerTimeout() {
    emitFilteredProcessScannerScanRequest();
    autoScannerTimer->stop();

    if(ui->cbAutoScanner->isChecked()) {
        autoScannerTimer->start(autoScannerInterval);
        qInfo() << "Autoscan timer started with interval: " << QString::number(autoScannerInterval);
    } else {
        qInfo() << "Autoscan timer has been deactivated.";
    }
}

void ProcessScannerDialog::evaluateWindowTreeItemSelection() {
    auto tree { ui->twProcessTree };
    auto selected_items { tree->selectedItems() };

    if(selected_items.size()) {
        auto selected_item { selected_items.first() };

        if(!selected_item->isHidden() && (!selected_item->childCount() || processScannerScope & ProcessScanner::PROCESS_MODE)) {
            const QString& window_title { selected_item->text(0) };
            const QString& window_hash  { selected_item->text(1) };

            const HWND& window_handle {
                processWindowHandleMap.contains(window_hash) ? processWindowHandleMap[window_hash] : nullptr
            };

            emit treeSelectionMade(selected_item->text(0), window_handle);
        }
    }
}

void ProcessScannerDialog::showTreeWidgetContextMenu(const QPoint& point) {
    const QPoint global_point { ui->twProcessTree->mapToGlobal(point) };
    treeWidgetContextMenu->popup(global_point);
}

ProcessScannerDialog::ProcessScannerDialog(QWidget* parent, const ProcessScanner::SCAN_SCOPE& scan_scope)
    :
      // ProcessScannerDialog Ui initialization.
      QDialog               { parent                       },
      ui                    { new Ui::ProcessScannerDialog },

      treeWidgetContextMenu { new QMenu         { this } },

      // Member variable initialization.
      autoScannerTimer          { new QTimer },
      autoScannerInterval       { 2500       },
      scannerCurrentlyScanning  { false      },
      processScannerScope       { scan_scope }
{
    ui->setupUi(this);

    setWindowFlags(
                Qt::Dialog
                | Qt::CustomizeWindowHint
                | Qt::WindowTitleHint
                | Qt::WindowCloseButtonHint
                );

    setAttribute(Qt::WA_DeleteOnClose);

    // Set initial tree widget header section sizes.
    ui->twProcessTree->header()->resizeSection(0, 250);
    ui->twProcessTree->header()->resizeSection(1, 130);
    ui->twProcessTree->header()->resizeSection(2, 40);

    switch(processScannerScope) {
    case(ProcessScanner::PROCESS_MODE) : {
        // When scannig for processes, hide window-related filters.
        ui->cbFilterDuplicateWindows->setHidden(true);
        ui->cbFilterInvisibleWindows->setHidden(true);

        // Interval can be lower when only scanning for processes, as less resources are consumed.
        ui->hsAutoScannerInterval->setMinimum(500);
        ui->sbAutoScannerInterval->setMinimum(500);
        ui->sbAutoScannerInterval->setValue(1000);
        ui->hsAutoScannerInterval->setValue(1000);
        autoScannerInterval = 1000;

        ui->cbFilterDuplicateProcesses->setChecked(true);

        break;
    }

    case(ProcessScanner::WINDOW_MODE) : {
        // When scanning for windows, set initial filter.
        ui->cbFilterInvisibleWindows->setChecked(true);
        ui->cbFilterDuplicateWindows->setChecked(true);
        break;
    }
    }

    // ui->cbFilterDuplicateWindowsbtn

    // Context Menu Signals -----------------------------------------------------------------------
    connect(ui->twProcessTree,          SIGNAL(customContextMenuRequested(const QPoint&)),
            this,                       SLOT(showTreeWidgetContextMenu(const QPoint&)));

    connect(treeWidgetContextMenu->addAction("Expand All"), &QAction::triggered, [this]() -> void {
        ui->twProcessTree->expandAll();
    });

    connect(treeWidgetContextMenu->addAction("Collapse All"), &QAction::triggered, [this]() -> void {
        ui->twProcessTree->collapseAll();
    });

    // Search Filter Signal -----------------------------------------------------------------------
    connect(ui->linSearchFilter,        SIGNAL(textChanged(QString)),
            this,                       SLOT(applySearchFilterToTree()));

    // Item Selection Signals ---------------------------------------------------------------------
    connect(ui->btnOkay,                SIGNAL(clicked()),
            this,                       SLOT(evaluateWindowTreeItemSelection()));

    connect(ui->twProcessTree,          SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),
            this,                       SLOT(evaluateWindowTreeItemSelection()));

    // ProcessScanner Signals  --------------------------------------------------------------------
    connect(this,                       SIGNAL(requestProcessScannerScan(ProcessScanner::SCAN_SCOPE, ProcessScanner::SCAN_FILTERS)),
            &processScanner,            SLOT(PerformScan(ProcessScanner::SCAN_SCOPE, ProcessScanner::SCAN_FILTERS)));

    connect(ui->btnScan,                SIGNAL(clicked()),
            this,                       SLOT(emitFilteredProcessScannerScanRequest()));

    connect(ui->btnCancel,              SIGNAL(clicked()),
            this,                       SLOT(close()));

    connect(&processScannerThread,      SIGNAL(started()),
           this,                        SLOT(emitFilteredProcessScannerScanRequest()));

    connect(&processScanner,            SIGNAL(ProcessInformationReady(ProcessScanner::ProcessInfo)),
            this,                       SLOT(integrateProcessInfoIntoTree(ProcessScanner::ProcessInfo)));

    connect(&processScanner,            SIGNAL(ScanStarted()),
            this,                       SLOT(onProcessScannerScanStarted()));

    connect(&processScanner,            SIGNAL(ScanFinished()),
            this,                       SLOT(onProcessScannerScanFinished()));

    // AutoScanner Signals ------------------------------------------------------------------------
    connect(autoScannerTimer,           SIGNAL(timeout()),
            this,                       SLOT(onAutoScannerTimerTimeout()));

    connect(ui->cbAutoScanner,          SIGNAL(stateChanged(int)),
            this,                       SLOT(onAutoScannerTimerTimeout()));

    connect(ui->hsAutoScannerInterval,  SIGNAL(sliderMoved(int)),
            ui->sbAutoScannerInterval,  SLOT(setValue(int)));

    connect(ui->sbAutoScannerInterval,  SIGNAL(valueChanged(int)),
            ui->hsAutoScannerInterval,  SLOT(setValue(int)));

    connect(ui->sbAutoScannerInterval,  &QSpinBox::valueChanged, [this](int interval) -> void {
        autoScannerInterval = interval;
    });

    // QSS Stylesheet
    // --------------------------------------------------------------------------------------------------------------
    connect(ui->linSearchFilter, &QLineEdit::textChanged, [&]() -> void { style()->polish(ui->linSearchFilter); });

    processScanner.moveToThread(&processScannerThread);     // Move the ProcessScanner instance to the QThread.
    processScannerThread.start();                           // Start the QThread attached to the instance.
}

ProcessScannerDialog::~ProcessScannerDialog() {
    autoScannerTimer->stop();
    ui->cbAutoScanner->setChecked(false);
    processScannerThread.quit();
    processScannerThread.terminate();
    processScannerThread.wait();
    delete ui;
}



