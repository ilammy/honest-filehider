#include "Test_HiddenModel.h"

void Test_HiddenModel::testColumnCount_root()
{
    HiddenModel *model = new HiddenModel();
    QCOMPARE(model->columnCount(QModelIndex()), 1);
    delete model;
}
