#ifndef TEST_HIDDENFILE_H
#define TEST_HIDDENFILE_H

#include <QtTest/QtTest>

#include "src/HiddenFile.h"

class Test_HiddenFile : public QObject {
    Q_OBJECT
private slots:
    void testNameAndInoInit();
    void testDirInit();
    void testParentInit();

    void testDefaultIno();
    void testDefaultHiddenStatus();
};

#endif // TEST_HIDDENFILE_H
