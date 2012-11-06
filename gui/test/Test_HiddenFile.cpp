#include "Test_HiddenFile.h"

void Test_HiddenFile::testNameAndInoInit()
{
    QString name("Vassily Poupkine");
    quint64 ino = 314159265358LL;
    HiddenFile subject(name, false, 0, ino);

    QCOMPARE(subject.getName(), name);
    QCOMPARE(subject.getIno(), ino);
}

void Test_HiddenFile::testDirInit()
{
    HiddenFile dir(QString(), true);
    HiddenFile file(QString(), false);

    QCOMPARE(dir.isDir(), true);
    QCOMPARE(file.isDir(), false);
}

void Test_HiddenFile::testParentInit()
{
    HiddenFile parent(QString(), false);
    HiddenFile child(QString(), false, &parent);

    QCOMPARE(child.parent(), &parent);
}

void Test_HiddenFile::testDefaultIno()
{
    HiddenFile subject(QString(), false);

    QVERIFY(subject.getIno() == HiddenFile::INVALID_INO);
}

void Test_HiddenFile::testDefaultHiddenStatus()
{
    HiddenFile dir(QString(), true);
    HiddenFile file(QString(), false);

    QCOMPARE(dir.isHidden(), false);
    QCOMPARE(file.isHidden(), true);
}
