#include "Camera.h"
#include "RectInt.h"
#include "Vector2Int.h"
#include <cstdlib>

namespace ui {
Camera::Camera(RectInt view, int scroll_delta) : view_(view), scroll_delta_(scroll_delta) {}

void Camera::CenterAt(const Vector2Int coord) {
    view_.x = coord.x - (view_.width / 2);
    view_.y = coord.y - (view_.height / 2);
}

bool Camera::IsCloseToEdge(const Vector2Int coord) const {
    return std::abs(coord.x - view_.x) < scroll_delta_ ||
           std::abs(coord.y - view_.y) < scroll_delta_ ||
           std::abs(coord.x - (view_.x + view_.width - 1)) < scroll_delta_ ||
           std::abs(coord.y - (view_.y + view_.height - 1)) < scroll_delta_;
}

bool Camera::IsInView(const Vector2Int coord) const { return view_.ContainsPoint(coord); }

RectInt Camera::View() const { return view_; }

} // namespace ui
