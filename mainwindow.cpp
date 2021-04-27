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
    connect(socket.data(), &QTcpSocket::hostFound, this, &MainWindow::slotHostFound);
    socket->connectToHost("localhost", 9292);
    // Добавить окошко "Подключение к хосту" которое висит пока не будет сигнал hostFound, затем закрывается.

}

void MainWindow::slotHostFound(){
    bool ok;
    QString str = QInputDialog::getText(nullptr, "Input", "Name:", QLineEdit::Normal,"Your name", &ok);
    if(ok){
        QJsonObject jObj;
        jObj.insert("username", str.toLower());
        QJsonDocument jDoc(jObj);
        socket->write(jDoc.toJson());
        if(socket->waitForReadyRead()){
            qDebug() << "readyRead correct";
            auto recData = QJsonDocument::fromJson(socket->readAll(), &jsonErr);
            QJsonObject json = recData.object();
            if(jsonErr.errorString() == "no error occurred" && json.contains("start_time")){
                servStartTime = json["username"].toString();
                QMessageBox::information(this, "Информация", "Авторизация успешна");
            }else{
                qDebug() << "Constructor. Json err " + jsonErr.errorString();
                // Здесь помимо отказа доступа сервер может просто долго отвечать(хоть это и маловероятно)
                // стоит подумать.
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
    qDebug() << "destructor";
    socket ->close();
    delete ui;
}
