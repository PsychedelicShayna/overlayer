#include "list_widget_window_item.hpp"

void ListWidgetWindowItem::WindowState::LayeredWindowAttributes::
    ApplyAttributes(const HWND& window_handle) const
{
    if(IsWindow(window_handle)) {
        SetLayeredWindowAttributes(window_handle, ColorRef, Alpha, Flags);
    }
}

void ListWidgetWindowItem::WindowState::LayeredWindowAttributes::
    RetrieveAttributes(const HWND& window_handle)
{
    if(IsWindow(window_handle)
       && GetWindowLong(window_handle, GWL_EXSTYLE) & WS_EX_LAYERED) {
        GetLayeredWindowAttributes(window_handle, &ColorRef, &Alpha, &Flags);

        // If the alpha mode isn't turned on, an alpha mode probably wasn't set,
        // so manually ensure that the alpha is 255 so that the window doesn't
        // start transparent if alpha is turned on.
        if(Flags != LWA_ALPHA) {
            Alpha = 0xFF;
        }
    } else {
        Clear();
    }
}

bool ListWidgetWindowItem::WindowState::LayeredWindowAttributes::operator==(
    const ListWidgetWindowItem::WindowState::LayeredWindowAttributes& other)
    const
{
    return ColorRef == other.ColorRef && Alpha == other.Alpha
           && Flags == other.Flags;
}

bool ListWidgetWindowItem::WindowState::LayeredWindowAttributes::operator!=(
    const ListWidgetWindowItem::WindowState::LayeredWindowAttributes& other)
    const
{
    return !(*this == other);
}

void ListWidgetWindowItem::WindowState::LayeredWindowAttributes::Clear()
{
    ColorRef = NULL;
    Alpha    = 0xFF;
    Flags    = NULL;
}

ListWidgetWindowItem::WindowState::LayeredWindowAttributes::
    LayeredWindowAttributes(const HWND& window_handle)
{
    RetrieveAttributes(window_handle);
}

ListWidgetWindowItem::WindowState::LayeredWindowAttributes::
    LayeredWindowAttributes()
{
    Clear();
}

bool ListWidgetWindowItem::WindowState::HasLayering() const
{
    return ExStyle & WS_EX_LAYERED;
}

void ListWidgetWindowItem::WindowState::EnableLayering()
{
    ExStyle |= WS_EX_LAYERED;
}

void ListWidgetWindowItem::WindowState::DisableLayering()
{
    ExStyle &= ~WS_EX_LAYERED;
}

bool ListWidgetWindowItem::WindowState::HasTopmost() const
{
    return Topmost;
}

void ListWidgetWindowItem::WindowState::EnableTopmost()
{
    Topmost = true;
}

void ListWidgetWindowItem::WindowState::DisableTopmost()
{
    Topmost = false;
}

bool ListWidgetWindowItem::WindowState::HasClickthrough() const
{
    return ExStyle & WS_EX_TRANSPARENT;
}

void ListWidgetWindowItem::WindowState::EnableClickthrough()
{
    ExStyle |= WS_EX_TRANSPARENT;
}

void ListWidgetWindowItem::WindowState::DisableClickthrough()
{
    ExStyle &= ~WS_EX_TRANSPARENT;
}

void ListWidgetWindowItem::WindowState::EnableAlphaTransparencyMode()
{
    LWAttributes.Flags = LWA_ALPHA;
}

void ListWidgetWindowItem::WindowState::EnableColorkeyTransparencyMode()
{
    LWAttributes.Flags = LWA_COLORKEY;
}

void ListWidgetWindowItem::WindowState::DisableTransparency()
{
    LWAttributes.Flags = NULL;
}

bool ListWidgetWindowItem::WindowState::ApplyState(
    const HWND& window_handle) const
{
    if(IsWindow(window_handle)) {
        SetWindowLong(window_handle, GWL_EXSTYLE, ExStyle);

        SetWindowPos(window_handle,
                     Topmost ? HWND_TOPMOST : HWND_NOTOPMOST,
                     NULL,
                     NULL,
                     NULL,
                     NULL,
                     SWP_NOSIZE | SWP_NOMOVE | SWP_ASYNCWINDOWPOS
                         | SWP_FRAMECHANGED);

        if(HasLayering()) {
            LWAttributes.ApplyAttributes(window_handle);
        }

        return true;
    }

    return false;
}

bool ListWidgetWindowItem::WindowState::RetrieveState(const HWND& window_handle)
{
    if(IsWindow(window_handle)) {
        ExStyle = GetWindowLong(window_handle, GWL_EXSTYLE);

        if(ExStyle & WS_EX_LAYERED) {
            LWAttributes.RetrieveAttributes(window_handle);
        }

        return true;
    }

    return false;
}

bool ListWidgetWindowItem::WindowState::operator==(
    const ListWidgetWindowItem::WindowState& other) const
{
    return ExStyle == other.ExStyle && Topmost == other.Topmost
           && LWAttributes == other.LWAttributes;
}

bool ListWidgetWindowItem::WindowState::operator!=(
    const ListWidgetWindowItem::WindowState& other) const
{
    return !(*this == other);
}

ListWidgetWindowItem::WindowState::WindowState(const HWND& window_handle)
  : LWAttributes {window_handle},
    Topmost {false}
{
    if(IsWindow(window_handle)) {
        ExStyle = GetWindowLong(window_handle, GWL_EXSTYLE);
    }
}

ListWidgetWindowItem::WindowState::WindowState()
  : ExStyle {NULL},
    Topmost {false}
{
}

bool ListWidgetWindowItem::IsValidWindow() const
{
    return IsWindow(WindowHandle);
}

void ListWidgetWindowItem::ApplyModifiedState()
{
    ModifiedState.ApplyState(WindowHandle);
}

void ListWidgetWindowItem::ApplyOriginalState()
{
    OriginalState.ApplyState(WindowHandle);
}

void ListWidgetWindowItem::ResetModifications()
{
    ModifiedState = OriginalState;
    ModifiedState.ApplyState(WindowHandle);
}

ListWidgetWindowItem::ListWidgetWindowItem(const HWND& window_handle)
  : ModifiedState {window_handle},
    OriginalState {window_handle},
    WindowHandle {window_handle},
    RespondToHotkey {false}
{
}

ListWidgetWindowItem::~ListWidgetWindowItem()
{
    ApplyOriginalState();
}
