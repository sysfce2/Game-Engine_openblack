/*****************************************************************************
 * Copyright (c) 2018-2022 openblack developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/openblack/openblack
 *
 * openblack is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#pragma once

#include "entt/fwd.hpp"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

namespace openblack::ecs::components
{

enum class MoveStateClockwise
{
	Undefined,
	CounterClockwise,
	Clockwise,
};

enum class MoveState
{
	Linear,
	Orbit,
	ExitCircle,
	StepThrough,
	FinalStep,
	Arrived,
};

template <MoveState S>
struct MoveStateTagComponent
{
	static constexpr MoveState value = S;
	MoveStateClockwise clockwise;
	glm::vec2 stepGoal;
};

typedef MoveStateTagComponent<MoveState::Linear> MoveStateLinearTag;
typedef MoveStateTagComponent<MoveState::Orbit> MoveStateOrbitTag;
typedef MoveStateTagComponent<MoveState::ExitCircle> MoveStateExitCircleTag;
typedef MoveStateTagComponent<MoveState::StepThrough> MoveStateStepThroughTag;
typedef MoveStateTagComponent<MoveState::FinalStep> MoveStateFinalStepTag;
typedef MoveStateTagComponent<MoveState::Arrived> MoveStateArrivedTag;

struct WallHugObjectReference
{
	uint8_t stepsAway;
	entt::entity entity;
};

struct WallHug
{
	glm::vec2 goal;
	glm::vec2 step;
	float y_angle; // FIXME(bwrsandman): member is a little redundant with transform or atan on step could keep or not
	float speed;
};

} // namespace openblack::ecs::components
