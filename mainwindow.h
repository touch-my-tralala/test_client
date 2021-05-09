#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QMessageBox>
#include <QtCore>
#include <QInputDialog>
#include <QHBoxLayout>
#include <QCheckBox>


namespace Ui {
class MainWindow;

#define READ_BLOCK_SIZE 32768
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    struct ResInf{
        ResInf(){}
        ResInf(QString usr, int hh, int mm, int ss){
            currenUser = usr;
            time = new QTime(hh, mm, ss);
        }
        ~ResInf(){
            delete time;
            time = nullptr;
        }

        QString currenUser = "Free";
        QTime* time = nullptr;
    };


public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


private slots:
    void slotSockReady();
    void slotSockDisconnected();
    void slotConnected();
    void on_takeRes_btn_clicked();
    void on_clearRes_btn_clicked();
    void on_reconnect_btn_clicked();
    void time_update();
    void timeout_recconect();


private:
    bool init(const QString &str);
    void json_handler(const QJsonObject &jObj);
    void autorization(const QJsonObject &jObj);
    void res_intercept(const QJsonObject &jObj);
    void req_responce_take(const QJsonObject &jObj);
    void req_responce_free(const QJsonObject &jObj);
    void table_update(const QJsonObject &jObj);
    void fail_to_connect();
    void filling_table();
    void send_to_host(const QJsonObject &jObj);


private:
    QString usrName;
    QMap<quint8, ResInf*> m_resList;
    Ui::MainWindow *ui;
    QJsonParseError jsonErr;
    QSharedPointer<QTcpSocket> socket;
    QSharedPointer<QTimer> timer;
    QSharedPointer<QTimer> reconnectTimer;
    QByteArray buff;
    qint64 secsPassed;
    quint32 reconnect_sec = 0;
};

#endif // MAINWINDOW_H
