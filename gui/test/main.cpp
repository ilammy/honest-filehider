#include "test/Test_HiddenFile.h"
#include "test/Test_HiddenModel.h"

#define RUN(testClass)               \
{   QObject *test = new testClass(); \
    QTest::qExec(test, argc, argv);  \
    delete test;                     \
}

int main(int argc, char *argv[])
{
    RUN(Test_HiddenFile);
    RUN(Test_HiddenModel);
}
