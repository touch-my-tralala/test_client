#ifndef HOSTINPUTDIALOG_H
#define HOSTINPUTDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>

class HostInputDialog : public QDialog
{
    Q_OBJECT

public:
    HostInputDialog(QWidget* parent = nullptr);

    QString getPort(){return m_port->text();}
    QString getAddress(){return m_address->text();}

private:
    QLineEdit* m_port;
    QLineEdit* m_address;
};

#endif // HOSTINPUTDIALOG_H
