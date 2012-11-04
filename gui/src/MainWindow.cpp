#include "MainWindow.hpp"
#include "ui_MainWindow.h"

#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupUi();
}

void MainWindow::setupUi()
{
    fs_model = new QFileSystemModel(this);
    fs_model->setRootPath(QDir::rootPath());
    ui->fs_tree->setModel(fs_model);
    ui->fs_tree->setColumnHidden(1, true);
    ui->fs_tree->setColumnHidden(2, true);
    ui->fs_tree->setColumnHidden(3, true);
    ui->fs_tree->sortByColumn(0, Qt::AscendingOrder);

    hd_model = new HiddenFilesModel(this);
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
    // incorrect
    foreach (const QModelIndex &i,
             ui->fs_tree->selectionModel()->selectedRows())
    {
        switch (hd_model->hideFile(fs_model->filePath(i))) {
        default:
            break;
        }
    }
}

void MainWindow::unhideSolacedFile()
{
    // incorrect
    foreach (const QModelIndex &i,
             ui->hidden_view->selectionModel()->selectedRows())
    {
        switch (hd_model->unhideFile(i)) {
        default:
            break;
        }
    }
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
    switch (selection.size()) {
    case 0:
        ui->path_display->setText("");
        break;

    case 1:
    {   QString path = fs_model->filePath(selection.at(0));
        ui->path_display->setText(path);
        break;
    }
    default:
        ui->path_display->setText(tr("Multiple files selected"));
        break;
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
