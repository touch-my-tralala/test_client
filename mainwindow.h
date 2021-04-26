#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QMessageBox>
#include <QtCore>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void slotSockReady();
    void slotSockDisconnected();

private:
    Ui::MainWindow *ui;
    QSharedPointer<QTcpSocket> socket;
};

#endif // MAINWINDOW_H
