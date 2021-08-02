#include "hostinputdialog.h"

HostInputDialog::HostInputDialog(QWidget* parrent)
    : QDialog(parrent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint)
{
        m_port = new QLineEdit(this);
        m_address  = new QLineEdit(this);

        QLabel* plblFirstName    = new QLabel("&First Name", this);
        QLabel* plblLastName     = new QLabel("&Last Name", this);

        plblFirstName->setBuddy(m_port);
        plblLastName->setBuddy(m_address);

        QPushButton* pcmdOk     = new QPushButton("&Ok", this);
        QPushButton* pcmdCancel = new QPushButton("&Cancel", this);

        connect(pcmdOk, SIGNAL(clicked()), SLOT(accept()));
        connect(pcmdCancel, SIGNAL(clicked()), SLOT(reject()));

        //Layout setup
        QGridLayout* ptopLayout = new QGridLayout(this);
        ptopLayout->addWidget(plblFirstName, 0, 0);
        ptopLayout->addWidget(plblLastName, 1, 0);
        ptopLayout->addWidget(m_port, 0, 1);
        ptopLayout->addWidget(m_address, 1, 1);
        ptopLayout->addWidget(pcmdOk, 2,0);
        ptopLayout->addWidget(pcmdCancel, 2, 1);
        setLayout(ptopLayout);
}
