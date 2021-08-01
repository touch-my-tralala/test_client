#include "hostinputdialog.h"

HostInputDialog::HostInputDialog(QWidget* parrent)
    : QDialog(parrent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint)
{
    /*m_port.setMaxLength(4);

    QLabel port_label("Порт");
    QLabel addr_label("Адрес");

    port_label.setBuddy(&m_port);
    addr_label.setBuddy(&m_address);

    QPushButton ok_btn("Ввод");
    QPushButton cancel_btn("Отмена");

    connect(&ok_btn, &QPushButton::clicked, &QDialog::accept);
    connect(&cancel_btn, &QPushButton::clicked, &QDialog::reject);

    QGridLayout ptopLayout;
    ptopLayout.addWidget(&port_label, 0, 0);
    ptopLayout.addWidget(&addr_label, 1, 0);
    ptopLayout.addWidget(&m_port, 0, 1);
    ptopLayout.addWidget(&m_address, 1, 1);
    ptopLayout.addWidget(&ok_btn, 2,0);
    ptopLayout.addWidget(&cancel_btn, 2, 1);
    setLayout(&ptopLayout);*/

    m_port = new QLineEdit;
        m_address  = new QLineEdit;

        QLabel* plblFirstName    = new QLabel("&First Name");
        QLabel* plblLastName     = new QLabel("&Last Name");

        plblFirstName->setBuddy(m_port);
        plblLastName->setBuddy(m_address);

        QPushButton* pcmdOk     = new QPushButton("&Ok");
        QPushButton* pcmdCancel = new QPushButton("&Cancel");

        connect(pcmdOk, SIGNAL(clicked()), SLOT(accept()));
        connect(pcmdCancel, SIGNAL(clicked()), SLOT(reject()));

        //Layout setup
        QGridLayout* ptopLayout = new QGridLayout;
        ptopLayout->addWidget(plblFirstName, 0, 0);
        ptopLayout->addWidget(plblLastName, 1, 0);
        ptopLayout->addWidget(m_port, 0, 1);
        ptopLayout->addWidget(m_address, 1, 1);
        ptopLayout->addWidget(pcmdOk, 2,0);
        ptopLayout->addWidget(pcmdCancel, 2, 1);
        setLayout(ptopLayout);
}
