#ifndef WINDOW_TREE_DIALOG_HXX
#define WINDOW_TREE_DIALOG_HXX

#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QLayout>
#include <QtWidgets/QDialog>
#include <QtWidgets/QMenu>
#include <QtGui/QAction>

#include <QtCore/QThread>
#include <QtCore/QTimer>

#include "process_scanner.hpp"

namespace Ui {
    class ProcessScannerDialog;
}

class ProcessScannerDialog : public QDialog {
Q_OBJECT

signals:
    void requestProcessScannerScan(ProcessScanner::SCAN_SCOPE, ProcessScanner::SCAN_FILTERS);

    // Emitted whenever the user chooses a window from the process/window tree.
    void treeSelectionMade(QString, HWND);

private:
    Ui::ProcessScannerDialog*    ui;

    QMenu*    treeWidgetContextMenu;

    QTimer*     autoScannerTimer;
    uint32_t    autoScannerInterval;            // The default auto-scan interval for the ProcessScanner QTimer.

    bool    scannerCurrentlyScanning;       // True when ProcessScanner is scanning, false when not. Determined by start/finished signals.

    ProcessScanner                processScanner;                 // The ProcessScanner instance tasked with WinAPI process scanning.
    ProcessScanner::SCAN_SCOPE    processScannerScope;            // The scope of the ProcessScanner instance (what to scan for).
    QThread                       processScannerThread;           // The thread that the ProcessScanner instance will run on.

    QList<QTreeWidgetItem*>    rootItemRemovalWhitelist;
    QList<QTreeWidgetItem*>    childItemRemovalWhitelist;
    QList<QTreeWidgetItem*>    searchExpansions;

    QMap<QString, HWND>        processWindowHandleMap;

    // Helper functions to collect QTreeWidgetItem*'s into QList's
    QList<QTreeWidgetItem*> collectTreeRoot(QTreeWidget*);
    QList<QTreeWidgetItem*> collectTreeChildren(QTreeWidgetItem*);

    template<typename T>
    void applyFuncToQList(QList<T> list, std::function<void(T&)> func) {
        std::for_each(list.begin(), list.end(), func);
    }

    void setAllTopLevelItemsHidden(bool);

private slots:
    void applySearchFilterToTree();
    void integrateProcessInfoIntoTree(ProcessScanner::ProcessInfo);
    void applyWhitelistToTree();

    void emitFilteredProcessScannerScanRequest();
    void onProcessScannerScanStarted();
    void onProcessScannerScanFinished();
    void onAutoScannerTimerTimeout();

    void evaluateWindowTreeItemSelection();
    void showTreeWidgetContextMenu(const QPoint& point);

public:
    explicit ProcessScannerDialog(QWidget* parent, const ProcessScanner::SCAN_SCOPE& scan_scope);
    ~ProcessScannerDialog() override;

};

#endif // WINDOW_TREE_DIALOG_HXX
