#include "MakeSession.h"
#include "Camera.h"
#include "Config.h"
#include "MakeRegistry.h"
#include "RectInt.h"
#include "Session.h"
#include "TestData.h"

game_states::Session MakeTestSession() {
    const domain::Config config = MakeTestConfig();
    const domain::ViewConfig view = MakeTestViewConfig();
    return game_states::Session{
        .config = config,
        .registry = MakeTestRegistry(),
        .runtime =
            game_states::RuntimeState{.camera = ui::Camera{RectInt{.x = view.camera_width,
                                                                   .y = view.camera_height,
                                                                   .width = view.camera_width,
                                                                   .height = view.camera_height},
                                                           view.scroll_delta},
                                      .log = {}},
        .db = MakeTestDatabase(),
        .stats = {}};
}
