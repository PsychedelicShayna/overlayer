# Overlayer
Overlayer allows you to turn any window(s) into an overlay for your games and programs, by allowing you to modify your windows to introduce three new components to them: topmost, clickthrough, and transparency - by using these three components together, you effectively create an overlay.

When the program closes, any window modifications made will be reset automatically, so you need not worry about persisting modifications. Windows can be selected, either from a list, or by using the window grabber - after being selected, the window will added to the inactive window list, where it can be moved to the active window list to enable modifications for that window, and moved back should you want to disable modifications temporarily, without losing the modified state.

In order to modify a window in the active window list, it must be selected first. Multiple windows can be selected as well in order to modify multiple windows using the same modifications at the same time. The modifications are stored on a per-window basis. The three modifications are as follows:

1) **Topmost** - when enabled, the window remains on top of all other windows.
 
2) **Clickthrough** - when enabled, clicks pass right through the window, preventing it from gaining focus by disabling interaction with it. Optionally, a hotkey can be used to toggle this modification on and off, allowing you to interact with the window momentarily by disabling clickthrough, and then re-enabling it once you're finished interacting with it.

3) **Transparency** - when enabled, the selected window will be configured to use the opacity set by the slider below the control buttons. 
   
### Demonstration Gif
![](overlayer-demo.gif?raw=true)
