#ifndef HIDDENFILESMODEL_H
#define HIDDENFILESMODEL_H

#include <QAbstractListModel>

#include "HiddenFile.h"

class HiddenFilesModel : public QAbstractItemModel {
public:
    enum ErrorCode { OKAY, INVALID_INDEX };

public:
    HiddenFilesModel(QObject *parent = 0);
    ~HiddenFilesModel();

    QModelIndex parent(const QModelIndex &index) const;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;

public slots:
    ErrorCode hideFile(const QString &path);
    ErrorCode unhideFile(const QModelIndex &index);

private:
    HiddenFile *root;

    HiddenFile* the(const QModelIndex &index) const;
};

#endif // HIDDENFILESMODEL_H
