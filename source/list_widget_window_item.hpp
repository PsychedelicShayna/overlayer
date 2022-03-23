#ifndef ListWidgetWindowItem_HXX
#define ListWidgetWindowItem_HXX

#ifndef WIN32_MEAN_AND_LEAN
#define WIN32_MEAN_AND_LEAN
#endif

#include <Windows.h>

#include <QtWidgets/QListWidgetItem>

#include <cstdint>

class ListWidgetWindowItem : public QListWidgetItem {
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

            void Clear();

            LayeredWindowAttributes(const HWND& window_handle);
            LayeredWindowAttributes();
        } LWAttributes;

        int32_t    ExStyle;
        bool       Topmost;

        bool HasLayering() const;
        void EnableLayering();
        void DisableLayering();

        bool HasTopmost() const;
        void EnableTopmost();
        void DisableTopmost();

        bool HasClickthrough() const;
        void EnableClickthrough();
        void DisableClickthrough();

        void EnableAlphaTransparencyMode();
        void EnableColorkeyTransparencyMode();
        void DisableTransparency();

        bool ApplyState(const HWND& window_handle) const;
        bool RetrieveState(const HWND& window_handle);

        bool operator==(const WindowState& other) const;
        bool operator!=(const WindowState& other) const;

        WindowState(const HWND& window_handle);
        WindowState();
    }                    ModifiedState;
    const WindowState    OriginalState;
    const HWND           WindowHandle;
    bool                 RespondToHotkey;

    bool IsValidWindow() const;

    void ApplyModifiedState();
    void ApplyOriginalState();
    void ResetModifications();

    ListWidgetWindowItem(const HWND& window_handle);
};

#endif // ListWidgetWindowItem_HXX
