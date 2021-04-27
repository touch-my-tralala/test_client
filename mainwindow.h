#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QMessageBox>
#include <QtCore>
#include <QInputDialog>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void slotSockReady();
    void slotSockDisconnected();
    void slotHostFound();

private:
    bool init(const QString &str);


private:
    Ui::MainWindow *ui;
    QSharedPointer<QTcpSocket> socket;
    QJsonParseError jsonErr;
    QString servStartTime;

};

#endif // MAINWINDOW_H
