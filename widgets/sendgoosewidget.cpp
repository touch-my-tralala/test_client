#include "sendgoosewidget.h"

SendGooseWidget::SendGooseWidget(QWidget* parrent)
    : QDialog(parrent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint)
{
    m_name = new QLineEdit(this);

    QLabel* nameLabel = new QLabel("Имя", this);

    QPushButton* okBtn     = new QPushButton("&Ok", this);
    QPushButton* cancelBtn = new QPushButton("&Cancel", this);

    connect(okBtn, SIGNAL(clicked()), SLOT(accept()));
    connect(cancelBtn, SIGNAL(clicked()), SLOT(reject()));

    QGridLayout* layout = new QGridLayout(this);
    layout->addWidget(nameLabel, 0, 0);
    layout->addWidget(m_name, 0, 1);
    layout->addWidget(okBtn, 2, 0);
    layout->addWidget(cancelBtn, 2, 1);
}

QJsonObject SendGooseWidget::getSendObj()
{
    QJsonObject obj({ { "type", "goose" },
                      { "username", m_name->text() } });
    return obj;
}
