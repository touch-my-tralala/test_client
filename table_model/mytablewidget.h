#ifndef MYTABLEWIDGET_H
#define MYTABLEWIDGET_H

#include <QWidget>
#include <QTableView>
#include <QTime>

#include "table_model/tablemodel.h"

class MyTableWidget : public QTableView
{
    Q_OBJECT

public:
    MyTableWidget(QWidget *parent = nullptr);
    //! \brief Изменение данных в таблице
    //! \param[tableData] <имя ресурса, <имя пользователя, время>>
    void updateTableData(const QMap<QString, QPair<QString, QTime>> &tableData);
    //! \brief Изменение времени текущего использования ресурса
    //! \param[secs] время в секундах, прошедшее с момента занятия ресурса
    void updateBusyTime(const QString &resName, const int &secs);
    //! \brief Запрос выделенных строк таблицы
    //! \return список выделенных строк таблицы
    QStringList getSelected(){ return m_model->getSelected();}

private:
    TableModel* m_model;
};

#endif // MYTABLEWIDGET_H
