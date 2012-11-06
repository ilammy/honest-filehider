#include "HiddenModel.h"

#include <QDir>

HiddenModel::HiddenModel(QObject *parent)
  : QAbstractItemModel(parent)
{
    root = new HiddenFile("", false);
}

HiddenModel::~HiddenModel()
{
    delete root;
}

QModelIndex HiddenModel::parent(const QModelIndex &index) const
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

QModelIndex HiddenModel::index(int row, int column,
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
int HiddenModel::columnCount(const QModelIndex&) const
{
    return 1;
}

int HiddenModel::rowCount(const QModelIndex &parent) const
{
    return the(parent)->childrenCount();
}

QVariant HiddenModel::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case Qt::DisplayRole:
        return QVariant(the(index)->getName());

    default:
        return QVariant();
    }
}

HiddenFile* HiddenModel::the(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return root;
    }
    else {
        return static_cast<HiddenFile*>(index.internalPointer());
    }
}

HiddenModel::ErrorCode
HiddenModel::hideFile(const QString &path)
{
    QFileInfo info(path);
    if (info.isFile()) {
        return tryHideFile(info);
    }
    else {
        return tryHideDirectory(info);
    }
}

HiddenModel::ErrorCode
HiddenModel::unhideFile(const QModelIndex &index)
{
    return OKAY;
}

HiddenModel::ErrorCode
HiddenModel::tryHideFile(QFileInfo &info)
{
    QStringList dirpath = getDirPath(info);
    if (alreadyPresent(info, dirpath)) {
        return ALREADY_HIDDEN;
    }
    quint64 ino;
    // hide(info.absoluteFilePath(), &ino)
    // check for errors
    QModelIndex parent = ensureParentPath(dirpath);
    int index = the(parent)->childrenCount();
    beginInsertRows(parent, index, index);
    HiddenFile *child = new HiddenFile(info.fileName(), false, the(parent), ino);
    the(parent)->append(child);
    endInsertRows();
    return OKAY;
}

HiddenModel::ErrorCode
HiddenModel::tryHideDirectory(QFileInfo &info)
{
    return OKAY;
}

bool HiddenModel::alreadyPresent(const QFileInfo &info,
                                 const QStringList &dirpath) const
{
    HiddenFile *parent = tryWalkToEnd(dirpath);
    if (!parent) {
        return false;
    }
    return (parent->childByName(info.fileName()) != NULL);
}

QModelIndex HiddenModel::ensureParentPath(const QStringList &dirpath)
{
    QModelIndex currentIndex = QModelIndex();
    HiddenFile *currentFile = root;
    QStringList::ConstIterator currentDir = dirpath.constBegin();
    QStringList::ConstIterator lastDir = dirpath.constEnd();
    while (currentDir != lastDir) {
        HiddenFile *child = currentFile->childByName(*currentDir, true);
        if (child != NULL) {
            currentIndex = index(child->row(), 0, currentIndex);
            currentFile = child;
            ++currentDir;
        }
        else {
            break;
        }
    }
    while (currentDir != lastDir) {
        HiddenFile *child = new HiddenFile(*currentDir, true, currentFile);
        beginInsertRows(currentIndex,
                        currentFile->childrenCount(),
                        currentFile->childrenCount());
        currentFile->append(child);
        endInsertRows();
        currentIndex = index(child->row(), 0, currentIndex);
        currentFile = child;
        ++currentDir;
    }
    return currentIndex;
}

QStringList HiddenModel::getDirPath(const QFileInfo &info)
{
    QDir dir(info.dir());
    QStringList result;
    while (!dir.isRoot()) {
        result.push_front(dir.dirName());
        dir.cdUp();
    }
    result.push_front(dir.absolutePath());
    return result;
}

HiddenFile* HiddenModel::tryWalkToEnd(const QStringList &dirpath) const
{
    HiddenFile *currentFile = root;
    for (QStringList::ConstIterator currentDir = dirpath.constBegin(),
                                    lastDir = dirpath.constEnd();
         currentDir != lastDir;
         ++currentDir)
    {
        currentFile = currentFile->childByName(*currentDir, true);
        if (currentFile == NULL) {
            break;
        }
    }
    return currentFile;
}
