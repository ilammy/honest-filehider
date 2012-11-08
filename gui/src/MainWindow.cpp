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
    QModelIndexList sel = ui->fs_tree->selectionModel()->selectedRows();
    Q_ASSERT(sel.count() <= 1);
    if (sel.count() == 1) {
        bool recursive = ui->recursive_checkbox->isChecked();
        switch (hd_model->hideFile(fs_model->filePath(sel.at(0)), recursive)) {
        // handle errors
        default:
            break;
        }
        // update fs_model
    }
}

void MainWindow::unhideSolacedFile()
{
    QModelIndexList sel = ui->hidden_view->selectionModel()->selectedRows();
    Q_ASSERT(sel.count() <= 1);
    if (sel.count() == 1) {
        switch (hd_model->unhideFile(sel.at(0))) {
        // handle errors
        default:
            break;
        }
        // update fs_model
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
