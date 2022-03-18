#ifndef PROCESSSCANNER_HPP
#define PROCESSSCANNER_HPP

#include <QtCore/QObject>
#include <QtWidgets/QTreeWidgetItem>

#ifndef WIN32_MEAN_AND_LEAN
#define WIN32_MEAN_AND_LEAN
#endif

#include <Windows.h>
#include <TlHelp32.h>

class ProcessScanner : public QObject {
Q_OBJECT
public:
    struct ProcessInfo {
        struct WindowInfo {
            QString    WindowTitle;
            QString    WindowHash;
            QString    WindowVisibility;
            bool       IsWindowVisible;
            HWND       WindowHandle;

            QTreeWidgetItem* MakeChildItem();

            WindowInfo(const QString&, const QString&, const bool&, const HWND&);
            WindowInfo() = default;
        };

        QList<WindowInfo> ProcessWindows;

        QString ProcessImageName;
        QString ProcessId;

        QTreeWidgetItem* MakeRootItem();

        ProcessInfo(const QString&, const QString&);
        ProcessInfo() = default;
    };

    enum SCAN_FILTERS {
        FILTER_DUPLICATE_WINDOWS      = 0b00000001,
        FILTER_DUPLICATE_PROCESSES    = 0b00000010,
        FILTER_INVISIBLE_WINDOWS      = 0b00000100,
        FILTER_WINDOWLESS_PROCESSES   = 0b00001000,
        FILTERS_DISABLED              = NULL
    };

    enum SCAN_SCOPE {
        PROCESS_MODE         = 0b00000001,
        WINDOW_MODE          = 0b00000010
    };

signals:
    void ScanStarted();
    void ProcessInformationReady(ProcessScanner::ProcessInfo);
    void ScanFinished();

private:
    QString getHwndHash(HWND);

public slots:
    void PerformScan(ProcessScanner::SCAN_SCOPE, ProcessScanner::SCAN_FILTERS);

    /* Deletes an object that was allocated within this class, in case of multithreading.
     * This is needed because this class is intended to be passed into a QThread, and so
     * whenever this class allocates something which is passed onto the receiving thread
     * through a signal (as is the case with PerformScan() allocating a QTreeWidgetItem
     * and passing it through the RootItemReady signal onto the receiver) the receiving
     * thread will not be able to delete it without crashing the program, because in Qt,
     * cross-thread access of Qt elements results in a bypass of the event loop of the
     * thread that the object was allocated on, which sets everything on fire. This slot
     * should be connected to a signal on the thread receiving the results, that should
     * be emitted on the receiving thread to delete objects that belong to this thread.
     *
     * https://stackoverflow.com/a/8592659/6136296
     * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
    void CrossThreadDelete(QTreeWidgetItem*);
};

#endif // PROCESSSCANNER_HPP
