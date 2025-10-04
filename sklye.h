/*
    SKLaYoutEngine (sklye)
        A lightweight layout engine for calculating GUI layouts
        made by @devskiddlee [https://github.com/devskiddlee]
*/

#pragma once
#include <vector>
#include <iostream>
#include <map>

enum LayoutType {
    LayoutType_None,
    LayoutType_Horizontal,
    LayoutType_Vertical
};

class Layout {
private:
    LayoutType type;
    Layout* parent;
    std::vector<Layout*> children;
    float padding;
public:
    void AddChild(Layout* child) {
        child->SetParent(this);
        children.push_back(child);
    }

    void AddChild(Layout& child) {
        AddChild(&child);
    }

    void AddChild(Layout&& child) {
        AddChild(&child);
    }

    std::vector<Layout*> GetChildren() {
        return children;
    }

    void SetPadding(float padding) {
        this->padding = padding;
    }

    float GetPadding() {
        return padding;
    }

    bool HasParent() {
        return parent != nullptr;
    }

    Layout* GetParent() {
        return parent;
    }

    void SetParent(Layout* parent) {
        this->parent = parent;
    }

    LayoutType GetType() {
        return type;
    }

    Layout(LayoutType type, float padding) {
        this->type = type;
        this->padding = padding;
        this->parent = nullptr;
    }

    Layout() {
        this->type = LayoutType_None;
        this->padding = 0;
        this->parent = nullptr;
    }
};

struct LayoutDimensions {
    float x, y, w, h;
};

struct LayoutRect {
    float min[2];
    float max[2];
};

namespace SKLEUtils {
    // Returns a rect containing a min and max point from a LayoutDimensions object
    LayoutRect GetRectFromDimensions(LayoutDimensions dimensions) {
        LayoutRect rect{};
        rect.min[0] = dimensions.x;
        rect.min[1] = dimensions.y;
        rect.max[0] = dimensions.x + dimensions.w;
        rect.max[1] = dimensions.y + dimensions.h;
        return rect;
    }

    // Flattens a LayoutTree in Pre-Order
    // Useful for renders as this sorts them from bottom to top in their respective areas
    void flattenLayoutTree(Layout* root, std::vector<Layout*>& result) {
        if (!root) return;
        result.push_back(root);
        for (auto& child : root->GetChildren()) {
            flattenLayoutTree(child, result);
        }
    }
}

namespace SKLE {
    // Returns a map of the provided Layouts and their dimensions after calculation them
    std::map<Layout*, LayoutDimensions> AssembleLayoutDimensions(
        Layout* root, float windowWidth, float windowHeight, float x = 0, float y = 0
    ) {
        std::map<Layout*, LayoutDimensions> dimensions;
        LayoutDimensions rootDimensions{};

        float p = root->GetPadding();

        rootDimensions.x = x;
        rootDimensions.y = y;
        rootDimensions.w = windowWidth;
        rootDimensions.h = windowHeight;
        dimensions[root] = rootDimensions;

        float contentX = x + p;
        float contentY = y + p;
        float contentW = windowWidth - 2 * p;
        float contentH = windowHeight - 2 * p;

        size_t child_count = root->GetChildren().size();
        float x_off = 0;
        float y_off = 0;
        float child_w = contentW;
        float child_h = contentH;

        switch (root->GetType()) {
        case LayoutType_Horizontal:
            if (child_count > 0) {
                child_w = (contentW - (child_count - 1) * p) / child_count;
                x_off = child_w + p;
            }
            break;

        case LayoutType_Vertical:
            if (child_count > 0) {
                child_h = (contentH - (child_count - 1) * p) / child_count;
                y_off = child_h + p;
            }
            break;
        }

        for (int ix = 0; ix < root->GetChildren().size(); ix++) {
            Layout* child = root->GetChildren()[ix];
            for (auto& dim : AssembleLayoutDimensions(
                child,
                child_w,
                child_h,
                contentX + x_off * ix,
                contentY + y_off * ix
            )) {
                dimensions[dim.first] = dim.second;
            }
        }

        return dimensions;
    }
}