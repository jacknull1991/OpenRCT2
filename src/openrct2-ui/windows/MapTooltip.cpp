/*****************************************************************************
 * Copyright (c) 2014-2022 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include "../interface/Theme.h"

#include <openrct2-ui/interface/Widget.h>
#include <openrct2-ui/windows/Window.h>
#include <openrct2/Context.h>
#include <openrct2/Input.h>
#include <openrct2/drawing/Drawing.h>
#include <openrct2/localisation/Formatter.h>
#include <openrct2/localisation/Localisation.h>

// clang-format off

static constexpr const int32_t WW = 200;
static constexpr const int32_t WH = 44;

static rct_widget window_map_tooltip_widgets[] = {
    MakeWidget({0, 0}, {200, 30}, WindowWidgetType::ImgBtn, WindowColour::Primary),
    WIDGETS_END,
};

// clang-format on

#define MAP_TOOLTIP_ARGS
//
static ScreenCoordsXY _lastCursor;
static int32_t _cursorHoldDuration;
//
static Formatter _mapTooltipArgs;

class MapTooltipWindow final : public Window
{
public:
    void OnOpen() override
    {
        widgets = window_map_tooltip_widgets;
    }

    void OnUpdate() override
    {
        Invalidate();
    }

    void OnDraw(rct_drawpixelinfo& dpi) override
    {
        StringId stringId;
        std::memcpy(&stringId, _mapTooltipArgs.Data(), sizeof(StringId));
        if (stringId == STR_NONE)
        {
            return;
        }

        ScreenCoordsXY stringCoords(windowPos + ScreenCoordsXY{ width / 2, height / 2 });
        DrawTextWrapped(&dpi, stringCoords, width, STR_MAP_TOOLTIP_STRINGID, _mapTooltipArgs, { TextAlignment::CENTRE });
    }
};

void SetMapTooltip(Formatter& ft)
{
    _mapTooltipArgs = ft;
}

const Formatter& GetMapTooltip()
{
    return _mapTooltipArgs;
}

void WindowMapTooltipUpdateVisibility()
{
    if (ThemeGetFlags() & UITHEME_FLAG_USE_FULL_BOTTOM_TOOLBAR)
    {
        // The map tooltip is drawn by the bottom toolbar
        window_invalidate_by_class(WindowClass::BottomToolbar);
        return;
    }

    const CursorState* state = context_get_cursor_state();
    auto cursor = state->position;
    auto cursorChange = cursor - _lastCursor;

    // Check for cursor movement
    _cursorHoldDuration++;
    if (abs(cursorChange.x) > 5 || abs(cursorChange.y) > 5 || (input_test_flag(INPUT_FLAG_5))
        || input_get_state() == InputState::ViewportRight)
    {
        _cursorHoldDuration = 0;
    }
    _lastCursor = cursor;

    // Show or hide tooltip
    StringId stringId;
    std::memcpy(&stringId, _mapTooltipArgs.Data(), sizeof(StringId));

    if (_cursorHoldDuration < 25 || stringId == STR_NONE
        || InputTestPlaceObjectModifier(
            static_cast<PLACE_OBJECT_MODIFIER>(PLACE_OBJECT_MODIFIER_COPY_Z | PLACE_OBJECT_MODIFIER_SHIFT_Z))
        || window_find_by_class(WindowClass::Error) != nullptr)
    {
        window_close_by_class(WindowClass::MapTooltip);
    }
    else
    {
        ScreenCoordsXY pos = { cursor.x - (WW / 2), cursor.y + 15 };

        auto window = WindowFocusOrCreate<MapTooltipWindow>(
            WindowClass::MapTooltip, pos, WW, WH, WF_STICK_TO_FRONT | WF_TRANSPARENT | WF_NO_BACKGROUND);

        window->Invalidate();
        window->windowPos = pos;
        window->width = WW;
        window->height = WH;
    }
}

