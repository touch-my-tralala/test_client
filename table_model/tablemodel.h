#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include <QAbstractTableModel>
#include <QDebug>

class TableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    TableModel(QObject *parent = nullptr);
    ~TableModel();

    int           rowCount(const QModelIndex& parent) const override;
    int           columnCount(const QModelIndex& parent) const override;
    QVariant      data(const QModelIndex& index, int role) const override;
    bool          setData(const QModelIndex& index, const QVariant& value, int role) override;
    QVariant      headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool          appendRes(const QString& resName);
    bool          setUser(const QString& resName, const QString& usrName);
    bool          setTime(const QString& resName, const QString& resTime);

public slots:
    QStringList removeSelected();

private:
    void setChecked(const QModelIndex& index, bool val);

private:
    enum Column
    {
        NAME = 0,
        USER,
        TIME,
        SELECTED,
        LAST
    };

    typedef QHash<Column, QVariant> ResData;
    QList<ResData>                  m_table_row;
};

#endif // TABLEMODEL_H
