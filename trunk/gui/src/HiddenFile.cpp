#include "HiddenFile.h"

HiddenFile::HiddenFile(const QString &path, quint64 inode, HiddenFile *parent)
  : fullPath(path),
    ino(inode),
    theParent(parent)
{}

HiddenFile::~HiddenFile()
{
    qDeleteAll(children);
}

void HiddenFile::append(HiddenFile *file)
{
    children.append(file);
}

void HiddenFile::removeAt(int idx)
{
    HiddenFile *orphan = children[idx];
    children.removeAt(idx);
    delete orphan;
}

HiddenFile* HiddenFile::extractAt(int idx)
{
    HiddenFile *orphan = children[idx];
    children.removeAt(idx);
    return orphan;
}

int HiddenFile::row() const
{
    if (theParent != NULL) {
        return theParent->children.indexOf(const_cast<HiddenFile*>(this));
    }
    return 0;
}
