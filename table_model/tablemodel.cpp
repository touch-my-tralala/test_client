#include "tablemodel.h"

TableModel::TableModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

TableModel::~TableModel() = default;

int TableModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return m_table_row.count();
}

int TableModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return LAST;
}

QVariant TableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Vertical)
        return section;

    switch (section)
    {
        case NAME:
            return "Имя";
        case USER:
            return "Пользователь";
        case TIME:
            return "Время использования";
        case SELECTED:
            return "Выбор";
    }
    return QVariant();
}

QVariant TableModel::data(const QModelIndex& index, int role) const
{
    if (index.isValid() && !(m_table_row.count() <= index.row()))
    {
        if (index.column() == SELECTED && role == Qt::CheckStateRole)
            return m_table_row[index.row()][SELECTED].toBool() ? Qt::Checked : Qt::Unchecked;

        if (role == Qt::TextAlignmentRole)
            return Qt::AlignCenter;

        if ((role == Qt::DisplayRole || role == Qt::EditRole) && index.column() != SELECTED)
            return m_table_row[index.row()][Column(index.column())];
    }
    return QVariant();
}

bool TableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || m_table_row.count() <= index.row() || role == Qt::EditRole)
        return false;
    if (role == Qt::CheckStateRole)
        set_checked(index, value.toBool());
    else
        m_table_row[index.row()][Column(index.column())] = value;

    emit dataChanged(index, index);
    return true;
}

Qt::ItemFlags TableModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return {};

    Qt::ItemFlags flags = QAbstractTableModel::flags(index);
    flags               = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    if (index.column() == SELECTED)
        flags |= Qt::ItemIsUserCheckable;

    return flags;
}

void TableModel::set_checked(const QModelIndex& index, bool val)
{
    m_table_row[index.row()][SELECTED] = val;
}

bool TableModel::appendRes(const QString& resName)
{
    for (auto& i : m_table_row)
    {
        if (i[NAME] == resName)
            return false;
    }
    ResData resurs;
    resurs[NAME] = resName;
    resurs[USER] = "free";
    resurs[TIME] = "00:00:00";
    auto row     = m_table_row.count();
    beginInsertRows(QModelIndex(), row, row);
    m_table_row.append(resurs);
    endInsertRows();
    return true;
}

void TableModel::removeAllRows()
{
    if (m_table_row.size() > 0)
    {
        beginRemoveRows(QModelIndex(), 0, m_table_row.size() - 1);
        m_table_row.clear();
        endRemoveRows();
    }
}

bool TableModel::setUser(const QString& resName, const QString& usrName)
{
    for (auto& i : m_table_row)
    {
        if (i[NAME] == resName)
        {
            i[USER] = usrName;
            return true;
        }
    }
    return false;
}

bool TableModel::setTime(const QString& resName, const QString& resTime)
{
    for (auto& i : m_table_row)
    {
        if (i[NAME] == resName)
        {
            i[TIME] = resTime;
            return true;
        }
    }
    return false;
}

QStringList TableModel::getSelected()
{
    QStringList selectedList;
    for (auto i = m_table_row.begin(); i != m_table_row.end(); i++)
    {
        if (i->value(SELECTED, false).toBool())
            selectedList << i->value(NAME).toString();
    }
    return selectedList;
}
