/******************************************************************************
 * Copyright (c) 2018-2024 openblack developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/openblack/openblack
 *
 * openblack is licensed under the GNU General Public License version 3.
 *******************************************************************************/

#define LOCATOR_IMPLEMENTATIONS

#include "CameraPathSystem.h"

#include <cassert>

#include <glm/gtx/euler_angles.hpp>

#include "3D/CameraPath.h"
#include "Camera/Camera.h"
#include "ECS/Components/CameraPathNode.h"
#include "ECS/Registry.h"
#include "Game.h"
#include "Locator.h"
#include "Resources/Resources.h"

using namespace openblack;
using namespace openblack::ecs::systems;
using namespace openblack::ecs::components;

entt::entity CreateLookAtEntity(const glm::vec3& origin, const openblack::CameraPoint& cameraPoint)
{
	auto& registry = Locator::entitiesRegistry::value();
	const auto lookAtEntity = registry.Create();
	const auto lookAtPosition = origin + cameraPoint.rotation;
	const auto lookAtRotation = glm::eulerAngleY(glm::radians(0.0f));
	registry.Assign<openblack::ecs::components::Transform>(lookAtEntity, lookAtPosition, lookAtRotation, glm::vec3(1.0f));
	return lookAtEntity;
}

entt::entity CreatePositionEntity(const glm::vec3 origin, const openblack::CameraPoint& cameraPoint)
{
	auto& registry = Locator::entitiesRegistry::value();
	const auto positionEntity = registry.Create();
	const auto currentPosition = origin + cameraPoint.position;
	const auto rotation = glm::eulerAngleY(glm::radians(.0f));
	registry.Assign<openblack::ecs::components::Transform>(positionEntity, currentPosition, rotation, glm::vec3(1.0f));
	return positionEntity;
}

entt::entity CreatePath(entt::id_type id, CameraPath& cameraPath)
{
	const auto& camera = Game::Instance()->GetCamera();
	const auto& cameraPos = camera.GetOrigin();
	const auto& vertices = cameraPath.GetPoints();

	// Create a copy of the path
	auto& registry = Locator::entitiesRegistry::value();

	// This is the start of the path and has an additional component
	const entt::entity start = CreatePositionEntity(cameraPos, vertices[0]);
	const uint32_t flags = 0;
	registry.Assign<CameraPathStart>(start, id, flags);
	entt::entity previousPositionEntity = start;
	entt::entity previousLookAtEntity = CreateLookAtEntity(cameraPos, vertices[0]);

	for (size_t i = 1; i < vertices.size(); i++)
	{
		const auto& vertex = vertices[i];
		auto positionEntity = CreatePositionEntity(cameraPos, vertex);
		registry.Assign<CameraPathNode>(previousPositionEntity, positionEntity, previousLookAtEntity);
		previousPositionEntity = positionEntity;
		previousLookAtEntity = CreateLookAtEntity(cameraPos, vertex);
	}

	registry.Assign<ecs::components::CameraPathNode>(previousPositionEntity, entt::null, previousLookAtEntity);
	return start;
}

void DeletePath(entt::entity start)
{
	auto& registry = Locator::entitiesRegistry::value();
	auto& startNode = registry.Get<ecs::components::CameraPathNode>(start);
	auto current = startNode.next;
	if (registry.Valid(startNode.lookAt))
	{
		registry.Destroy(startNode.lookAt);
	}

	while (current != entt::null)
	{
		auto node = registry.Get<ecs::components::CameraPathNode>(current);
		auto next = node.next;
		auto lookAt = node.lookAt;
		registry.Destroy(current);
		if (registry.Valid(lookAt))
		{
			registry.Destroy(lookAt);
		}
		current = next;
	}

	registry.Destroy(start);
}

void CameraPathSystem::Start(entt::id_type id)
{
	const auto& cameraPath = Locator::resources::value().GetCameraPaths().Handle(id);
	if (!cameraPath)
	{
		return;
	}
	_startingEntity = CreatePath(id, cameraPath);
	_nextStepEntity = _startingEntity;
	auto& registry = Locator::entitiesRegistry::value();

	const auto& nextStepNode = registry.Get<CameraPathNode>(_nextStepEntity);
	auto& camera = Game::Instance()->GetCamera();
	_currentStepCameraPosition = camera.GetOrigin();
	_currentStepLookAtPosition = camera.GetFocus();
	if (registry.Valid(nextStepNode.lookAt))
	{
		_currentStepLookAtPosition = registry.Get<Transform>(nextStepNode.lookAt).position;
	}
	_timeElapsedDuringStep = std::chrono::microseconds(0);
	_duration = cameraPath->GetDuration() / cameraPath->GetPoints().size();
	_paused = false;
}

void CameraPathSystem::Stop()
{
	DeletePath(_startingEntity);
	_startingEntity = entt::null;
	_nextStepEntity = entt::null;
	_duration = std::chrono::microseconds (0);
	_timeElapsedDuringStep = std::chrono::microseconds (0);
	_paused = false;
}

float easeCubic(float t)
{
	if (t < 0.5f)
	{
		return 4 * t * t * t;
	}

	return (t - 1) * (2 * t - 2) * (2 * t - 2) + 1;
}

void CameraPathSystem::Update(const std::chrono::microseconds& dt)
{
	auto& registry = Locator::entitiesRegistry::value();
	auto validStart = _startingEntity != entt::null && registry.Valid(_startingEntity);
	if (!validStart)
	{
		return;
	}

	auto& camera = Game::Instance()->GetCamera();
	const auto& nextStepNode = registry.Get<CameraPathNode>(_nextStepEntity);
	const auto& nextStepCameraPosition = registry.Get<Transform>(_nextStepEntity).position;
	_timeElapsedDuringStep += dt;
	float blendFactor = std::chrono::duration<float, std::micro>(_timeElapsedDuringStep / _duration).count();
	blendFactor = glm::min(blendFactor, 1.f); // Avoid overstepping
	float easedBlend = easeCubic(blendFactor);
	camera.SetOrigin(glm::lerp(_currentStepCameraPosition, nextStepCameraPosition, easedBlend));

	// Control where we look
	if (registry.Valid(nextStepNode.lookAt))
	{
		const auto& nextStepLookAtPosition = registry.Get<Transform>(nextStepNode.lookAt).position;
		camera.SetFocus(glm::lerp(_currentStepLookAtPosition, nextStepLookAtPosition, easedBlend));
	}

	// If a next step exists, prepare for it, otherwise we stop completely
	if (blendFactor >= 1.f)
	{
		// Update to a new starting position to transition from, for the camera position
		_currentStepCameraPosition = nextStepCameraPosition;
		// Update to a new starting position to transition from, for the camera angle
		if (registry.Valid(nextStepNode.lookAt))
		{
			const auto& nextStepLookAtPosition = registry.Get<Transform>(nextStepNode.lookAt).position;
			_currentStepLookAtPosition = nextStepLookAtPosition;
		}

		_nextStepEntity = nextStepNode.next;
		if (_nextStepEntity == entt::null)
		{
			Stop();
			return;
		}

		_timeElapsedDuringStep = std::chrono::microseconds(0);
	}
}

bool CameraPathSystem::Active()
{
	auto& registry = Locator::entitiesRegistry::value();
	return registry.Valid(_startingEntity);
}
