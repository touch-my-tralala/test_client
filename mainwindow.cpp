#include "mainwindow.h"
#include "ui_mainwindow.h"


// 1) возможность сворачиваться в трей (высокий приоритет)
// 3) в меню надо возможность сменить пользователя (заново ввести имя), или просто сбросится до первоначалных настроек.
// 4) нормальная система авторизация (низкий приоритет)

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ini_parse("settings.ini");

    m_tray_icon = new QSystemTrayIcon(this);
    m_tray_icon->setIcon(style()->standardIcon(QStyle::SP_ComputerIcon));
    menu = new QMenu(this);
    view_window = new QAction("Открыть", this);
    quit_app = new QAction("Выход", this);

    connect(view_window, &QAction::triggered, this, &MainWindow::show);
    connect(quit_app, &QAction::triggered, this, &MainWindow::close);
    menu->addAction(view_window);
    menu->addAction(quit_app);

    m_tray_icon->setContextMenu(menu);
    m_tray_icon->show();

    connect(m_tray_icon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(on_tray_icon_clicked(QSystemTrayIcon::ActivationReason)));


    ui->setupUi(this);
    ui->reconnectButton->hide();

    m_table_w = new MyTableWidget();
    ui->tableLayout->addWidget(m_table_w);

    // Таймер времени переподключения
    reconnectTimer = QSharedPointer<QTimer>(new QTimer);
    reconnectTimer->setInterval(1000);
    connect(reconnectTimer.data(), &QTimer::timeout, this, &MainWindow::timeout_recconect);
    reconnectTimer->start();

    // Таймер времени сервера/ресурсов
    timer = QSharedPointer<QTimer>(new QTimer());
    timer->setInterval(1000);
    connect(timer.data(), &QTimer::timeout, this, &MainWindow::time_update);

    socket = QSharedPointer<QTcpSocket>(new QTcpSocket(this));
    connect(socket.data(), &QTcpSocket::readyRead, this, &MainWindow::slotSockReady);
    connect(socket.data(), &QTcpSocket::disconnected, this, &MainWindow::slotSockDisconnected);
    connect(socket.data(), &QTcpSocket::connected, this, &MainWindow::slotConnected);

    socket->connectToHost(m_address, m_port);
    statusBar()->showMessage("Waiting for connection to host");
}


MainWindow::~MainWindow()
{
    sett->beginGroup(KEYS::Config().settings);
    sett->setValue(KEYS::Config().port, m_port);
    sett->setValue(KEYS::Config().address, m_address.toString());
    sett->endGroup();

    sett->beginGroup(KEYS::Config().user_settings);
    sett->setValue(KEYS::Config().name, m_name);
    sett->endGroup();

    socket ->close();
    delete m_table_w;
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event){

    if(isVisible()){
        event->ignore();
        this->hide();
        QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::MessageIcon(QSystemTrayIcon::Information);
        m_tray_icon->showMessage("Tray Program", "Приложение свернуто в трей", icon, 2000);
    }
}
void MainWindow::on_tray_icon_clicked(QSystemTrayIcon::ActivationReason reason){
    switch (reason) {
    case QSystemTrayIcon::Trigger:
        if(!isVisible())
            show();
        else
            hide();

        break;

    default:
        break;
    }
}




void MainWindow::slotConnected(){
    statusBar()->showMessage("Сonnect to host successfully");
    reconnectTimer->stop();

    // имя считано из конфига
    if(m_name.size() > 0){
        QJsonObject jObj({{KEYS::Json().user_name, m_name}});
        send_to_host(jObj);
        return;
    }

    bool ok;
    QString str = QInputDialog::getText(nullptr, "Input", "Name:", QLineEdit::Normal,"Your name", &ok);
    if(ok){
        m_name = str.toLower();
        statusBar()->showMessage("Authorization request");
        QJsonObject jObj({{KEYS::Json().user_name, m_name}});
        send_to_host(jObj);
   }else{
        close();
   }
}


void MainWindow::slotSockReady()
{

    qint64 curByteNum = socket->bytesAvailable();
    qDebug();
    curByteNum = socket->bytesAvailable();
    if(curByteNum <= READ_BLOCK_SIZE){
        buff.append(socket->read(curByteNum));
    }else{        
        for(int i=0, e =curByteNum/READ_BLOCK_SIZE; i<= e; i++)
            buff.append( socket->read(READ_BLOCK_SIZE));
    }

    // переполнение буфера
    if(buff.size()*sizeof(buff[0]) > 1000000){
        qDebug() << "Buffer size > 1 Mb";
        buff.clear();
        return;
    }

    QJsonDocument jDoc = QJsonDocument::fromJson(buff, &jsonErr);
    if(jsonErr.error == QJsonParseError::NoError){
         json_handler(jDoc.object());
    }else{
         qDebug() << jsonErr.errorString();
         buff.clear();
         return;
    }
    buff.clear();
}





void MainWindow::slotSockDisconnected()
{
    statusBar()->showMessage("Lost connection to host.");
    socket->close();
    ui->reconnectButton->show();
}


void MainWindow::json_handler(const QJsonObject &jObj)
{
    auto jType = jObj[KEYS::Json().type].toString();

    if(jType == KEYS::Json().connect_fail)
        fail_to_connect();

    if(jType == KEYS::Json().authorization)
        autorization();

    if(jType == KEYS::Json().grab_res)
        res_intercept(jObj);

    if(jType == KEYS::Json().request_responce)
        req_responce(jObj);

    if(jType == KEYS::Json().broadcast)
        table_info_update(jObj);

    table_info_update(jObj);
}

void MainWindow::ini_parse(const QString &fname){
    qDebug() << QDir::currentPath() + "/" + fname;
    sett = new QSettings(QDir::currentPath() + "/" + fname, QSettings::IniFormat);
    sett->beginGroup(KEYS::Config().settings);
    m_port = static_cast<quint16>(sett->value(KEYS::Config().port, 9292).toUInt());
    QString addres= sett->value(KEYS::Config().address, "localhost").toString();
    if(!m_address.setAddress(addres) || addres == "localhost")
        qDebug() << "ip addres incorrect";
    sett->endGroup();

    sett->beginGroup(KEYS::Config().user_settings);
    m_name = sett->value(KEYS::Config().name).toString();
    sett->endGroup();
}

void MainWindow::autorization()
{
    statusBar()->showMessage("Autorization successfully");
    timer->start();
}

void MainWindow::table_info_update(const QJsonObject &jObj)
{
    QJsonArray resArr = jObj[KEYS::Json().resources].toArray();
    QString user, res;
    qint32 time;
    for(auto i : resArr){
        auto obj = i.toObject();
        res = obj[KEYS::Json().res_name].toString();
        user = obj[KEYS::Json().user_name].toString();
        time = obj[KEYS::Json().time].toInt();
        m_resList.insert(res, {user, QTime().addSecs(time)});
    }
    m_table_w->updateTableData(m_resList);
}

void MainWindow::res_intercept(const QJsonObject &jObj){
    auto grabResNum = jObj[KEYS::Json().resources].toArray();

    QString respocne = "Resources num:";
    for(auto i : grabResNum)
        respocne += i.toString() + ", ";

    respocne.remove(respocne.size()-2, 2);
    respocne += " were intercepted.";
    statusBar()->showMessage(respocne);
}

void MainWindow::req_responce(const QJsonObject &jObj)
{
    auto action = jObj[KEYS::Json().action].toString();
    auto jArr = jObj[KEYS::Json().resources].toArray();

    QString res, responce = "", take = "", notTake = "";
    bool answer;
    for(auto i : jArr){
        auto obj = i.toObject();
        res = obj[KEYS::Json().res_name].toString();
        answer = obj[KEYS::Json().status].toBool();

        if(answer)
            take += res + ", ";
        else
            notTake += res + ", ";
    }

    if(take.size() > 0){
        take.remove(take.size()-2, 2);
        responce += "Resourcs num: " + take + " are " + action  + " successfully.";
    }
    if(notTake.size() > 0){
        notTake.remove(notTake.size()-2, 2);
        responce += "Resources num: " + notTake + " access denied.";
    }

    statusBar()->showMessage(responce);
}

void MainWindow::fail_to_connect()
{
    statusBar()->showMessage("Authorization denied. Your IP is in the ban list");
    ui->reconnectButton->show();
}

void MainWindow::timeout_recconect(){
    reconnect_sec++;
    if(reconnect_sec <= 10){
        if(socket && socket->state() != QTcpSocket::ConnectedState){
            socket->connectToHost("localhost", 9292);
            statusBar()->showMessage("Recconect time(max 10 sec): " + QString::number(reconnect_sec) + " s...");
            reconnectTimer->start();
        }else{
            reconnect_sec = 0;
        }
    }else{
        ui->reconnectButton->show();
        statusBar()->showMessage("Reconnect failing :(");
        reconnectTimer->stop();
    }
}


void MainWindow::time_update()
{
    if(socket->state() == QTcpSocket::ConnectedState){
        QString busyTime;
        int secs;
        for(auto i = m_resList.begin(); i != m_resList.end(); ++i){
            if(i.value().first != KEYS::Common().no_user){
                secs = i.value().second.secsTo(QTime::currentTime());
                m_table_w->updateBusyTime(i.key(), secs);
            }
        }
        timer->start();
    }else{
        statusBar()->showMessage("Сервер не доступен.");
        timer->stop();
    }
}


void MainWindow::send_to_host(const QJsonObject &jObj){
    if(socket->state() == QTcpSocket::ConnectedState){
        QJsonDocument jDoc(jObj);
        socket->write(jDoc.toJson());
    }else{
        qDebug() << "Socket not connected";
    }
}


void MainWindow::on_takeButton_clicked()
{
    auto selected_list = m_table_w->getSelected();

    if(selected_list.size() > 0){
        QJsonArray jArr;
        for(auto i: selected_list)
            jArr << i;

        QJsonObject jObj({{KEYS::Json().action, KEYS::Json().take},
                          {KEYS::Json().user_name, m_name},
                          {KEYS::Json().time, QTime::currentTime().second()},
                          {KEYS::Json().resources, jArr}
                         });
        send_to_host(jObj);
    }
}

void MainWindow::on_dropButton_clicked()
{
    auto selected_list = m_table_w->getSelected();

    if(selected_list.size() > 0){
        QJsonArray jArr;
        for(auto i: selected_list)
            jArr << i;

        QJsonObject jObj({{KEYS::Json().action, KEYS::Json().drop},
                          {KEYS::Json().user_name, m_name},
                          {KEYS::Json().resources, jArr}
                         });
        send_to_host(jObj);
    }
}

void MainWindow::on_reconnectButton_clicked()
{
    reconnect_sec = 0;
    reconnectTimer->start();
    ui->reconnectButton->hide();
}
