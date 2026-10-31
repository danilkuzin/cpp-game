#pragma once

#include "RectInt.h"
#include "Vector2Int.h"

namespace ui {
class Camera {
public:
    Camera(RectInt view, int scroll_delta);

    void CenterAt(Vector2Int coord);
    [[nodiscard]] bool IsCloseToEdge(Vector2Int coord) const;
    [[nodiscard]] bool IsInView(Vector2Int coord) const;
    [[nodiscard]] RectInt View() const;

private:
    RectInt view_;
    int scroll_delta_;
};
} // namespace ui
