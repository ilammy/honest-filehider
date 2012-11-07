#ifndef HIDDENMODEL_H
#define HIDDENMODEL_H

#include <QAbstractItemModel>
#include <QFileInfo>
#include <QDir>
#include <QStringList>

#include "HiddenFile.h"

class HiddenModel : public QAbstractItemModel {
public:
    enum ErrorCode { OKAY, INVALID_INDEX, ALREADY_HIDDEN };

public:
    HiddenModel(QObject *parent = 0);
    ~HiddenModel();

    QModelIndex parent(const QModelIndex &index) const;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;

    ErrorCode hideFile(const QString &path, bool recursive);
    ErrorCode unhideFile(const QModelIndex &index);

private:
    HiddenFile *root;

    HiddenFile* the(const QModelIndex &index) const;

    ErrorCode hideFile(QFileInfo &info);
    ErrorCode hideDir(QFileInfo &info, bool recursive);

    ErrorCode doHideFile(const QModelIndex &parent, const QFileInfo &file);

    bool fileAlreadyHidden(const QFileInfo &file, const QStringList &dir) const;
    bool dirAlreadyHidden(const QStringList &dirpath) const;

    QModelIndex ensureDirPath(const QStringList &dirpath);

    static QStringList tokenizeDirPath(QDir dir);
    HiddenFile* tryDescent(const QStringList &dirpath) const;

    ErrorCode hideChildFiles(const QModelIndex &parentIndex, const QDir &dir);
    ErrorCode hideChildDirs(const QModelIndex &parentIndex, const QDir &dir);
};

#endif // HIDDENMODEL_H
