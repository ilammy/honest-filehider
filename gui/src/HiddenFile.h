#ifndef HIDDENFILE_H
#define HIDDENFILE_H

#include <QList>
#include <QString>

class HiddenFile {
public:
    HiddenFile(const QString &path, quint64 inode, HiddenFile *parent = 0);
    ~HiddenFile();

    const QString& getPath() const { return fullPath; }
    const quint64  getIno()  const { return ino; }

    void append(HiddenFile *file);
    void removeAt(int idx);
    HiddenFile* extractAt(int idx);

    HiddenFile* childAt(int idx) { return children[idx]; }
    HiddenFile* parent() { return theParent; }
    int childrenCount() const { return children.count(); }
    int row() const;

private:
    QString fullPath;
    quint64 ino;

    QList<HiddenFile*> children;
    HiddenFile *theParent;
};

#endif // HIDDENFILE_H
