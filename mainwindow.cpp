#include "mainwindow.h"
#include "ui_mainwindow.h"

// 1) как сделать маштабируемость адекватную, а не константный размер.
// 2) переделать обновление таблицы. Сейчас при дисконекте контент таблицы удаляется, затем при подключении создатся заново. Норм чи не?
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->reconnect_btn->hide(); // Кнопка повторной попытки подключения к серверу
    QStringList headerLabel;
    headerLabel << "Res num" << "Res user";
    ui->tableWidget->setHorizontalHeaderLabels(headerLabel);
    headerLabel << "Busy time" << "Take";
    ui->tableWidget_2->setHorizontalHeaderLabels(headerLabel);
    timer = QSharedPointer<QTimer>(new QTimer);
    timer->setInterval(1000);
    servStartTime = QSharedPointer<QDateTime>(new QDateTime());
    socket = QSharedPointer<QTcpSocket>(new QTcpSocket(this));
    connect(socket.data(), &QTcpSocket::readyRead, this, &MainWindow::slotSockReady);
    connect(socket.data(), &QTcpSocket::disconnected, this, &MainWindow::slotSockDisconnected);
    connect(socket.data(), &QTcpSocket::connected, this, &MainWindow::slotConnected);
    connect(timer.data(), &QTimer::timeout, this, &MainWindow::time_update);
    socket->connectToHost("localhost", 9292);
    // Добавить окошко "Подключение к хосту" которое висит пока не будет сигнал hostFound, затем закрывается.
    statusBar()->showMessage("Waiting for connection to host");
}


MainWindow::~MainWindow()
{
    socket ->close();
    delete ui;
}


void MainWindow::slotConnected(){
    statusBar()->showMessage("Сonnect to host successfully");
    bool ok;
    QString str = QInputDialog::getText(nullptr, "Input", "Name:", QLineEdit::Normal,"Your name", &ok);
    if(ok){
        usrName = str.toLower();
        statusBar()->showMessage("Authorization request");
        QJsonObject jObj;
        jObj.insert("type", "authorization");
        jObj.insert("username", usrName);
        send_to_host(jObj);
   }else{
        qDebug() << "Cancel btn pressed";
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
    // FIXME надо добавить то, что если буфер превышает по размеру какое-то значение полностью его очищать
    auto jDoc = QJsonDocument::fromJson(buff, &jsonErr);
    if(jsonErr.error == QJsonParseError::UnterminatedObject){
        qDebug() << jsonErr.errorString();
        return;
    }
    if(jsonErr.error == QJsonParseError::NoError){
        json_handler(jDoc.object());
        buff.clear();
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

    if(jType == "request_responce")
        req_responce(jObj);

    if(jType == "broadcast")
        table_update(jObj);

    if(jType == "connect_fail")
        fail_to_connect();
}


void MainWindow::autorization(const QJsonObject &jObj){
    statusBar()->showMessage("Autorization successfully");
    QString start_time = jObj["start_time"].toString();
    QJsonArray resNum = jObj["resnum"].toArray();
    QJsonArray resUsr = jObj["resuser"].toArray();
    QJsonArray busyTime = jObj["busyTime"].toArray();
    int dd = static_cast<int>(start_time.left(2).toInt());
    int MM = static_cast<int>(start_time.mid(3, 2).toInt());
    int yyyy = static_cast<int>(start_time.mid(6, 4).toInt());
    int hh = static_cast<int>(start_time.mid(11, 2).toInt());
    int mm = static_cast<int>(start_time.mid(14, 2).toInt());
    int ss = static_cast<int>(start_time.right(4).toInt());
    QDate servDate(yyyy, MM, dd);
    QTime servTime(hh, mm, ss);
    servStartTime->setDate(servDate);
    servStartTime->setTime(servTime);

    QJsonArray::const_iterator usrIdx = resUsr.begin();
    QJsonArray::const_iterator timeIdx = busyTime.begin();
    for(auto i : resNum){
        ui->tableWidget->insertRow(i.toInt());
        ui->tableWidget_2->insertRow(i.toInt());
        QTableWidgetItem a(QCheckBox);
        // настройка чекбокса
        // FIXME надо удалять это говно в деструкторе?
        QWidget *checkBoxWidget = new QWidget();
        QCheckBox *checkBox = new QCheckBox();
        QHBoxLayout *layoutCheckBox = new QHBoxLayout(checkBoxWidget); // слой с привязкой к виджету
        layoutCheckBox->addWidget(checkBox);            // чекбокс в слой
        layoutCheckBox->setAlignment(Qt::AlignCenter);  // Отцентровка чекбокса
        layoutCheckBox->setContentsMargins(0,0,0,0);    // Устанавка нулевых отступов
        checkBox->setChecked(false);
        ui->tableWidget_2->setCellWidget(i.toInt(), 3, checkBoxWidget);
        // FIXME почему то программа крашится если использовать умные указатели. А они нужны тут вобще?
//        QSharedPointer<QWidget> checkBoxWidget = QSharedPointer<QWidget>(new QWidget());
//        QSharedPointer<QCheckBox> checkBox = QSharedPointer<QCheckBox>(new QCheckBox());
//        QSharedPointer<QHBoxLayout> layoutCheckBox = QSharedPointer<QHBoxLayout>(new QHBoxLayout(checkBoxWidget.data())); // слой с привязкой к виджету
//        checkBox->setChecked(false);
//        layoutCheckBox->addWidget(checkBox.data());
//        layoutCheckBox->setAlignment(Qt::AlignCenter);
//        layoutCheckBox->setContentsMargins(0, 0, 0, 0);
//        ui->tableWidget_2->setCellWidget(i.toInt(), 3, checkBoxWidget.data());

        start_time = timeIdx->toString();
        hh = static_cast<int>(start_time.left(2).toInt());
        mm = static_cast<int>(start_time.mid(3, 2).toInt());
        ss = static_cast<int>(start_time.right(2).toInt());
        m_resList.insert( static_cast<quint8>(i.toInt()), new ResInf(usrIdx->toString(), hh, mm, ss) );
        timeIdx++;
        usrIdx++;
    }
    timer->start();
    int row = 0;
    QMap<quint8, ResInf*>::const_iterator i;

    for(i = m_resList.begin(); i != m_resList.end(); ++i){
        // Обновление первой таблицы
        ui->tableWidget->setItem(row, 0, new QTableWidgetItem( QString::number(i.key())) );
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem(i.value()->currenUser));

        // Обновление второй таблицы
        ui->tableWidget_2->setItem(row, 0, new QTableWidgetItem( QString::number(i.key())) );
        ui->tableWidget_2->setItem(row, 1, new QTableWidgetItem(i.value()->currenUser));
        ui->tableWidget_2->setItem(row, 2, new QTableWidgetItem(i.value()->time->toString("hh:mm:ss"))); // FIXME изменить на вычисление времени сколько занимается пользователем. возможно она вообще не тут должны быть.

        row ++;
    }
}


void MainWindow::res_intercept(const QJsonObject &jObj){
    QJsonArray grabResNum = jObj["resnum"].toArray();
    QString respocne = "Resource(s) num:";
    for(auto i : grabResNum){
        respocne += ' ' + i.toString();
    }
    respocne += " were intercepted.";
    statusBar()->showMessage(respocne);
}


void MainWindow::req_responce(const QJsonObject &jObj){
    QJsonArray resNum = jObj["resource"].toArray();
    QJsonArray resStatus = jObj["status"].toArray();
    QString respocne;
    QString take, notTake;
    for(int i = 0; i < resNum.size(); i++){
        if(resStatus[i].toInt() == 1){
            take += resNum[i].toString() + ", ";
        }else{
            notTake += resNum[i].toString() + ", ";
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


void MainWindow::table_update(const QJsonObject &jObj){
    QString usrTime;
    QJsonArray resNum = jObj["resnum"].toArray();
    QJsonArray resUsr = jObj["reuser"].toArray();
    QJsonArray busyTime = jObj["busyTime"].toArray();
    QJsonArray::const_iterator usrIdx = resNum.begin();
    QJsonArray::const_iterator timeIdx = busyTime.begin();
    int hh, mm, ss;

    for(auto i : resNum){
        usrTime = timeIdx->toString();
        hh = static_cast<int>(usrTime.left(2).toInt());
        mm = static_cast<int>(usrTime.mid(3, 2).toInt());
        ss = static_cast<int>(usrTime.right(2).toInt());
        m_resList.insert( static_cast<quint8>(i.toInt()), new ResInf(usrIdx->toString(), hh, mm, ss) );
    }
    filling_table();
}


void MainWindow::fail_to_connect(){
    statusBar()->showMessage("Authorization is not successful. Your IP in ban list");
    ui->reconnect_btn->show();
}


void MainWindow::filling_table(){
    int row = 0;
    QMap<quint8, ResInf*>::const_iterator i;
    QString busyTime;
    for(i = m_resList.begin(); i != m_resList.end(); ++i){
        // Обновление первой таблицы
        ui->tableWidget->item(row, 0)->setData( Qt::DisplayRole, QString::number(i.key()) );
        ui->tableWidget->item(row, 1)->setData(Qt::DisplayRole, i.value()->currenUser);

        // Обновление второй таблицы
        ui->tableWidget_2->item(row, 0)->setData( Qt::DisplayRole, QString::number(i.key()) );
        ui->tableWidget_2->item(row, 1)->setData(Qt::DisplayRole, i.value()->currenUser);
        if(i.value()->currenUser != "Free"){
            busyTime = QString::number(secsPassed/3600) + ":" + QString::number((secsPassed%3600)/60) + ":" + QString::number(secsPassed%60);
            ui->tableWidget_2->item(row, 2)->setData(Qt::DisplayRole, busyTime);
        }else{
            ui->tableWidget_2->item(row, 2)->setData(Qt::DisplayRole, "00:00:00");
        }
        row ++;
    }
        //ui->tableWidget->resizeColumnsToContents();
        //ui->tableWidget_2->resizeColumnsToContents();
}


void MainWindow::on_takeRes_btn_clicked()
{
    quint32 req = 0;  // если будет больше 4 ресурсов, то хрень полная
    QCheckBox *checkBox;
    for(quint8 i=0; i<m_resList.size(); i++){
        checkBox = qobject_cast<QCheckBox*>(ui->tableWidget_2->cellWidget(i, 4));
        if(checkBox->isChecked()){
            req = req | (1 << (i * 8));
            checkBox->setChecked(false);
        }
    }
    QJsonObject jObj;
    jObj.insert("type", "res_request");
    jObj.insert("action", "take");
    jObj.insert("username", usrName);
    jObj.insert("time", QTime::currentTime().toString("hh:mm:ss"));
    jObj.insert("request", QString::number(req));
    send_to_host(jObj);
}


void MainWindow::on_clearRes_btn_clicked()
{
    quint32 req = 0;  // если будет больше 4 ресурсов, то хрень полная
    QCheckBox *checkBox;
    for(quint8 i=0; i<m_resList.size(); i++){
        checkBox = qobject_cast<QCheckBox*>(ui->tableWidget_2->cellWidget(i, 4));
        if(checkBox->isChecked()){
            req = req | (1 << (i * 8));
            checkBox->setChecked(false);
        }
    }
    QJsonObject jObj;
    jObj.insert("type", "res_request");
    jObj.insert("action", "free");
    jObj.insert("username", usrName);
    jObj.insert("time", QTime::currentTime().toString("hh:mm:ss"));
    jObj.insert("request", QString::number(req));
    send_to_host(jObj);
}


void MainWindow::on_clearAllRes_btn_clicked()
{
    quint32 req = 0;  // если будет больше 4 ресурсов, то хрень полная
    for(quint8 i=0; i<m_resList.size(); i++){
        req = req | (1 << (i * 8));
    }
    QJsonObject jObj;
    jObj.insert("type", "clear_res");
    send_to_host(jObj);
}

// FIXME кнопка пропадает, надо поставить таймер на повторное подключение после которого снова разрешать переподключаться.
void MainWindow::on_reconnect_btn_clicked()
{
    socket->connectToHost("localhost", 9292);
    ui->reconnect_btn->hide();
}


void MainWindow::on_setTime_btn_clicked()
{
    QString timeEdit = ui->timeEdit->time().toString(); // format hh:mm:ss
    qint64 secs = timeEdit.left(2).toInt() * 3600 + timeEdit.mid(3, 2).toInt() * 60 + timeEdit.right(2).toInt();
    QJsonObject jObj;
    jObj.insert("action", "occupancy_time");
    jObj.insert("value", QString::number(secs));
    send_to_host(jObj);
}


void MainWindow::on_rejectResReq_chkBox_stateChanged(int arg1)
{
    QJsonObject jObj;
    jObj.insert("action", "reject_res_req");
    jObj.insert("value", arg1);
    send_to_host(jObj);
}


void MainWindow::on_rejectNewConn_chkBox_stateChanged(int arg1)
{
    QJsonObject jObj;
    jObj.insert("action", "reject_connections");
    jObj.insert("value", arg1);
    send_to_host(jObj);
}


void MainWindow::time_update()
{
    if(socket->isValid()){
        dayPassed = servStartTime->daysTo(QDateTime::currentDateTime());  // количество дней работы сервера
        secsPassed = servStartTime->time().secsTo(QTime::currentTime());  // количество секунд работы сервера в текущем дне
        QString labelText = "Время работы сервера: " + QString::number(dayPassed) + "-й день, и "
                + QString::number(secsPassed/3600) + ":" + QString::number((secsPassed%3600)/60) + ":" + QString::number(secsPassed%60) + " часов";
        ui->label->setText(labelText);

        qint64 secs;
        QString busyTime;
        int row = 0;
        QMap<quint8, ResInf*>::const_iterator i;
        for(i = m_resList.begin(); i != m_resList.end(); ++i){
            if(i.value()->currenUser != "Free"){
                secs = i.value()->time->secsTo(QTime::currentTime());
                busyTime = QString::number(secsPassed/3600) + ":" + QString::number((secsPassed%3600)/60) + ":" + QString::number(secsPassed%60);
                ui->tableWidget_2->item(row, 2)->setData(Qt::DisplayRole, busyTime);
            }
            row ++;
        }
        timer->start();
    }else{
        ui->label->setText("Сервер не доступен.");
        ui->tableWidget->clearContents();
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






