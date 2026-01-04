#pragma once
#include <utility>

#include "Vectrix/Input/Input.h"

namespace Vectrix {

	class LinInput : public Input
	{
	protected:
		bool isKeyPressedImpl(int keycode) override;

		bool isMouseButtonPressedImpl(int button) override;

		std::pair<float, float> getMousePositionImpl() override;
		float getMouseXImpl() override;
		float getMouseYImpl() override;
	};

}
