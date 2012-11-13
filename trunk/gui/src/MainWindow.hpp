#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDirModel>

#include "HiddenModel.h"
#include "DriverGate.hpp"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void hideVictimFile();
    void unhideSolacedFile();
    void unhideAll();
    void selectVictimFile();

    void updatePathViewer();
    void scrollFsTreeTo(const QString &path);

private:
    Ui::MainWindow *ui;
    QDirModel *fs_model;
    HiddenModel *hd_model;

    void setupUi();
    void displayErrorMessage(HiddenModel::ErrorCode err);
    bool tryChangeDevice();
    bool tryUnhideParents(const QModelIndex &index);
};

#endif // MAINWINDOW_H
