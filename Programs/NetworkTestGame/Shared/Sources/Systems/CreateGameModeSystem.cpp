#include "CreateGameModeSystem.h"

#include "Scene3D/Scene.h"
#include "Scene3D/Entity.h"
#include "Logger/Logger.h"

#include "ShooterConstants.h"
#include "ShooterUtils.h"

#include "Systems/GameModeSystemCars.h"
#include "Systems/GameModeSystem.h"
#include "Systems/PhysicsProjectileSystem.h"
#include "Systems/GameInputSystem.h"
#include "Systems/ShootInputSystem.h"
#include "Systems/ShootSystem.h"
#include "Systems/PhysicsProjectileInputSystem.h"
#include "Systems/MarkerSystem.h"
#include "Components/SingleComponents/BattleOptionsSingleComponent.h"

#include <Physics/PhysicsSystem.h>

#include <NetworkCore/NetworkCoreUtils.h>

#include <Reflection/ReflectionRegistrator.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkReplicationSingleComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkResimulationSingleComponent.h>
#include <NetworkCore/Scene3D/Systems/NetworkRemoteInputSystem.h>

using namespace DAVA;

DAVA_VIRTUAL_REFLECTION_IMPL(CreateGameModeSystem)
{
    ReflectionRegistrator<CreateGameModeSystem>::Begin()[M::Tags("network", "client")]
    .ConstructorByPointer<Scene*>()
    .Method("Process", &CreateGameModeSystem::Process)[M::SystemProcess(SP::Group::GAMEPLAY, SP::Type::NORMAL, 2.0f)]
    .End();
}

CreateGameModeSystem::CreateGameModeSystem(DAVA::Scene* scene)
    : SceneSystem(scene, ComponentMask())
{
    DVASSERT(IsClient(scene));
}

void CreateGameModeSystem::Process(DAVA::float32 timeElapsed)
{
    NetworkReplicationSingleComponent::EntityToInfo& entityToInfo = GetScene()->GetSingleComponent<NetworkReplicationSingleComponent>()->replicationInfo;
    BattleOptionsSingleComponent* optionsComp = GetScene()->GetSingleComponent<BattleOptionsSingleComponent>();
    if (!isInit)
    {
        const auto& findIt = entityToInfo.find(NetworkID::SCENE_ID);
        if (findIt != entityToInfo.end() && findIt->second.frameIdLastApply > 0)
        {
            CreateGameSystems(optionsComp->gameModeId);

            if (optionsComp->isEnemyPredicted)
            {
                GetScene()->AddTag(FastName("enemy_predict"));
            }

            if (!optionsComp->options.gameStatsLogPath.empty())
            {
                GetScene()->AddTag(FastName("log_game_stats"));
            }

            isInit = true;
        }
    }

    if (GetScene()->HasTag(FastName("input")) && nullptr == remoteInputSystem)
    {
        remoteInputSystem = GetScene()->GetSystem<NetworkRemoteInputSystem>();
        if (remoteInputSystem)
        {
            remoteInputSystem->SetFullInputComparisonFlag(optionsComp->compareInputs);
        }
    }
}

void CreateGameModeSystem::CreateGameSystems(GameMode::Id gameModeId)
{
    // TODO: clean up this mess.
    UnorderedSet<FastName> tags = { FastName("input") };
    bool isShooterGm = false;
    bool isInvaderGm = false;
    bool isCubesGm = false;
    switch (gameModeId)
    {
    case GameMode::Id::CARS:
        tags.insert({ FastName("gm_cars") });
        break;
    case GameMode::Id::TANKS:
        tags.insert({ FastName("gm_tanks"), FastName("shoot") });
        break;
    case GameMode::Id::SHOOTER:
        tags.insert({ FastName("gm_shooter"), FastName("gameshow") });
        isShooterGm = true;
        break;
    case GameMode::Id::INVADERS:
        tags.insert({ FastName("gm_invaders") });
        isInvaderGm = true;
        break;
    case GameMode::Id::CUBES:
        tags.insert({ FastName("gm_cubes") }); // this is actually ignored.
        isCubesGm = true;
        break;
    default:
        DVASSERT(0);
    }

    if (isShooterGm)
    {
        MarkerSystem* markerSystem = GetScene()->GetSystem<MarkerSystem>();
        if (markerSystem)
        {
            markerSystem->SetHealthParams(SHOOTER_CHARACTER_MAX_HEALTH, 0.02f);
        }

        InitializeScene(*GetScene());
    }
    else if (!isInvaderGm)
    {
        tags.insert({ FastName("gameinput"), FastName("gameshow"), FastName("playerentity") });
    }

    if (isCubesGm)
    {
        for (const char* tag : { "gm_cubes", "input", "gameshow" })
        {
            GetScene()->AddTag(FastName(tag));
        }
    }
    else
    {
        for (auto& tag : tags)
        {
            GetScene()->AddTag(tag);
        }
    }
}
