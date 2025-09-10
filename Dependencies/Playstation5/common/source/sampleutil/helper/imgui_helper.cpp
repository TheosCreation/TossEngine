/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2020 Sony Interactive Entertainment Inc.
 * 
 */

#include <sampleutil/helper/imgui_helper.h>
#include "../../source/imgui/imgui_internal.h"

namespace sce { namespace SampleUtil { namespace Helper {
namespace ImGui {
	bool	Spinner(const char	*pLabel, float	radius, int	thickness, const ImU32	&color)
	{
		ImGuiWindow *pWindow = ::ImGui::GetCurrentWindow();
		if (pWindow->SkipItems)
		{
			return false;
		}
		
		ImGuiContext		&g		= *GImGui;
		const ImGuiStyle	&style	= g.Style;
		const ImGuiID		id		= pWindow->GetID(pLabel);
		
		const ImVec2		pos		= pWindow->DC.CursorPos;
		const ImVec2		size(radius * 2.f, (radius + style.FramePadding.y) * 2.f);
		
		const ImRect		bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
		::ImGui::ItemSize(bb, style.FramePadding.y);
		if (!::ImGui::ItemAdd(bb, id))
		{
			return false;
		}
		
		// Render
		pWindow->DrawList->PathClear();
		
		const int	kNumSegments = 30;
		const int	start	= abs(ImSin(g.Time * 1.8f) * (kNumSegments - 5));
		
		const float	aMin	= IM_PI * 2.f * ((float)start) / (float)kNumSegments;
		const float	aMax	= IM_PI * 2.f * ((float)kNumSegments - 3) / (float)kNumSegments;
		
		const ImVec2 centre = ImVec2(pos.x + radius, pos.y + radius + style.FramePadding.y);
		
		for (int i = 0; i < kNumSegments; i++)
		{
			const float a = aMin + ((float)i / (float)kNumSegments) * (aMax - aMin);
			pWindow->DrawList->PathLineTo(ImVec2(centre.x + ImCos(a + g.Time * 8.f) * radius,
												centre.y + ImSin(a + g.Time * 8.f) * radius));
		}
		
		pWindow->DrawList->PathStroke(color, false, thickness);

		return true;
	}

} // namespace ImGui
}}} // namespace sce::SampleUtil::Helper