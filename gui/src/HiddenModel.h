#ifndef HIDDENMODEL_H
#define HIDDENMODEL_H

#include <QAbstractItemModel>
#include <QFileInfo>
#include <QDir>
#include <QStringList>

#include "DriverGate.hpp"
#include "HiddenFile.h"

class HiddenModel : public QAbstractItemModel {
public:
    enum ErrorCode { OKAY,
                     ALREADY_HIDDEN,
                     DEVICE_NOT_FOUND,
                     DEVICE_BUSY,
                     OPEN_FILE_PROBLEM,
                     MOUNT_POINT,
                     HIDDEN_PARENT,
                     LOST_FILE,
                     HIDING_PROBLEM
                   };
public:
    HiddenModel(DriverGate *gate, QObject *parent = 0);
    ~HiddenModel();

    QModelIndex parent(const QModelIndex &index) const;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;

    ErrorCode hideFile(const QString &path, bool recursive);
    ErrorCode unhideFile(const QModelIndex &index, bool recursive);
    ErrorCode unhideAll();
    ErrorCode unhideParents(const QModelIndex &index);

    void changeDevice(const QString &path);

private:
    DriverGate *gate;
    HiddenFile *root;

    static ErrorCode translate(DriverGate::OpenStatus);
    static ErrorCode translate(DriverGate::Status);

    HiddenFile* the(const QModelIndex &index) const;

    ErrorCode hideFile(QFileInfo &info);
    ErrorCode hideDir(QFileInfo &info, bool recursive);

    ErrorCode doHideFile(const QModelIndex &parent, const QFileInfo &file);
    ErrorCode internalHideFile(const QFileInfo &file, quint64 *ino);

    bool fileAlreadyHidden(const QFileInfo &file, const QStringList &dir) const;
    bool dirAlreadyHidden(const QStringList &dirpath) const;

    QModelIndex ensureDirPath(const QStringList &dirpath);
    void removeTrashDirectories(const QModelIndex &index);

    static QStringList tokenizeDirPath(QDir dir);
    HiddenFile* tryDescent(const QStringList &dirpath) const;

    ErrorCode hideChildFiles(const QModelIndex &parentIndex, const QDir &dir);
    ErrorCode hideChildDirs(const QModelIndex &parentIndex, const QDir &dir);

    ErrorCode unhideFile_(const QModelIndex &index);
    ErrorCode unhideDir(const QModelIndex &index, bool recursive);

    ErrorCode doUnhideFile(const QModelIndex &parent, HiddenFile *file);
    ErrorCode unhideTree(HiddenFile *root);
};

#endif // HIDDENMODEL_H
