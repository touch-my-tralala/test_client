#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QStringList headerLabel;
    headerLabel << "Res num" << "Res user";
    ui->tableWidget->setHorizontalHeaderLabels(headerLabel);

    socket = QSharedPointer<QTcpSocket>(new QTcpSocket(this));
    connect(socket.data(), &QTcpSocket::readyRead, this, &MainWindow::slotSockReady);
    connect(socket.data(), &QTcpSocket::disconnected, this, &MainWindow::slotSockDisconnected);
    connect(socket.data(), &QTcpSocket::connected, this, &MainWindow::slotConnected);
    socket->connectToHost("localhost", 6666);
    // Добавить окошко "Подключение к хосту" которое висит пока не будет сигнал hostFound, затем закрывается.
    statusBar()->showMessage("Waiting for connection to host");
}


void MainWindow::slotConnected(){
    statusBar()->showMessage("Сonnect to host successfully");
    bool ok;
    QString str = QInputDialog::getText(nullptr, "Input", "Name:", QLineEdit::Normal,"Your name", &ok);
    if(ok){
        statusBar()->showMessage("Authorization request");
        QJsonObject jObj;
        jObj.insert("username", str.toLower());
        QJsonDocument jDoc(jObj);
        socket->write(jDoc.toJson());
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
    if(jsonErr.errorString() == QJsonParseError::UnterminatedObject){
        return;
    }
    if(jsonErr.errorString() == QJsonParseError::NoError){
        json_handler(jDoc.object());
        buff.clear();
    }
}


void MainWindow::slotSockDisconnected(){

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
    QJsonArray resUsr = jObj["reuser"].toArray();
    QJsonArray busyTime = jObj["busyTime"].toArray();
    int dd = static_cast<int>(start_time.left(2).toInt());
    int MM = static_cast<int>(start_time.mid(3, 2).toInt());
    int yyyy = static_cast<int>(start_time.mid(6, 4).toInt());
    int hh = static_cast<int>(start_time.mid(11, 2).toInt());
    int mm = static_cast<int>(start_time.mid(14, 2).toInt());
    int ss = static_cast<int>(start_time.right(4).toInt());
    QDate servDate(yyyy, MM, dd);
    QTime servTime(hh, mm, ss);
    servStartTime = QSharedPointer<QDateTime>(new QDateTime(servDate, servTime));

    QJsonArray::const_iterator usrIdx = resNum.begin();
    QJsonArray::const_iterator timeIdx = busyTime.begin();
    for(auto i : resNum){
        start_time = timeIdx->toString();
        hh = static_cast<int>(start_time.left(2).toInt());
        mm = static_cast<int>(start_time.mid(3, 2).toInt());
        ss = static_cast<int>(start_time.right(2).toInt());
        m_resList.insert( i.toInt(), new ResInf(usrIdx->toString(), hh, mm, ss) );
    }
}


void MainWindow::res_intercept(const QJsonObject &jObj){

}


void MainWindow::req_responce(const QJsonObject &jObj){

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
        m_resList.insert( i.toInt(), new ResInf(usrIdx->toString(), hh, mm, ss) );
    }
}


void MainWindow::fail_to_connect(){
    statusBar()->showMessage("Authorization is not successful. Your IP in ban list");
}


void MainWindow::filling_table(){
    int row = 0;
    QMap<quint8, ResInf*>::const_iterator i;

    for(i = m_resList.begin(); i != m_resList.end(); ++i){ // FIXME как получить количество колонок?
        // Обновление первой таблицы
        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(i.key()));
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem(i.value()->currenUser));

        // Обновление второй таблицы
        ui->tableWidget_2->setItem(row, 0, new QTableWidgetItem(i.key()));
        ui->tableWidget_2->setItem(row, 1, new QTableWidgetItem(i.value()->currenUser));
        ui->tableWidget_2->setItem(row, 2, new QTableWidgetItem(i.value()->time->toString("hh:mm:ss"))); // FIXME изменить на вычисление времени сколько занимается пользователем. возможно она вообще не тут должны быть.

        row ++;
    }
}

MainWindow::~MainWindow()
{
    qDebug() << "destructor";
    socket ->close();
    delete ui;
}
