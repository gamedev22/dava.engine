#pragma once

#include "Entity/SceneSystem.h"
#include "Base/FastName.h"
#include "Base/Vector.h"
#include "NetworkCore/UDPTransport/UDPServer.h"

namespace DAVA
{
class Scene;
class Entity;
class NetworkConnectionsSingleComponent;
class NetworkGameModeSingleComponent;
class Camera;
}

class GameModeSingleComponent;

class GameModeSystem : public DAVA::SceneSystem
{
public:
    enum GameModeType
    {
        WAITING,
        BATTLE
    };

    DAVA_VIRTUAL_REFLECTION(GameModeSystem, DAVA::SceneSystem);

    GameModeSystem(DAVA::Scene* scene);
    void Process(DAVA::float32 timeElapsed) override;
    void PrepareForRemove() override{};
    void OnClientConnected(const DAVA::FastName& token);

private:
    void ProcessWaitingGameMode();
    void ProcessBattleGameMode();
    void TuneComponentPrivacy();

    DAVA::Camera* camera = nullptr;
    DAVA::NetworkConnectionsSingleComponent* netConnectionsComp = nullptr;
    DAVA::NetworkGameModeSingleComponent* netGameModeComp = nullptr;
    GameModeSingleComponent* gameModeComp = nullptr;
};
