#ifndef PROCESSWINDOWLISTITEM_HXX
#define PROCESSWINDOWLISTITEM_HXX

#ifndef WIN32_MEAN_AND_LEAN
#define WIN32_MEAN_AND_LEAN
#endif

#include <Windows.h>

#include <QtWidgets/QListWidgetItem>

#include <cstdint>

class ProcessWindowListItem : public QListWidgetItem {
public:
    struct WindowState {
        struct LayeredWindowAttributes {
            ulong      ColorRef;
            uint8_t    Alpha;
            ulong      Flags;

            void ApplyAttributes(const HWND& window_handle) const;
            void RetrieveAttributes(const HWND& window_handle);

            bool operator==(const LayeredWindowAttributes& other) const;
            bool operator!=(const LayeredWindowAttributes& other) const;

            LayeredWindowAttributes(const HWND& window_handle);
            LayeredWindowAttributes() = default;
        } LWAttributes;

        int32_t    ExStyle;
        bool       Topmost;

        bool HasTopmost() const;
        void EnableTopmost();
        void DisableTopmost();

        bool HasTransparency() const;
        void EnableTransparency();
        void DisableTransparency();

        bool HasClickthrough() const;
        void EnableClickthrough();
        void DisableClickthrough();

        bool ApplyState(const HWND& window_handle) const;
        bool RetrieveState(const HWND& window_handle);

        bool operator==(const WindowState& other) const;
        bool operator!=(const WindowState& other) const;

        WindowState(const HWND& window_handle);
        WindowState() = default;
    }                    ModifiedState;
    const WindowState    OriginalState;
    const HWND           WindowHandle;

    void ApplyModifiedState();
    void ApplyOriginalState();
    void ResetModifications();

    ProcessWindowListItem(const HWND& window_handle);
};

#endif // PROCESSWINDOWLISTITEM_HXX
