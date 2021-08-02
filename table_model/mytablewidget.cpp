#include "mytablewidget.h"

MyTableWidget::MyTableWidget(QWidget *parent) : QTableView(parent)
{
    setModel(m_model = new TableModel(this));
    resizeColumnsToContents();
    setSelectionMode(QAbstractItemView::NoSelection);
    setFocusPolicy(Qt::NoFocus);
}

void MyTableWidget::updateTableData(const QMap<QString, QPair<QString, QTime> > &tableData)
{
    QString res, usr, busy_time;
    qint32 secs;

    m_model->removeAllRows();
    for(auto i = tableData.begin(), e = tableData.end(); i != e; i++){
        res = i.key();
        usr = i.value().first;
        secs = i.value().second.secsTo(QTime::currentTime());
        busy_time = QString::number(secs/3600) + ":" + QString::number((secs%3600)/60) + ":" + QString::number(secs%60);
        m_model->appendRes(res);
        m_model->setUser(res, usr);
        m_model->setTime(res, busy_time);
    }
    resizeColumnsToContents();
}

void MyTableWidget::updateBusyTime(const QString &resName, const int &secs)
{
    auto busyTime = QString::number(secs/3600) + ":" + QString::number((secs%3600)/60) + ":" + QString::number(secs%60);
    m_model->setTime(resName, busyTime);
}

