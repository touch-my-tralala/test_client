#include "mainwindow.h"

#include "ui_mainwindow.h"

// 1) возможность сворачиваться в трей (высокий приоритет)
// 3) в меню надо возможность сменить пользователя (заново ввести имя), или просто сбросится до первоначалных настроек.
// 4) нормальная система авторизация (низкий приоритет)

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    init();

    ui->setupUi(this);
    ui->reconnectButton->hide();

    m_table_w = new MyTableWidget(this);
    ui->tableLayout->addWidget(m_table_w);
}

MainWindow::~MainWindow()
{
    m_user_config.writeConfiguration();
    m_common_config.writeConfiguration();
    socket->close();
    delete ui;
}

void MainWindow::init()
{

    build_interface();
    QSettings* sett = new QSettings(QDir::currentPath() + "/" + KEYS::Config().file_name);
    m_user_config.loadConfiguration(sett);
    m_common_config.loadConfiguration(sett);

    m_port = static_cast<quint16>(m_common_config.getConfigParam(KEYS::Config().port).toUInt());
    m_address = m_common_config.getConfigParam(KEYS::Config().address).toString();

    m_name = m_user_config.getConfigParam(KEYS::Config().name).toString();

    // Таймер времени переподключения
    reconnectTimer.setInterval(1000);
    connect(&reconnectTimer, &QTimer::timeout, this, &MainWindow::timeout_recconect);
    reconnectTimer.start();

    // Таймер времени сервера/ресурсов
    timer.setInterval(1000);
    connect(&timer, &QTimer::timeout, this, &MainWindow::time_update);

    // socket
    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::slotSockReady);
    connect(socket, &QTcpSocket::disconnected, this, &MainWindow::slotSockDisconnected);
    connect(socket, &QTcpSocket::connected, this, &MainWindow::slotConnected);

    // Autoupdater
    //m_autoupdater.setUpdateFilePath();
    // зарегистрировать файлы обнолвений

    m_goose.setPixmap(QPixmap("goose.png"));

    socket->connectToHost(m_address, m_port);
    statusBar()->showMessage("Waiting for connection to host");
}

void MainWindow::build_interface()
{
    // init tray icon
    m_tray_icon = new QSystemTrayIcon(this);
    m_tray_icon->setIcon(style()->standardIcon(QStyle::SP_ComputerIcon));
    // FIXME: так можно оставить?
    QMenu* menu        = new QMenu(this);
    QAction* view_window = new QAction("Открыть", this);
    QAction* quit_app    = new QAction("Выход", this);

    connect(view_window, &QAction::triggered, this, &MainWindow::show);
    connect(quit_app, &QAction::triggered, this, &MainWindow::close);
    connect(m_tray_icon, &QSystemTrayIcon::activated, this, &MainWindow::trayIconActivated);

    menu->addAction(view_window);
    menu->addAction(quit_app);
    m_tray_icon->setContextMenu(menu);
}

void MainWindow::closeEvent(QCloseEvent* event)
{

    if (isVisible() && ui->tray_en->isChecked())
    {
        event->ignore();
        this->hide();
        m_tray_icon->show();
        if (m_message_flag)
        {
            QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::MessageIcon(QSystemTrayIcon::Information);
            m_tray_icon->showMessage("Tray Program", "Приложение свернуто в трей", icon, 2000);
            m_message_flag = false;
        }
    }
}

void MainWindow::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
        case QSystemTrayIcon::DoubleClick:
            if (!isVisible())
            {
                this->show();
                m_tray_icon->hide();
            }
            else
            {
                this->hide();
                m_tray_icon->show();
            }
            break;

        default:
            break;
    }
}

void MainWindow::slotConnected()
{
    statusBar()->showMessage("Сonnect to host successfully");
    reconnectTimer.stop();

    // имя считано из конфига
    if (m_name.size() > 0)
    {
        QJsonObject jObj({ { KEYS::Json().user_name, m_name } });
        send_to_host(jObj);
        return;
    }

    bool    ok;
    QString str = QInputDialog::getText(nullptr, "Input", "Name:", QLineEdit::Normal, "Your name", &ok);
    if (ok)
    {
        m_name = str.toLower();
        statusBar()->showMessage("Authorization request");
        QJsonObject jObj({ { KEYS::Json().user_name, m_name } });
        send_to_host(jObj);
    }
    else
    {
        close();
    }
}

void MainWindow::slotSockReady()
{
    QDataStream readStream(socket);
    readStream.setVersion(QDataStream::Qt_5_12);

    while (!readStream.atEnd())
    {
        if (!m_data_size)
        {
            qint32 header_size = sizeof(quint32) + sizeof(quint8);
            if (socket->bytesAvailable() < header_size)
                return;
            readStream >> m_input_data_type;
            readStream >> m_data_size;
        }

        if (m_input_data_type == File_type)
        {
            // обработка файлового запроса
            m_buff.clear();
            m_data_size = 0;
            return;
        }

        if (m_input_data_type == Json_type)
        {
            if (socket->bytesAvailable() < m_data_size)
                return;

            quint8 byte;
            for (quint32 i = 0; i < m_data_size; i++)
            {
                readStream >> byte;
                m_buff.append(byte);
            }

            auto jDoc = QJsonDocument::fromJson(m_buff, &jsonErr);
            if (jsonErr.error == QJsonParseError::NoError)
            {
                auto address = socket->peerAddress();
                json_handler(jDoc.object());
            }
            else
                qDebug() << "Ошибка json-формата" << jsonErr.errorString();

            m_buff.clear();
            m_data_size = 0;
        }
    }
}

void MainWindow::slotSockDisconnected()
{
    if (ui->dropButton->isEnabled())
    {
        ui->dropButton->setEnabled(false);
        ui->takeButton->setEnabled(false);
    }
    statusBar()->showMessage("Lost connection to host.");
    socket->close();
    ui->reconnectButton->show();
}

void MainWindow::json_handler(const QJsonObject& jObj)
{
    auto jType = jObj[KEYS::Json().type].toString();

    if (jType == KEYS::Json().connect_fail)
        fail_to_connect();

    if (jType == KEYS::Json().goose){
        m_goose.show();
        QTimer::singleShot(1000, this, &MainWindow::show_goose);
    }

    if (jType == KEYS::Json().authorization)
        autorization();

    if (jType == KEYS::Json().grab_res)
        res_intercept(jObj);

    if (jType == KEYS::Json().request_responce)
        req_responce(jObj);

    if (jType == KEYS::Json().broadcast)
        table_info_update(jObj);

    table_info_update(jObj);
}

void MainWindow::autorization()
{
    statusBar()->showMessage("Autorization successfully");
    ui->dropButton->setEnabled(true);
    ui->takeButton->setEnabled(true);
    timer.start();
}

void MainWindow::table_info_update(const QJsonObject& jObj)
{
    QJsonArray resArr = jObj[KEYS::Json().resources].toArray();
    QString    user, res;
    qint32     time;
    for (auto i : resArr)
    {
        auto obj = i.toObject();
        res      = obj[KEYS::Json().res_name].toString();
        user     = obj[KEYS::Json().user_name].toString();
        time     = obj[KEYS::Json().time].toInt();
        m_resList.insert(res, { user, QTime().addSecs(time) });
    }
    m_table_w->updateTableData(m_resList);
}

void MainWindow::res_intercept(const QJsonObject& jObj)
{
    auto grabResNum = jObj[KEYS::Json().resources].toArray();

    QString respocne = "Resources num:";
    for (auto i : grabResNum)
        respocne += i.toString() + ", ";

    respocne.remove(respocne.size() - 2, 2);
    respocne += " were intercepted.";
    statusBar()->showMessage(respocne);
}

void MainWindow::req_responce(const QJsonObject& jObj)
{
    auto action = jObj[KEYS::Json().action].toString();
    auto jArr   = jObj[KEYS::Json().resources].toArray();

    QString res, responce = "", take = "", notTake = "";
    bool    answer;
    for (auto i : jArr)
    {
        auto obj = i.toObject();
        res      = obj[KEYS::Json().res_name].toString();
        answer   = obj[KEYS::Json().status].toBool();

        if (answer)
            take += res + ", ";
        else
            notTake += res + ", ";
    }

    if (take.size() > 0)
    {
        take.remove(take.size() - 2, 2);
        responce += "Resourcs num: " + take + " are " + action + " successfully.";
    }
    if (notTake.size() > 0)
    {
        notTake.remove(notTake.size() - 2, 2);
        responce += "Resources num: " + notTake + " access denied.";
    }

    statusBar()->showMessage(responce);
}

void MainWindow::fail_to_connect()
{
    statusBar()->showMessage("Authorization denied. Your IP is in the ban list");
    ui->reconnectButton->show();
}

void MainWindow::timeout_recconect()
{
    reconnect_sec++;
    if (reconnect_sec <= 10)
    {
        if (socket && socket->state() != QTcpSocket::ConnectedState)
        {
            statusBar()->showMessage("Recconect time(max 10 sec): " + QString::number(reconnect_sec) + " s...");
            reconnectTimer.start();
        }
        else
        {
            reconnect_sec = 0;
        }
    }
    else
    {
        ui->reconnectButton->show();
        statusBar()->showMessage("Reconnect failing :(");
        reconnectTimer.stop();
    }
}

void MainWindow::time_update()
{
    if (socket->state() == QTcpSocket::ConnectedState)
    {
        int secs;
        for (auto i = m_resList.begin(); i != m_resList.end(); ++i)
        {
            if (i.value().first != KEYS::Common().no_user)
            {
                secs = i.value().second.secsTo(QTime::currentTime());
                m_table_w->updateBusyTime(i.key(), secs);
            }
        }
        timer.start();
    }
    else
    {
        statusBar()->showMessage("Сервер не доступен.");
        timer.stop();
    }
}

void MainWindow::send_to_host(const QJsonObject& jObj)
{
    if (socket->state() == QTcpSocket::ConnectedState)
    {
        QDataStream sendStream(socket);
        sendStream.setVersion(QDataStream::Qt_5_12);
        sendStream << QJsonDocument(jObj).toJson(QJsonDocument::Compact);
    }
    else
    {
        qDebug() << "Socket not connected";
    }
}

void MainWindow::on_takeButton_clicked()
{
    auto selected_list = m_table_w->getSelected();

    if (selected_list.size() > 0)
    {
        QJsonArray jArr;
        for (auto i : selected_list)
            jArr << i;

        qint32 curTime = QTime(0, 0, 0).secsTo(QTime::currentTime());

        QJsonObject jObj({ { KEYS::Json().action, KEYS::Json().take },
                           { KEYS::Json().user_name, m_name },
                           { KEYS::Json().time, curTime },
                           { KEYS::Json().resources, jArr } });
        send_to_host(jObj);
    }
}

void MainWindow::on_dropButton_clicked()
{
    auto selected_list = m_table_w->getSelected();

    if (selected_list.size() > 0)
    {
        QJsonArray jArr;
        for (const auto& i : qAsConst(selected_list))
            jArr << i;

        QJsonObject jObj({ { KEYS::Json().action, KEYS::Json().drop },
                           { KEYS::Json().user_name, m_name },
                           { KEYS::Json().resources, jArr } });
        send_to_host(jObj);
    }
}

void MainWindow::on_reconnectButton_clicked()
{
    reconnect_sec = 0;
    socket->connectToHost(m_address, m_port);
    reconnectTimer.start();
    ui->reconnectButton->hide();
}

void MainWindow::on_change_name_triggered()
{
    bool    ok;
    QString str = QInputDialog::getText(nullptr, "Input", "Имя:", QLineEdit::Normal, "Введите имя", &ok);
    if (ok)
    {
        m_name = str.toLower();
        statusBar()->showMessage("Authorization request");
        QJsonObject jObj({ { KEYS::Json().user_name, m_name } });
        send_to_host(jObj);
    }
}

void MainWindow::on_change_host_triggered()
{
    QSharedPointer<HostInputDialog> h_dialog = QSharedPointer<HostInputDialog>(new HostInputDialog);
    if (h_dialog->exec() == QDialog::Accepted)
    {
        m_address = h_dialog->getAddress();
        m_port    = static_cast<quint16>(h_dialog->getPort().toInt());

        if (socket->state() == QTcpSocket::ConnectedState)
            socket->disconnectFromHost();

        socket->connectToHost(m_address, m_port);
    }
}

void MainWindow::show_goose()
{
    m_goose.hide();
}

void MainWindow::on_send_goose_triggered()
{
    QSharedPointer<SendGooseWidget> goose_widget = QSharedPointer<SendGooseWidget>(new SendGooseWidget);
    if (goose_widget->exec() == QDialog::Accepted)
    {
        auto obj = goose_widget->getSendObj();
        send_to_host(obj);
    }
}
