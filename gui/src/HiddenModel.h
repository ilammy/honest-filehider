#ifndef HIDDENMODEL_H
#define HIDDENMODEL_H

#include <QAbstractItemModel>
#include <QFileInfo>
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

    ErrorCode hideFile(const QString &path);
    ErrorCode unhideFile(const QModelIndex &index);

private:
    HiddenFile *root;

    HiddenFile* the(const QModelIndex &index) const;

    ErrorCode tryHideFile(QFileInfo &info);
    ErrorCode tryHideDirectory(QFileInfo &info);

    bool alreadyPresent(const QFileInfo &info, const QStringList &dirs) const;
    QModelIndex ensureParentPath(const QStringList &dirpath);

    static QStringList getDirPath(const QFileInfo &info);
    HiddenFile* tryWalkToEnd(const QStringList &dirpath) const;
};

#endif // HIDDENMODEL_H
