#include "mytablewidget.h"

MyTableWidget::MyTableWidget(QWidget *parent) : QTableView(parent)
{
    resizeColumnsToContents();
    setSelectionMode(QAbstractItemView::NoSelection);
    setFocusPolicy(Qt::NoFocus);
    setModel(m_model = new TableModel);
}

