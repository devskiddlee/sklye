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
    float maxWidth;
    float maxHeight;
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

    float GetMaxWidth() {
        return maxWidth;
    }

    float GetMaxHeight() {
        return maxHeight;
    }

    Layout(LayoutType type, float padding, float maxWidth = -1.f, float maxHeight = -1.f) {
        this->type = type;
        this->padding = padding;
        this->parent = nullptr;
        this->maxWidth = maxWidth;
        this->maxHeight = maxHeight;
    }

    Layout() {
        this->type = LayoutType_None;
        this->padding = 0;
        this->parent = nullptr;
        this->maxWidth = -1.f;
        this->maxHeight = -1.f;
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

        if (root->GetMaxWidth() > 0 && windowWidth > root->GetMaxWidth())
            windowWidth = root->GetMaxWidth();
        if (root->GetMaxHeight() > 0 && windowHeight > root->GetMaxHeight())
            windowHeight = root->GetMaxHeight();

        rootDimensions.x = x;
        rootDimensions.y = y;
        rootDimensions.w = windowWidth;
        rootDimensions.h = windowHeight;
        dimensions[root] = rootDimensions;

        float contentX = x + p;
        float contentY = y + p;
        float contentW = windowWidth - 2 * p;
        float contentH = windowHeight - 2 * p;

        const auto& children = root->GetChildren();
        size_t child_count = children.size();
        if (child_count == 0)
            return dimensions;

        switch (root->GetType()) {
        case LayoutType_Horizontal: {
            float totalGap = (child_count - 1) * p;
            float usableWidth = contentW - totalGap;

            std::vector<float> widths(child_count, usableWidth / child_count);
            float totalUsed = 0;
            for (size_t i = 0; i < child_count; i++) {
                float maxW = children[i]->GetMaxWidth();
                if (maxW > 0 && widths[i] > maxW)
                    widths[i] = maxW;
                totalUsed += widths[i];
            }

            float leftover = usableWidth - totalUsed;
            size_t unclampedCount = 0;
            for (size_t i = 0; i < child_count - 1; i++) {
                if (!(children[i]->GetMaxWidth() > 0 && widths[i] >= children[i]->GetMaxWidth()))
                    unclampedCount++;
            }

            if (unclampedCount > 0 && leftover > 0) {
                float add = leftover / unclampedCount;
                for (size_t i = 0; i < child_count - 1; i++) {
                    if (!(children[i]->GetMaxWidth() > 0 && widths[i] >= children[i]->GetMaxWidth()))
                        widths[i] += add;
                }
            }

            float sumWidths = 0;
            for (size_t i = 0; i < child_count - 1; i++)
                sumWidths += widths[i];
            widths[child_count - 1] = usableWidth - sumWidths;

            float curX = contentX;
            for (size_t i = 0; i < child_count; i++) {
                Layout* child = children[i];
                for (auto& dim : AssembleLayoutDimensions(
                    child,
                    widths[i],
                    contentH,
                    curX,
                    contentY))
                {
                    dimensions[dim.first] = dim.second;
                }
                curX += widths[i] + p;
            }
            break;
        }

        case LayoutType_Vertical: {
            float totalGap = (child_count - 1) * p;
            float usableHeight = contentH - totalGap;

            std::vector<float> heights(child_count, usableHeight / child_count);
            float totalUsed = 0;
            for (size_t i = 0; i < child_count; i++) {
                float maxH = children[i]->GetMaxHeight();
                if (maxH > 0 && heights[i] > maxH)
                    heights[i] = maxH;
                totalUsed += heights[i];
            }

            float leftover = usableHeight - totalUsed;
            size_t unclampedCount = 0;
            for (size_t i = 0; i < child_count - 1; i++) {
                if (!(children[i]->GetMaxHeight() > 0 && heights[i] >= children[i]->GetMaxHeight()))
                    unclampedCount++;
            }
            if (unclampedCount > 0 && leftover > 0) {
                float add = leftover / unclampedCount;
                for (size_t i = 0; i < child_count - 1; i++) {
                    if (!(children[i]->GetMaxHeight() > 0 && heights[i] >= children[i]->GetMaxHeight()))
                        heights[i] += add;
                }
            }

            float sumHeights = 0;
            for (size_t i = 0; i < child_count - 1; i++)
                sumHeights += heights[i];
            heights[child_count - 1] = usableHeight - sumHeights;

            float curY = contentY;
            for (size_t i = 0; i < child_count; i++) {
                Layout* child = children[i];
                for (auto& dim : AssembleLayoutDimensions(
                    child,
                    contentW,
                    heights[i],
                    contentX,
                    curY))
                {
                    dimensions[dim.first] = dim.second;
                }
                curY += heights[i] + p;
            }
            break;
        }
        }

        return dimensions;
    }
}