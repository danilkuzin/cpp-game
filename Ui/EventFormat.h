#pragma once

#include "Event.h"
#include "MessageLog.h"
#include "RectInt.h"
#include "Registry.h"
#include "RenderScene.h"
#include <string>
#include <vector>

namespace ui {

std::string FormatEvent(const event::Event &event, const domain::Registry &registry);
std::vector<std::string> FormatEventResult(const event::Result &result,
                                           const domain::Registry &registry);

void PushLogPanel(ui::RenderScene &scene, const RectInt &rect, const MessageLog &log);

int CountMonstersSlain(const event::Result &result);

} // namespace ui
