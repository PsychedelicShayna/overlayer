#include "process_scanner.hpp"

QTreeWidgetItem* ProcessScanner::ProcessInfo::WindowInfo::MakeChildItem() {
    auto child_item {
        new QTreeWidgetItem {{
            WindowTitle,
            WindowHash,
            WindowVisibility
        }}
    };

    return child_item;
}

ProcessScanner::ProcessInfo::WindowInfo::WindowInfo(const QString& title, const QString& hash, const bool& is_visible, const HWND& window_handle)
    :
      WindowTitle         { title                                },
      WindowHash          { hash                                 },
      WindowVisibility    { is_visible ? "Visible" : "Invisible" },
      IsWindowVisible     { is_visible                           },
      WindowHandle        { window_handle                        }
{

}

QTreeWidgetItem* ProcessScanner::ProcessInfo::MakeRootItem() {
    auto root_item {
        new QTreeWidgetItem { {
            ProcessImageName, ProcessId
        } }
    };

    for(const auto& window : ProcessWindows) {
        auto child_item {
            new QTreeWidgetItem {{
                window.WindowTitle,
                window.WindowHash,
                window.WindowVisibility
            }}
        };

        root_item->addChild(child_item);
    }

    return root_item;
}

ProcessScanner::ProcessInfo::ProcessInfo(const QString& process_image_name, const QString& process_id)
    :
      ProcessImageName    { process_image_name },
      ProcessId           { process_id         }
{

}

QString ProcessScanner::getHwndHash(HWND hwnd) {
    uint8_t* hwnd_bytes { reinterpret_cast<uint8_t*>(&hwnd) };
    int32_t  hwnd_size  { sizeof(hwnd)                      };

    uint8_t digest_buffer[4];
    ZeroMemory(digest_buffer, sizeof(digest_buffer));

    for(int32_t i { 0 }, c { hwnd_size - 1 }; i < hwnd_size && c >= 0; ++i, --c) {
        hwnd_bytes[c] ^= hwnd_bytes[i];
    }

    for(int32_t i { 0 }; i < static_cast<int32_t>(hwnd_size); ++i) {
        uint16_t& digest_short = reinterpret_cast<uint16_t*>(digest_buffer)[(i % 2 == 0) ? 0 : 1];
        const uint8_t& hwnd_byte { hwnd_bytes[i] };

        digest_short ^= static_cast<uint16_t>(hwnd_byte) << 8;

        for(uint8_t c { 0 }; c < 8; ++c) {
            if(digest_short & 0x8000) {
                digest_short ^= (digest_short << 1) ^ 0x1021;
            } else {
                digest_short <<= 1;
            }
        }
    }

    for(int32_t i { 0 }, c { sizeof(digest_buffer) - 1 }; i < sizeof(digest_buffer) && c >= 0; ++i, --c) {
        digest_buffer[c] ^= digest_buffer[i];
    }

    QString hexdigest;

    for(size_t i { 0 }; i < sizeof(digest_buffer); ++i) {
        char hex_buffer[5];
        ZeroMemory(hex_buffer, sizeof(hex_buffer));
        sprintf(hex_buffer,"%02X", digest_buffer[i]);
        hexdigest += hex_buffer;
    }

    return hexdigest;
}

void ProcessScanner::PerformScan(ProcessScanner::SCAN_SCOPE scope, ProcessScanner::SCAN_FILTERS filters) {
    emit ScanStarted();

    HANDLE process_snapshot { CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0) };

    if(process_snapshot == INVALID_HANDLE_VALUE) {
        return;
    }

    PROCESSENTRY32 process_entry;
    process_entry.dwSize = sizeof(PROCESSENTRY32);

    QList<QString> existing_process_names;

    if(Process32First(process_snapshot, &process_entry)) {
        do {
            QString process_image_name { QString::fromWCharArray(process_entry.szExeFile) };
            QString process_pid { QString::number(process_entry.th32ProcessID) };

            /* The ProcessInfo instance that will store information about the current iteration's process,
             * which will later be populated with information about its window, and emited as a signal. */
            ProcessInfo process_information { process_image_name, process_pid };

            if(filters & FILTER_DUPLICATE_PROCESSES) {
                if(existing_process_names.contains(process_image_name)) {
                    continue;
                } else {
                    existing_process_names.append(process_image_name);
                }
            }

            // Grab the process windows, if included in the scope.
            if(scope & WINDOW_MODE) {
                HWND process_window { nullptr };
                QList<QString> existing_window_titles;

                do {
                    process_window = FindWindowEx(nullptr, process_window, nullptr, nullptr);

                    DWORD window_pid { NULL };
                    GetWindowThreadProcessId(process_window, &window_pid);

                    if(window_pid == process_entry.th32ProcessID) {
                        BOOL window_visible { IsWindowVisible(process_window) };

                        if(filters & FILTER_INVISIBLE_WINDOWS && !window_visible) {
                            continue;
                        }

                        wchar_t window_title_buffer[512];
                        ZeroMemory(window_title_buffer, sizeof(window_title_buffer));

                        int32_t bytes_written { GetWindowText(process_window, window_title_buffer, sizeof(window_title_buffer)) };

                        if(bytes_written) {
                            QString window_title { QString::fromWCharArray(window_title_buffer) };

                            if(filters & FILTER_DUPLICATE_WINDOWS) {
                                if(existing_window_titles.contains(window_title)) {
                                    continue;
                                } else {
                                    existing_window_titles.append(window_title);
                                }
                            }

                            process_information.ProcessWindows.emplace_back(window_title, getHwndHash(process_window), window_visible, process_window);
                        }
                    }
                } while(process_window != nullptr);
            }

            if(filters & FILTER_WINDOWLESS_PROCESSES) {
                if(process_information.ProcessWindows.size()) {
                    emit ProcessInformationReady(process_information);
                }
            } else {
                emit ProcessInformationReady(process_information);
            }

        } while (Process32Next(process_snapshot, &process_entry));
    }

    CloseHandle(process_snapshot);
    emit ScanFinished();
}

void ProcessScanner::CrossThreadDelete(QTreeWidgetItem* new_root_item) {
    delete new_root_item;
}
