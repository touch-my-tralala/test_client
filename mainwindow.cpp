#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    socket = QSharedPointer<QTcpSocket>(new QTcpSocket(this));
    connect(socket.data(), &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater); // no matching member functiom for call to 'connect'

}

void MainWindow::slotSockReady(){

}

void MainWindow::slotSockDisconnected(){

}

MainWindow::~MainWindow()
{
    delete ui;
}
