#ifndef TEST_HIDDENMODEL_H
#define TEST_HIDDENMODEL_H

#include <QtTest/QtTest>

#include "src/HiddenModel.h"

class Test_HiddenModel : public QObject {
    Q_OBJECT
private slots:
    void testColumnCount_root();
};

#endif // TEST_HIDDENMODEL_H
