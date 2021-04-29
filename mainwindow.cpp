#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

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
        QJsonObject jObj;
        jObj.insert("username", str.toLower());
        QJsonDocument jDoc(jObj);
        socket->write(jDoc.toJson());
        if(socket->waitForReadyRead()){
            statusBar()->showMessage("Parse data from host");
            qDebug() << "readyRead correct";
            auto recData = QJsonDocument::fromJson(socket->readAll(), &jsonErr);
            QJsonObject json = recData.object();
            if(jsonErr.errorString() == "no error occurred" && json.contains("start_time")){
                statusBar()->showMessage("Authorization successful");
                servStartTime = json["username"].toString();
            }else{
                qDebug() << "Constructor. Json err " + jsonErr.errorString();
                QMessageBox::information(this, "Информация", "В доступе отказано");
                this->close();
            }
        }else{
            qDebug() << "readyRead uncorrect";
            this->close();
        }
    }else{
        qDebug() <<"Close";
        this->close();
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

    if(jType == "grab_res")

    if(jType == "request_responce")

    if(jType == "broadcast")

}


MainWindow::~MainWindow()
{
    qDebug() << "destructor";
    socket ->close();
    delete ui;
}
