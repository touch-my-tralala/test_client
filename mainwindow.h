#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QMessageBox>
#include <QtCore>
#include <QInputDialog>



namespace Ui {
class MainWindow;

#define READ_BLOCK_SIZE 32768
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
    void slotConnected();

private:
    bool init(const QString &str);
    void json_handler(const QJsonObject &jObj);


private:
    Ui::MainWindow *ui;
    QSharedPointer<QTcpSocket> socket;
    QJsonParseError jsonErr;
    QString servStartTime;
    QByteArray buff;

};

#endif // MAINWINDOW_H
