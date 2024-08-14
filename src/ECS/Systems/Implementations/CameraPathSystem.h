/******************************************************************************
 * Copyright (c) 2018-2024 openblack developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/openblack/openblack
 *
 * openblack is licensed under the GNU General Public License version 3.
 *******************************************************************************/

#pragma once

#include <chrono>
#include <deque>

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include "ECS/Systems/CameraPathSystemInterface.h"

#if !defined(LOCATOR_IMPLEMENTATIONS)
#error "ECS System implementations should only be included in Locator.cpp"
#endif

namespace openblack::ecs::systems
{

class CameraPathSystem final: public CameraPathSystemInterface
{
public:
	CameraPathSystem()
	    : _startingEntity(entt::null)
	    , _nextStepEntity(entt::null) {};
	void Start(entt::id_type id) override;
	void Stop() override;
	void Pause(bool flag) override { _paused = flag; };
	void Update(const std::chrono::microseconds& dt) override;
	bool Active() override;
	bool IsPaused() override { return _paused; };

private:
	entt::entity _startingEntity;
	entt::entity _nextStepEntity;
	glm::vec3 _currentStepCameraPosition;
	glm::vec3 _currentStepLookAtPosition;
	std::chrono::microseconds _timeElapsedDuringStep;
	std::chrono::microseconds _duration;
	bool _paused = false;
};
} // namespace openblack::ecs::systems
