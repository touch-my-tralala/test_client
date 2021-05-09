#include "mainwindow.h"
#include "ui_mainwindow.h"


// 1) как сделать маштабируемость адекватную, а не константный размер.

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->reconnect_btn->hide();
    QStringList headerLabel;
    headerLabel << "Res num" << "Res user" << "Busy time" << "Take";
    ui->tableWidget_2->setHorizontalHeaderLabels(headerLabel);

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

    socket->connectToHost("localhost", 9292);
    statusBar()->showMessage("Waiting for connection to host");
}


MainWindow::~MainWindow()
{
    socket ->close();
    delete ui;
}


void MainWindow::slotConnected(){
    statusBar()->showMessage("Сonnect to host successfully");
    reconnectTimer->stop();
    bool ok;
    QString str = QInputDialog::getText(nullptr, "Input", "Name:", QLineEdit::Normal,"Your name", &ok);
    if(ok){
        usrName = str.toLower();
        statusBar()->showMessage("Authorization request");
        QJsonObject jObj;
        jObj.insert("username", usrName);
        send_to_host(jObj);
   }else{
        close();
    }
}


void MainWindow::slotSockReady(){
    qint64 curByteNum = socket->bytesAvailable();
    if(curByteNum <= READ_BLOCK_SIZE){
        buff.append(socket->read(curByteNum));
    }else{
        for(int i=0; i<=curByteNum/READ_BLOCK_SIZE; i=i+READ_BLOCK_SIZE){
            buff.append( socket->read(READ_BLOCK_SIZE) );
        }
    }
    // переполнение буфера
    if(buff.size()*sizeof(buff[0]) > 1000000){
        qDebug() << "Buffer size > 1 Mb";
        buff.clear();
        return;
    }

    qint64 cnt = buff.count('}');
    for (auto i = 0; i < cnt; i++){
        qint64 idx = buff.indexOf('}');
        if(idx != -1){
            QJsonDocument jDoc = QJsonDocument::fromJson(buff.left(idx+2), &jsonErr);
            if(jsonErr.error == QJsonParseError::NoError){
                json_handler(jDoc.object());
                buff.remove(0, idx+2);
            }else{
                qDebug() << jsonErr.errorString();
            }
        }else{
            qDebug() << "buff not contained '}' char";
        }
    }
}


void MainWindow::slotSockDisconnected(){
    statusBar()->showMessage("Lost connection to host.");
    socket->close();
    ui->reconnect_btn->show();
}


void MainWindow::json_handler(const QJsonObject &jObj){
    auto jType = jObj["type"].toString();
    if(jType == "authorization")
        autorization(jObj);

    if(jType == "grab_res")
        res_intercept(jObj);

    if(jType == "request_responce"){
        if(jObj["action"].toString() == "take")
            req_responce_take(jObj);
        else if(jObj["action"].toString() == "free")
            req_responce_free(jObj);
        else
            qDebug() << "res_request not contain action: " + jObj["action"].toString();
    }

    if(jType == "broadcast")
        table_update(jObj);

    if(jType == "connect_fail")
        fail_to_connect();
}


void MainWindow::autorization(const QJsonObject &jObj){
    statusBar()->showMessage("Autorization successfully");
    QJsonArray resNum = jObj["resnum"].toArray();
    QJsonArray resUsr = jObj["resuser"].toArray();
    QJsonArray busyTime = jObj["busyTime"].toArray();
    int hh, mm, ss;
    QString res_time;
    QJsonArray::const_iterator usrIdx = resUsr.begin();
    QJsonArray::const_iterator timeIdx = busyTime.begin();
    for(auto i : resNum){
        ui->tableWidget_2->insertRow(i.toInt());

        // FIXME почему то программа крашится если использовать умные указатели. А они нужны тут вобще?
//        QSharedPointer<QWidget> checkBoxWidget = QSharedPointer<QWidget>(new QWidget());
//        QSharedPointer<QCheckBox> checkBox = QSharedPointer<QCheckBox>(new QCheckBox());
//        QSharedPointer<QHBoxLayout> layoutCheckBox = QSharedPointer<QHBoxLayout>(new QHBoxLayout(checkBoxWidget.data())); // слой с привязкой к виджету
//        checkBox->setChecked(false);
//        layoutCheckBox->addWidget(checkBox.data());
//        layoutCheckBox->setAlignment(Qt::AlignCenter);
//        layoutCheckBox->setContentsMargins(0, 0, 0, 0);
//        ui->tableWidget_2->setCellWidget(i.toInt(), 3, checkBoxWidget.data());

        res_time = timeIdx->toString();
        hh = static_cast<int>(res_time.left(2).toInt());
        mm = static_cast<int>(res_time.mid(3, 2).toInt());
        ss = static_cast<int>(res_time.right(2).toInt());
        m_resList.insert( static_cast<quint8>(i.toInt()), new ResInf(usrIdx->toString(), hh, mm, ss) );
        timeIdx++;
        usrIdx++;
    }
    timer->start();

    int row = 0;
    for(auto i = m_resList.begin(); i != m_resList.end(); ++i){
        // заполнение таблицы
        ui->tableWidget_2->setItem(row, 0, new QTableWidgetItem( QString::number(i.key())) );
        ui->tableWidget_2->setItem(row, 1, new QTableWidgetItem(i.value()->currenUser));
        ui->tableWidget_2->setItem(row, 2, new QTableWidgetItem(i.value()->time->toString("hh:mm:ss")));

        // настройка чекбокса
        // FIXME надо удалять это говно в деструкторе?
        QWidget *checkBoxWidget = new QWidget();
        QCheckBox *checkBox = new QCheckBox();
        QHBoxLayout *layoutCheckBox = new QHBoxLayout(checkBoxWidget); // слой с привязкой к виджету
        layoutCheckBox->addWidget(checkBox);            // чекбокс в слой
        layoutCheckBox->setAlignment(Qt::AlignCenter);  // Отцентровка чекбокса
        layoutCheckBox->setContentsMargins(0,0,0,0);    // Устанавка нулевых отступов
        checkBox->setChecked(false);
        ui->tableWidget_2->setCellWidget(row, 3, checkBoxWidget);

        row ++;
    }
}


void MainWindow::res_intercept(const QJsonObject &jObj){
    QJsonArray grabResNum = jObj["resource"].toArray();
    QString respocne = "Resource(s) num:";
    for(auto i : grabResNum){
        respocne += QString::number(i.toInt()) + ", ";
    }
    respocne.remove(respocne.size()-2, 2);
    respocne += " were intercepted.";
    statusBar()->showMessage(respocne);
}


void MainWindow::req_responce_take(const QJsonObject &jObj){
    QJsonArray resReq = jObj["resource_responce"].toArray();
    QJsonArray resStatus = jObj["status"].toArray();
    QString respocne;
    QString take, notTake;
    for(auto i = 0; i < resReq.size(); i++){
        if(resStatus[i].toInt() == 1){
            take += QString::number(resReq[i].toInt()) + ", ";
        }else{
            notTake += QString::number(resReq[i].toInt()) + ", ";
        }
    }
    if(take.size() > 0){
        take.remove(take.size()-2, 2);
        respocne += "Resources(s) num: " + take + " are busy successfully.";
    }
    if(notTake.size() > 0){
        notTake.remove(notTake.size()-2, 2);
        respocne += "Resources(s) num: " + notTake + " access denied.";
    }
    statusBar()->showMessage(respocne);
}


void MainWindow::req_responce_free(const QJsonObject &jObj){
    QJsonArray resReq = jObj["resource_responce"].toArray();
    QJsonArray resStatus = jObj["status"].toArray();
    QString respocne;
    QString free, notFree;
    for(int i = 0; i < resReq.size(); i++){
        if(resStatus[i].toInt() == 1){
            free += QString::number(resReq[i].toInt()) + ", ";
        }else{
            notFree += QString::number(resReq[i].toInt()) + ", ";
        }
    }
    if(free.size() > 0){
        free.remove(free.size()-2, 2);
        respocne += "Resources(s) num: " + free + " are free successfully.";
    }
    if(notFree.size() > 0){
        notFree.remove(notFree.size()-2, 2);
        respocne += "Resources(s) num: " + notFree + " access denied.";
    }
    statusBar()->showMessage(respocne);
}


void MainWindow::table_update(const QJsonObject &jObj){
    QString usrTime;
    QJsonArray resUsr = jObj["resuser"].toArray();
    QJsonArray busyTime = jObj["busyTime"].toArray();
    int hh, mm, ss;
    for(quint8 i = 0; i <m_resList.size(); i++){
        usrTime = busyTime[i].toString();
        hh = static_cast<int>(usrTime.left(2).toInt());
        mm = static_cast<int>(usrTime.mid(3, 2).toInt());
        ss = static_cast<int>(usrTime.right(2).toInt());
        m_resList[i]->currenUser = resUsr[i].toString();
        m_resList[i]->time->setHMS(hh, mm, ss);
    }
    filling_table();
}


void MainWindow::fail_to_connect(){
    statusBar()->showMessage("Authorization is not successful. Your IP in ban list");
    ui->reconnect_btn->show();
}


void MainWindow::filling_table(){
    int row = 0;
    QString busyTime;
    for(auto i = m_resList.begin(); i != m_resList.end(); ++i){
        // Обновление второй таблицы
        ui->tableWidget_2->item(row, 0)->setData( Qt::DisplayRole, QString::number(i.key()) );
        ui->tableWidget_2->item(row, 1)->setData(Qt::DisplayRole, i.value()->currenUser);
        if(i.value()->currenUser != "Free"){
            secsPassed = i.value()->time->secsTo(QTime::currentTime());
            busyTime = QString::number(secsPassed/3600) + ":" + QString::number((secsPassed%3600)/60) + ":" + QString::number(secsPassed%60);            
            ui->tableWidget_2->item(row, 2)->setData(Qt::DisplayRole, busyTime);
        }else{
            ui->tableWidget_2->item(row, 2)->setData(Qt::DisplayRole, "00:00:00");
        }
        row ++;
    }
}


void MainWindow::on_takeRes_btn_clicked()
{
    qint64 req = 0;  // если будет больше 4 ресурсов, то хрень полная
    QCheckBox *checkBox = nullptr;
    for(quint8 i=0; i<m_resList.size(); ++i){
        checkBox = ui->tableWidget_2->cellWidget(i, 3)->findChild<QCheckBox*>();
        if(checkBox && checkBox->isChecked()){
            req = req | (1 << (i * 8));
            checkBox->setChecked(false);
        }
    }
    if(req){        
        QTime time(0, 0, 0);
        QJsonObject jObj;
        jObj.insert("action", "take");
        jObj.insert("username", usrName);
        jObj.insert("time", time.secsTo(QTime::currentTime())); // не верно
        jObj.insert("request", req);
        send_to_host(jObj);
    }
    checkBox = nullptr; //  FIXME: это надо делать чи нет?
}


void MainWindow::on_clearRes_btn_clicked()
{
    qint64 req = 0;  // если будет больше 4 ресурсов, то хрень полная
    QCheckBox *checkBox = nullptr;
    for(quint8 i=0; i<m_resList.size(); i++){
        checkBox = ui->tableWidget_2->cellWidget(i, 3)->findChild<QCheckBox*>();
        if(checkBox && checkBox->isChecked()){
            req = req | (1 << (i * 8));
            checkBox->setChecked(false);
        }
    }
    if(req){
        QJsonObject jObj;
        jObj.insert("action", "free");
        jObj.insert("username", usrName);
        jObj.insert("request", req);
        send_to_host(jObj);
    }
    checkBox = nullptr;
}


void MainWindow::on_reconnect_btn_clicked()
{
    reconnect_sec = 0;
    reconnectTimer->start();
    ui->reconnect_btn->hide();
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
        ui->reconnect_btn->show();
        statusBar()->showMessage("Reconnect failing :(");
        reconnectTimer->stop();
    }
}


void MainWindow::time_update()
{
    if(socket->state() == QTcpSocket::ConnectedState){
        // время ресурсов
        QString busyTime;
        int row = 0;
        for(auto i = m_resList.begin(); i != m_resList.end(); ++i){
            if(i.value()->currenUser != "Free"){
                secsPassed = i.value()->time->secsTo(QTime::currentTime());
                busyTime = QString::number(secsPassed/3600) + ":" + QString::number((secsPassed%3600)/60) + ":" + QString::number(secsPassed%60);
                ui->tableWidget_2->item(row, 2)->setData(Qt::DisplayRole, busyTime);
            }
            row ++;
        }
        timer->start();
    }else{
        statusBar()->showMessage("Сервер не доступен.");
        ui->tableWidget_2->clearContents();
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






