# SkLayoutEngine (sklye)
A lightweight layout engine for calculating GUI layouts

```cpp
root = Layout(LayoutType_Horizontal, 5);
auto c1 = Layout(LayoutType_Vertical, 20);
auto c2 = Layout(LayoutType_Vertical, 20);
c1.AddChild(Layout(LayoutType_None, 0));
c1.AddChild(Layout(LayoutType_None, 0));
c2.AddChild(Layout(LayoutType_None, 0));
c2.AddChild(Layout(LayoutType_None, 0, -1.f, 50.f));
c2.AddChild(Layout(LayoutType_None, 0, -1.f, 50.f));
c2.AddChild(Layout(LayoutType_None, 0, -1.f, 50.f));
root.AddChild(c1);
root.AddChild(c2);
```
![demo pic](https://github.com/devskiddlee/sklye/blob/main/demo/window.png)
(This project does not include a renderer, the example picture was rendered using DirectX11)
