#ifndef SENDGOOSEWIDGET_H
#define SENDGOOSEWIDGET_H

#include <QDialog>
#include <QGridLayout>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

class SendGooseWidget : public QDialog
{
    Q_OBJECT

public:
    SendGooseWidget(QWidget* parrent = nullptr);
    QJsonObject getSendObj();

private:
    QLineEdit* m_name;
};

#endif // SENDGOOSEWIDGET_H
