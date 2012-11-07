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

//
// Qt MVC boilerplate
//

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

//
// Hidden file tree management
//

HiddenModel::ErrorCode
HiddenModel::hideFile(const QString &path, bool recursive)
{
    QFileInfo info(path);
    Q_ASSERT(info.isAbsolute());
    if (info.isFile()) {
        return hideFile(info);
    }
    else {
        return hideDir(info, recursive);
    }
}

HiddenModel::ErrorCode
HiddenModel::unhideFile(const QModelIndex &index)
{
    return OKAY;
}

HiddenModel::ErrorCode
HiddenModel::hideFile(QFileInfo &file)
{
    QStringList parentPath = tokenizeDirPath(file.dir());
    if (fileAlreadyHidden(file, parentPath)) {
        return ALREADY_HIDDEN;
    }
    QModelIndex parent = ensureDirPath(parentPath);
    return doHideFile(parent, file);
}

HiddenModel::ErrorCode
HiddenModel::doHideFile(const QModelIndex &parent, const QFileInfo &file)
{
    quint64 ino;
    // hide(file.absoluteFilePath(), &ino)
    // check for errors
    int index = the(parent)->childrenCount();
    beginInsertRows(parent, index, index);
    HiddenFile *child = new HiddenFile(file.fileName(), false, the(parent), ino);
    the(parent)->append(child);
    endInsertRows();
    return OKAY;
}

HiddenModel::ErrorCode
HiddenModel::hideDir(QFileInfo &dir, bool recursive)
{
    const QDir the_dir(dir.absoluteFilePath());
    QStringList dirpath = tokenizeDirPath(the_dir);
    if (dirAlreadyHidden(dirpath)) {
        return ALREADY_HIDDEN;
    }
    ErrorCode err = OKAY;
    QModelIndex dirIndex = ensureDirPath(dirpath);
    err = hideChildFiles(dirIndex, the_dir);
    if (err != OKAY) {
        return err;
    }
    if (recursive) {
        err = hideChildDirs(dirIndex, the_dir);
        if (err != OKAY) {
            return err;
        }
        // hide the dir
        // check for errors
        the(dirIndex)->hide(true);
    }
    return OKAY;
}

bool HiddenModel::fileAlreadyHidden(const QFileInfo &info,
                                    const QStringList &dirpath) const
{
    HiddenFile *parent = tryDescent(dirpath);
    if (!parent) {
        return false;
    }
    HiddenFile *child = parent->childFileByName(info.fileName());
    return (child != NULL && child->isHidden());
}

bool HiddenModel::dirAlreadyHidden(const QStringList &dirpath) const
{
    HiddenFile *dir = tryDescent(dirpath);
    return (dir != NULL && dir->isHidden());
}

HiddenFile* HiddenModel::tryDescent(const QStringList &dirpath) const
{
    HiddenFile *currentFile = root;
    for (QStringList::ConstIterator currentDir = dirpath.constBegin(),
                                    lastDir = dirpath.constEnd();
         currentDir != lastDir;
         ++currentDir)
    {
        currentFile = currentFile->childDirByName(*currentDir);
        if (currentFile == NULL) {
            return NULL;
        }
    }
    return currentFile;
}

QStringList HiddenModel::tokenizeDirPath(QDir dir)
{
    QStringList result;
    while (!dir.isRoot()) {
        result.push_front(dir.dirName());
        dir.cdUp();
    }
    result.push_front(dir.absolutePath());
    return result;
}

QModelIndex HiddenModel::ensureDirPath(const QStringList &dirpath)
{
    QModelIndex currentIndex = QModelIndex();
    HiddenFile *currentFile = root;
    QStringList::ConstIterator currentDir = dirpath.constBegin();
    QStringList::ConstIterator lastDir = dirpath.constEnd();
    while (currentDir != lastDir) {
        HiddenFile *child = currentFile->childDirByName(*currentDir);
        if (child == NULL) {
            break;
        }
        currentIndex = index(child->row(), 0, currentIndex);
        currentFile = child;
        ++currentDir;
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

HiddenModel::ErrorCode
HiddenModel::hideChildFiles(const QModelIndex &parentIndex, const QDir &dir)
{
    QFileInfoList children = dir.entryInfoList();
    foreach (const QFileInfo &entry, children) {
        if (!entry.isFile()) {
            continue;
        }
        ErrorCode err = doHideFile(parentIndex, entry);
        if (err != OKAY) {
            return err;
        }
    }
    return OKAY;
}

HiddenModel::ErrorCode
HiddenModel::hideChildDirs(const QModelIndex &parentIndex, const QDir &dir)
{
    QFileInfoList children = dir.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot);
    foreach (const QFileInfo &entry, children) {
        if (entry.isFile()) {
            continue;
        }
        // inefficient
        QFileInfo theDir(entry);
        ErrorCode err = hideDir(theDir, true);
        if (err != OKAY) {
            return err;
        }
    }
    return OKAY;
}
