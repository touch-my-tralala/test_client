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

    bool ok;
    QString str = QInputDialog::getText(0, "Input", "Name:", QLineEdit::Normal,"Your name", &ok);
    if(ok){
        if(!init(str))
            this->~MainWindow(); // может тут какой-то цикл сделать?
    }else{
        this->~MainWindow();
    }


}

bool MainWindow::init(const QString &str){
    QJsonObject jObj;
    jObj.insert("username", str.toLower());
    QJsonDocument jDoc(jObj);
    socket->connectToHost("localhost", 9292);
    if(socket->waitForConnected(1000)){
        qDebug() << "connect correct";
        socket->write(jDoc.toJson());
        if(socket->waitForReadyRead(5000)){
            qDebug() << "readyRead correct";
            auto recData = QJsonDocument::fromJson(socket->readAll(), &jsonErr);
            QJsonObject json = recData.object();
            if(jsonErr.errorString() == "no error occurred" && json.contains("start_time")){
                servStartTime = json["username"].toString();
                QMessageBox::information(this, "Информация", "Авторизация успешна");
                return true;
            }else{
                qDebug() << "Constructor. Json err " + jsonErr.errorString();
                // Здесь помимо отказа доступа сервер может просто долго отвечать(хоть это и маловероятно)
                // стоит подумать.
                QMessageBox::information(this, "Информация", "В доступе отказано");
                return false;
            }
        }else{
            return false;
        }
    }else{
        QMessageBox::information(this, "Информация", "Сервер недоступен");
        return false;
    }

    // как дождаться ответа от сервера и потом продолжить

}

void MainWindow::slotSockReady(){
    auto jDoc = QJsonDocument::fromJson(socket->readAll(), &jsonErr);
    QJsonObject json = jDoc.object();
    if(jsonErr.errorString() == "no error occurred"){

    }else{
        qDebug() << "Json err " + jsonErr.errorString();
    }
}

void MainWindow::slotSockDisconnected(){

}

MainWindow::~MainWindow()
{
    qDebug() << "distructor";
    delete ui;
}
