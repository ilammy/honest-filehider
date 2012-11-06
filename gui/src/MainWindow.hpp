#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemModel>

#include "HiddenModel.h"

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
    void selectVictimFile();

    void updatePathViewer();
    void scrollFsTreeTo(const QString &path);

private:
    Ui::MainWindow *ui;
    QFileSystemModel *fs_model;
    HiddenModel *hd_model;

    void setupUi();
};

#endif // MAINWINDOW_H
