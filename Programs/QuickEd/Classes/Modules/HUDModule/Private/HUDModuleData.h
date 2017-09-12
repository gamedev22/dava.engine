#pragma once

#include "Model/PackageHierarchy/ControlNode.h"

#include <TArc/DataProcessing/DataNode.h>

class HUDSystem;

class HUDModuleData : public DAVA::TArc::DataNode
{
    ~HUDModuleData() override;

private:
    friend class HUDModule;
    std::unique_ptr<HUDSystem> hudSystem;

    DAVA_VIRTUAL_REFLECTION(HUDModuleData, DAVA::TArc::DataNode);
};
