#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "json_keys/keys.h"
#include <QMainWindow>
#include <QTcpSocket>
//#include <QMessageBox>
#include <QtCore>
#include <QInputDialog>
//#include <QHBoxLayout>
//#include <QCheckBox>
#include <QHostAddress>
#include <QSystemTrayIcon>
#include <QCloseEvent>
#include "table_model/mytablewidget.h"


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

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void slotSockReady();
    void slotSockDisconnected();
    void slotConnected();
    void time_update();
    void timeout_recconect();
    void on_takeButton_clicked();
    void on_dropButton_clicked();
    void on_reconnectButton_clicked();
    void on_tray_icon_clicked(QSystemTrayIcon::ActivationReason reason);

private:
    void ini_parse(const QString &fname);
    bool init(const QString &str);
    void json_handler(const QJsonObject &jObj);
    void autorization();
    void res_intercept(const QJsonObject &jObj);
    void req_responce(const QJsonObject &jObj);
    void table_info_update(const QJsonObject &jObj);
    void fail_to_connect();
    void filling_table();
    void send_to_host(const QJsonObject &jObj);


private:
    quint16 m_port;

    QSystemTrayIcon *m_tray_icon;
    QMenu *menu;
    QAction *view_window;
    QAction *quit_app;

    QHostAddress m_address;
    QString m_name;
    QMap<QString, QPair<QString, QTime>> m_resList; //!< <имя ресурса, <пользователь, время использования>>
    Ui::MainWindow *ui;
    MyTableWidget *m_table_w = nullptr;
    QSettings* sett;
    QJsonParseError jsonErr;
    QSharedPointer<QTcpSocket> socket;
    QSharedPointer<QTimer> timer;
    QSharedPointer<QTimer> reconnectTimer;
    QByteArray buff;
    quint32 reconnect_sec = 0;
};

#endif // MAINWINDOW_H
