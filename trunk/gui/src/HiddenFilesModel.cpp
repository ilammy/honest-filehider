#include "HiddenFilesModel.h"

HiddenFilesModel::HiddenFilesModel(QObject *parent)
  : QAbstractItemModel(parent)
{
    root = new HiddenFile("", -1);
}

HiddenFilesModel::~HiddenFilesModel()
{
    delete root;
}

QModelIndex HiddenFilesModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    HiddenFile *parent = the(index)->parent();
    if (parent == root) {
        return QModelIndex();
    }
    return createIndex(parent->row(), 0, parent);
}

QModelIndex HiddenFilesModel::index(int row, int column,
                                    const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    HiddenFile *file = the(parent)->childAt(row);
    if (file) {
        return createIndex(row, column, file);
    }
    else {
        return QModelIndex();
    }
}

inline
int HiddenFilesModel::columnCount(const QModelIndex&) const
{
    return 1;
}

int HiddenFilesModel::rowCount(const QModelIndex &parent) const
{
    return the(parent)->childrenCount();
}

QVariant HiddenFilesModel::data(const QModelIndex &index, int role) const
{
    if (role != Qt::DisplayRole || !index.isValid()) {
        return QVariant();
    }
    return QVariant(the(index)->getPath());
}

HiddenFile* HiddenFilesModel::the(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return root;
    }
    else {
        return static_cast<HiddenFile*>(index.internalPointer());
    }
}

HiddenFilesModel::ErrorCode
HiddenFilesModel::hideFile(const QString &path)
{
#if 1
    quint64 ino = 0;
    // hide file
    beginInsertRows(QModelIndex(), root->childrenCount(), root->childrenCount());
    HiddenFile *freshie = new HiddenFile(path, ino, root);
    root->append(freshie);
    endInsertRows();
#endif
    return OKAY;
}

HiddenFilesModel::ErrorCode
HiddenFilesModel::unhideFile(const QModelIndex &index)
{
#if 1
    if (!index.isValid()) {
        return INVALID_INDEX;
    }
    int idx = index.row();
    HiddenFile *parent = the(index)->parent();
    if (!parent) {
        parent = root;
    }
    // unhide file
    beginRemoveRows(QModelIndex(), idx, idx);
    parent->removeAt(idx);
    endRemoveRows();
#endif
    return OKAY;
}
