#include "mainwindow.h"
#include <ui_mainwindow.h>
#include <cameracontrolshelp.h>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->mygl->setFocus();
    ui->PhotonNumBox->setDisabled(true);
    ui->SearchRadiusBox->setDisabled(true);
    connect(ui->integratorSpinBox, SIGNAL(currentIndexChanged(int)), this, SLOT(select_PhotonMapper(int)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionQuit_triggered()
{
    QApplication::exit();
}

void MainWindow::on_actionLoad_Scene_triggered()
{
    ui->mygl->SceneLoadDialog();
}

void MainWindow::slot_ResizeToCamera(int w, int h)
{
    setFixedWidth(w);
    setFixedWidth(h);
}

void MainWindow::on_actionRender_triggered()
{
    ui->mygl->RenderScene();
}

void MainWindow::on_actionCamera_Controls_triggered()
{
    help.show();
}
void MainWindow::slot_DisableGUI(bool b)
{
    ui->integratorSpinBox->setEnabled(!b);
    ui->samplesSpinBox->setEnabled(!b);
    ui->recursionSpinBox->setEnabled(!b);
    ui->checkBox_Progressive->setEnabled(!b);
}

void MainWindow::select_PhotonMapper(int i) {
    if (i == 4) {
        ui->PhotonNumBox->setEnabled(true);
        ui->SearchRadiusBox->setEnabled(true);
        ui->mygl->numPhotons = ui->PhotonNumBox->value();
        ui->mygl->photonSearchRadius = ui->SearchRadiusBox->value();
    }
}
