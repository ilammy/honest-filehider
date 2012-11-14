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
        if (tryChangeDevice()) {
            goto again;
        }
        return;
    case HiddenModel::OKAY:
        break;
    default:
        displayErrorMessage(err);
        return;
    }
    fs_model->refresh();
}

void MainWindow::unhideSolacedFile()
{
again:
    QModelIndexList sel = ui->hidden_view->selectionModel()->selectedRows();
    Q_ASSERT(sel.count() <= 1);
    if (sel.count() == 0) {
        return;
    }
    bool recursive = ui->recursive_unhide_checkbox->isChecked();
    HiddenModel::ErrorCode err = hd_model->unhideFile(sel.at(0), recursive);
    switch (err) {
    case HiddenModel::HIDDEN_PARENT:
        if (tryUnhideParents(sel.at(0))) {
            goto again;
        }
        return;
    case HiddenModel::DEVICE_NOT_FOUND:
        if (tryChangeDevice()) {
            goto again;
        }
        return;
    case HiddenModel::OKAY:
        break;
    default:
        displayErrorMessage(err);
        return;
    }
    fs_model->refresh();
}

void MainWindow::unhideAll()
{
again:
    HiddenModel::ErrorCode err = hd_model->unhideAll();
    switch (err) {
    case HiddenModel::DEVICE_NOT_FOUND:
        if (tryChangeDevice()) {
            goto again;
        }
        return;
    case HiddenModel::OKAY:
        break;
    default:
        displayErrorMessage(err);
        return;
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

void MainWindow::manualPathEntered()
{
    scrollFsTreeTo(ui->path_display->text());
}

void MainWindow::scrollFsTreeTo(const QString &path)
{
    QModelIndex index = fs_model->index(path);
    if (index.isValid()) {
        ui->fs_tree->selectionModel()
            ->setCurrentIndex(index, QItemSelectionModel::Clear
                                   | QItemSelectionModel::Select
                                   | QItemSelectionModel::Current
                                   | QItemSelectionModel::Rows);
        ui->fs_tree->scrollTo(index, QAbstractItemView::PositionAtCenter);
    }
}

bool MainWindow::tryChangeDevice()
{
    QMessageBox::StandardButton choice = QMessageBox::critical(
        this, tr("Humble error"),
        tr("Driver communication file could not be found. "
           "Would you like to select another one?"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::Yes);
    bool changed = false;
    if (choice == QMessageBox::Yes) {
        QString filename = QFileDialog::getOpenFileName(
            this, tr("Select the driver communication file"));
        if (!filename.isNull()) {
            // set new driver name
            changed = true;
        }
    }
    return changed;
}

bool MainWindow::tryUnhideParents(const QModelIndex &index)
{
    QMessageBox::StandardButton choice = QMessageBox::critical(
        this, tr("Humble error"),
        tr("Cannot unhide the file while its parent is still hidden. "
           "Would you like to unhide its parents?"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);
    bool didHide = (choice == QMessageBox::Yes);
    if (didHide) {
        HiddenModel::ErrorCode err = hd_model->unhideParents(index);
        if (err != HiddenModel::OKAY) {
            didHide = false;
            displayErrorMessage(err);
        }
    }
    return didHide;
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
    case HiddenModel::HIDDEN_PARENT:
        message = tr("Cannot unhide the file while its parent is still hidden");
        break;
    case HiddenModel::LOST_FILE:
        message = tr("Could not find the requested file.");
        break;
    default:
        return;
    }
    QMessageBox::critical(this, tr("Humble error"), message);
}
