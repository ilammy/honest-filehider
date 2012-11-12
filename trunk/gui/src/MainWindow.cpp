#include "MainWindow.hpp"
#include "ui_MainWindow.h"

#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupUi();
}

void MainWindow::setupUi()
{
    fs_model = new QDirModel(this);
    ui->fs_tree->setModel(fs_model);
    ui->fs_tree->setColumnHidden(1, true);
    ui->fs_tree->setColumnHidden(2, true);
    ui->fs_tree->setColumnHidden(3, true);
    ui->fs_tree->sortByColumn(0, Qt::AscendingOrder);

    DriverGate *gate = new DriverGate("/dev/hcontrol");
    hd_model = new HiddenModel(gate, this);
    ui->hidden_view->setModel(hd_model);

    connect(ui->fs_tree->selectionModel(),
            SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(updatePathViewer()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::hideVictimFile()
{
again:
    QModelIndexList sel = ui->fs_tree->selectionModel()->selectedRows();
    Q_ASSERT(sel.count() <= 1);
    if (sel.count() == 0) {
        return;
    }
    bool recursive = ui->recursive_checkbox->isChecked();
    HiddenModel::ErrorCode err;
    err = hd_model->hideFile(fs_model->filePath(sel.at(0)), recursive);
    switch (err) {
    case HiddenModel::DEVICE_NOT_FOUND:
    {   QMessageBox::StandardButton choice = QMessageBox::critical(
            this, tr("Humble error"),
            tr("Driver communication file could not be found. "
               "Would you like to select another one?"),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::Yes);
        if (choice == QMessageBox::No) {
            return;
        }
        QString filename = QFileDialog::getOpenFileName(
            this, tr("Select the driver communication file"));
        if (!filename.isNull()) {
            // set driver name & try again
            goto again;
        }
        return;
    }
    case HiddenModel::DEVICE_BUSY:
    case HiddenModel::ALREADY_HIDDEN:
    case HiddenModel::OPEN_FILE_PROBLEM:
    case HiddenModel::MOUNT_POINT:
    case HiddenModel::HIDING_PROBLEM:
        displayErrorMessage(err);
        return;
    default:
        break;
    }
    fs_model->refresh();
}

void MainWindow::unhideSolacedFile()
{
    QModelIndexList sel = ui->hidden_view->selectionModel()->selectedRows();
    Q_ASSERT(sel.count() <= 1);
    if (sel.count() == 0) {
        return;
    }
    switch (hd_model->unhideFile(sel.at(0))) {
    // handle errors
    default:
        break;
    }
    fs_model->refresh();
}

void MainWindow::unhideAll()
{
    switch (hd_model->unhideAll()) {
    // handle errors
    default:
        break;
    }
    fs_model->refresh();
}

void MainWindow::selectVictimFile()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Select a file to hide"));
    if (!filename.isNull()) {
        scrollFsTreeTo(filename);
        ui->path_display->setText(filename);
    }
}

void MainWindow::updatePathViewer()
{
    QModelIndexList selection = ui->fs_tree->selectionModel()->selectedRows();
    if (selection.size() == 1) {
        QString path = fs_model->filePath(selection.at(0));
        ui->path_display->setText(path);
    }
    else {
        ui->path_display->setText("");
    }
}

void MainWindow::scrollFsTreeTo(const QString &path)
{
    QModelIndex index = fs_model->index(path);
    if (index.isValid()) {
        ui->fs_tree->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
        ui->fs_tree->scrollTo(index, QAbstractItemView::PositionAtCenter);
    }
}

void MainWindow::displayErrorMessage(HiddenModel::ErrorCode err)
{
    // too lazy to create a map
    QString message;
    switch (err) {
    case HiddenModel::DEVICE_BUSY:
        message = tr("Device is busy, try again later");
        break;
    case HiddenModel::ALREADY_HIDDEN:
        message = tr("File is already hidden");
        break;
    case HiddenModel::OPEN_FILE_PROBLEM:
        message = tr("Problems with opening device file. Check permissons");
        break;
    case HiddenModel::MOUNT_POINT:
        message = tr("Hiding of mount points and their direct descendants is prohibited");
        break;
    case HiddenModel::HIDING_PROBLEM:
        message = tr("Internal problems in the module");
        break;
    default:
        return;
    }
    QMessageBox::critical(this, tr("Humble error"), message);
}
