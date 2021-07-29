#ifndef MYTABLEWIDGET_H
#define MYTABLEWIDGET_H

#include <QWidget>
#include <QTableView>

#include "table_model/tablemodel.h"

class MyTableWidget : QTableView
{
    Q_OBJECT

public:
    MyTableWidget(QWidget *parent = nullptr);
    bool appendRes(const QString& resName){ return m_model->appendRes(resName);}
    bool setUser(const QString& resName, const QString& usrName){ return m_model->setUser(resName, usrName);}
    bool setTime(const QString& resName, const QString& resTime){ return m_model->setTime(resName, resTime);}

private:
    TableModel* m_model;
};

#endif // MYTABLEWIDGET_H
