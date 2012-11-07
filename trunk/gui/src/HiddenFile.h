#ifndef HIDDENFILE_H
#define HIDDENFILE_H

#include <QList>
#include <QString>

class HiddenFile {
public:
    static const quint64 INVALID_INO = ~((quint64)0);

public:
    HiddenFile(const QString &name, bool isDirectory,
               HiddenFile *parent = 0, quint64 ino = INVALID_INO);
    ~HiddenFile();

    const QString& getName() const { return name; }
    const quint64  getIno()  const { return ino; }

    void hide(bool really) { hidden = really; }
    bool isHidden() const { return hidden; }
    bool isDir() const { return directory; }

    void append(HiddenFile *file);
    void removeAt(int idx);
    HiddenFile* extractAt(int idx);

    HiddenFile* childAt(int idx) { return children[idx]; }
    HiddenFile* parent() { return theParent; }
    int childrenCount() const { return children.count(); }
    int row() const;

    HiddenFile* childFileByName(const QString &name) const;
    HiddenFile* childDirByName(const QString &name) const;

private:
    QString name;
    quint64 ino;

    bool hidden;
    bool directory;

    QList<HiddenFile*> children;
    HiddenFile *theParent;
};

#endif // HIDDENFILE_H
