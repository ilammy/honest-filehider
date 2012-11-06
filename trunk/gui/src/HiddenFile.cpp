#include "HiddenFile.h"

HiddenFile::HiddenFile(const QString &name, bool isDirectory,
                       HiddenFile *parent, quint64 ino)
  : name(name),
    ino(ino),
    hidden(isDirectory? false : true),
    directory(isDirectory),
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

HiddenFile* HiddenFile::childByName(const QString &name, bool shouldBeDir)
{
    foreach (HiddenFile *theOne, children) {
        if (theOne->isDir() == shouldBeDir && theOne->getName() == name) {
            return theOne;
        }
    }
    return NULL;
}
