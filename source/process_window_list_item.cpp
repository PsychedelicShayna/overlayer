#include "process_window_list_item.hpp"

void ProcessWindowListItem::WindowState::LayeredWindowAttributes::ApplyAttributes(const HWND& window_handle) const {
    if(IsWindow(window_handle)) {
        SetLayeredWindowAttributes(window_handle, ColorRef, Alpha, Flags);
    }
}

void ProcessWindowListItem::WindowState::LayeredWindowAttributes::RetrieveAttributes(const HWND& window_handle) {
    if(IsWindow(window_handle)) {
        GetLayeredWindowAttributes(window_handle, &ColorRef, &Alpha, &Flags);
    }
}

bool ProcessWindowListItem::WindowState::LayeredWindowAttributes::operator==(const ProcessWindowListItem::WindowState::LayeredWindowAttributes& other) const {
    return    ColorRef == other.ColorRef
           && Alpha    == other.Alpha
           && Flags    == other.Flags;
}

bool ProcessWindowListItem::WindowState::LayeredWindowAttributes::operator!=(const ProcessWindowListItem::WindowState::LayeredWindowAttributes& other) const {
    return !(*this == other);
}

ProcessWindowListItem::WindowState::LayeredWindowAttributes::LayeredWindowAttributes(const HWND& window_handle) {
    RetrieveAttributes(window_handle);
}

bool ProcessWindowListItem::WindowState::HasTopmost() const {
    return Topmost;
}

void ProcessWindowListItem::WindowState::EnableTopmost() {
    Topmost = true;
}

void ProcessWindowListItem::WindowState::DisableTopmost() {
    Topmost = false;
}

bool ProcessWindowListItem::WindowState::HasClickthrough() const {
    return ExStyle & WS_EX_TRANSPARENT;
}

void ProcessWindowListItem::WindowState::EnableClickthrough() {
    ExStyle |= WS_EX_TRANSPARENT;
}

void ProcessWindowListItem::WindowState::DisableClickthrough() {
    ExStyle &= ~WS_EX_TRANSPARENT;
}

bool ProcessWindowListItem::WindowState::HasTransparency() const {
    return ExStyle & WS_EX_LAYERED;
}

void ProcessWindowListItem::WindowState::EnableTransparency() {
    ExStyle |= WS_EX_LAYERED;
}

void ProcessWindowListItem::WindowState::DisableTransparency() {
    ExStyle &= ~WS_EX_LAYERED;
}

bool ProcessWindowListItem::WindowState::ApplyState(const HWND& window_handle) const {
    if(IsWindow(window_handle)) {
        SetWindowLong(window_handle, GWL_EXSTYLE, ExStyle);
        SetWindowPos(window_handle, Topmost ? HWND_TOPMOST : HWND_NOTOPMOST, NULL, NULL, NULL, NULL,  SWP_NOSIZE
                                                                                                    | SWP_NOMOVE
                                                                                                    | SWP_ASYNCWINDOWPOS
                                                                                                    | SWP_FRAMECHANGED);
        if(HasTransparency()) {
            LWAttributes.ApplyAttributes(window_handle);
        }

        return true;
    }

    return false;
}

bool ProcessWindowListItem::WindowState::RetrieveState(const HWND& window_handle) {
    if(IsWindow(window_handle)) {
        ExStyle = GetWindowLong(window_handle, GWL_EXSTYLE);

        if(ExStyle & WS_EX_LAYERED) {
            LWAttributes.RetrieveAttributes(window_handle);
        }

        return true;
    }

    return false;
}

bool ProcessWindowListItem::WindowState::operator==(const ProcessWindowListItem::WindowState& other) const {
    return     ExStyle      == other.ExStyle
            && Topmost      == other.Topmost
            && LWAttributes == other.LWAttributes;
}

bool ProcessWindowListItem::WindowState::operator!=(const ProcessWindowListItem::WindowState& other) const {
    return !(*this == other);
}


ProcessWindowListItem::WindowState::WindowState(const HWND& window_handle)
    :
      LWAttributes { window_handle }
{

}

void ProcessWindowListItem::ApplyModifiedState() {
    ModifiedState.ApplyState(WindowHandle);
}

void ProcessWindowListItem::ApplyOriginalState() {
    OriginalState.ApplyState(WindowHandle);
}

void ProcessWindowListItem::ResetModifications() {
    ModifiedState = OriginalState;
    ModifiedState.ApplyState(WindowHandle);
}

ProcessWindowListItem::ProcessWindowListItem(const HWND& window_handle)
    :
      ModifiedState { window_handle },
      OriginalState { window_handle },
      WindowHandle  { window_handle }
{

}
