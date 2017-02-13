#pragma once

#include "FileSystem/FilePath.h"
#include "Base/FastName.h"

class PackageInformation;
class ControlInformation;

class FindFilter
{
public:
    FindFilter();
    virtual ~FindFilter();

    virtual bool CanAcceptPackage(const PackageInformation* package) const = 0;
    virtual bool CanAcceptControl(const ControlInformation* control) const = 0;
};

class PrototypeUsagesFilter : public FindFilter
{
public:
    PrototypeUsagesFilter(const DAVA::String& packagePath, const DAVA::FastName& prototypeName);
    ~PrototypeUsagesFilter();

    bool CanAcceptPackage(const PackageInformation* package) const override;
    bool CanAcceptControl(const ControlInformation* control) const override;

private:
    DAVA::String packagePath;
    DAVA::FastName prototypeName;
};

class CompositeFilter : public FindFilter
{
public:
    CompositeFilter(const DAVA::Vector<std::shared_ptr<FindFilter>>& filters);
    ~CompositeFilter();

    bool CanAcceptPackage(const PackageInformation* package) const override;
    bool CanAcceptControl(const ControlInformation* control) const override;

private:
    DAVA::Vector<std::shared_ptr<FindFilter>> filters;
};

class ControlNameFilter : public FindFilter
{
public:
    ControlNameFilter(const DAVA::FastName& expectedName);
    ~ControlNameFilter();

    bool CanAcceptPackage(const PackageInformation* package) const override;
    bool CanAcceptControl(const ControlInformation* control) const override;

private:
    DAVA::FastName expectedName;
};
