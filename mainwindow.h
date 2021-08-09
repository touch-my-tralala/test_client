#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCloseEvent>
#include <QHostAddress>
#include <QInputDialog>
#include <QMainWindow>
#include <QPicture>
#include <QSystemTrayIcon>
#include <QTcpSocket>
#include <QtCore>

#include "autoupdater/restautoupdater.h"
#include "context_config/contextconfiguration.h"
#include "json_keys/keys.h"
#include "table_model/mytablewidget.h"
#include "widgets/hostinputdialog.h"
#include "widgets/sendgoosewidget.h"

namespace Ui
{
    class MainWindow;

#define READ_BLOCK_SIZE 32768
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
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
    void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void on_change_name_triggered();
    void on_change_host_triggered();
    void on_send_goose_triggered();
    void on_check_updates_triggered();
    void update_successfull_load();

private:
    void init();
    void build_interface();
    void json_handler(const QJsonObject& jObj);
    void autorization();
    void res_intercept(const QJsonObject& jObj);
    void req_responce(const QJsonObject& jObj);
    void table_info_update(const QJsonObject& jObj);
    void fail_to_connect();
    void filling_table();
    void send_to_host(const QJsonObject& jObj);
    void show_goose();

private:
    quint16                              m_port;
    quint32                              m_data_size    = 0;
    bool                                 m_message_flag = true;
    QSystemTrayIcon*                     m_tray_icon;
    RestAutoupdater                      m_autoupdater;
    QLabel                               m_goose;
    QString                              m_address;
    QString                              m_name;
    QMap<QString, QPair<QString, QTime>> m_resList; //!< <имя ресурса, <пользователь, время использования>>
    Ui::MainWindow*                      ui;
    MyTableWidget*                       m_table_w = nullptr;
    UserContextConfiguration             m_user_config;
    CommonContextConfiguration           m_common_config;
    QJsonParseError                      jsonErr;
    QTcpSocket*                          socket;
    QTimer                               timer;
    QByteArray                           m_buff;
    quint32                              reconnect_sec = 0;
};

#endif // MAINWINDOW_H
