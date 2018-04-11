#ifndef __DAVAENGINE_TESTCORE_H__
#define __DAVAENGINE_TESTCORE_H__

#include "Base/BaseTypes.h"
#include "Debug/DVAssert.h"
#include "Functional/Function.h"
#include "UnitTests/TestClass.h"

namespace DAVA
{
namespace UnitTests
{
class TestClass;
class TestClassFactoryBase;

// Guard to run test functions without previous handlers and to put them back when testing is over
class AssertsHandlersGuard final
{
public:
    AssertsHandlersGuard()
        : previousHandlers(Assert::GetAllHandlers())
    {
        Assert::RemoveAllHandlers();
    }

    ~AssertsHandlersGuard()
    {
        Assert::RemoveAllHandlers();
        for (const Assert::Handler& handler : previousHandlers)
        {
            AddHandler(handler);
        }
    }

private:
    const Vector<Assert::Handler> previousHandlers;
};

// Guard and install 'Continue' Handler
static Assert::FailBehaviour ContinueHandler(const Assert::AssertInfo& assertInfo)
{
    return Assert::FailBehaviour::Continue;
}
class AssertsHandlersGuardNoAssert final
{
public:
    AssertsHandlersGuardNoAssert()
        : previousHandlers(Assert::GetAllHandlers())
    {
        Assert::RemoveAllHandlers();
        Assert::AddHandler(ContinueHandler);
    }

    ~AssertsHandlersGuardNoAssert()
    {
        Assert::RemoveAllHandlers();
        for (const Assert::Handler& handler : previousHandlers)
        {
            AddHandler(handler);
        }
    }

private:
    const Vector<Assert::Handler> previousHandlers;
};

class TestCore final
{
    struct TestClassInfo
    {
        TestClassInfo(const char* name, TestClassFactoryBase* factory);
        TestClassInfo(TestClassInfo&& other);
        ~TestClassInfo();

        String name;
        bool runTest = true;
        std::unique_ptr<TestClassFactoryBase> factory;
        TestCoverageInfo testedFiles; //-V730_NOINIT
    };

public:
    using TestClassStartedCallback = Function<void(const String&)>;
    using TestClassFinishedCallback = Function<void(const String&)>;
    using TestClassDisabledCallback = Function<void(const String&)>;
    using TestStartedCallback = Function<void(const String&, const String&)>;
    using TestFinishedCallback = Function<void(const String&, const String&)>;
    using TestFailedCallback = Function<void(const String&, const String&, const String&, const char*, int, const String&)>;

private:
    TestCore() = default;
    ~TestCore() = default;

public:
    static TestCore* Instance();

    void Init(TestClassStartedCallback testClassStartedCallback, TestClassFinishedCallback testClassFinishedCallback,
              TestStartedCallback testStartedCallback, TestFinishedCallback testFinishedCallback,
              TestFailedCallback testFailedCallback, TestClassDisabledCallback testClassDisabledCallback);

    void RunOnlyTheseTestClasses(const String& testClassNames);
    void DisableTheseTestClasses(const String& testClassNames);

    bool HasTestClasses() const;

    bool ProcessTests(float32 timeElapsed);

    Map<String, TestCoverageInfo> GetTestCoverage();

    void TestFailed(const String& condition, const char* filename, int lineno, const String& userMessage);
    void RegisterTestClass(const char* name, TestClassFactoryBase* factory);

private:
    Vector<TestClassInfo> testClasses;

    TestClass* curTestClass = nullptr;
    String curTestClassName;
    String curTestName;
    size_t curTestClassIndex = 0;
    size_t curTestIndex = 0;
    bool runLoopInProgress = false;
    bool testSetUpInvoked = false;

    TestClassStartedCallback testClassStartedCallback;
    TestClassFinishedCallback testClassFinishedCallback;
    TestClassDisabledCallback testClassDisabledCallback;
    TestStartedCallback testStartedCallback;
    TestFinishedCallback testFinishedCallback;
    TestFailedCallback testFailedCallback;
};

} // namespace UnitTests
} // namespace DAVA

#endif // __DAVAENGINE_TESTCORE_H__
