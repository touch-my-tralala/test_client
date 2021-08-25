#include "updateinputdialog.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

UpdateInputDialog::UpdateInputDialog(QWidget* parrent)
    : QDialog(parrent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint)
{
    setWindowTitle("Update ready");
    setFixedSize(180, 80);

    QLabel* label = new QLabel("Дотупно новое обновление. Рекомендую обновиться.",
                               this);
    label->setWordWrap(true);

    QPushButton* btnOk     = new QPushButton("Ok", this);
    QPushButton* btnCancel = new QPushButton("Cancel", this);

    connect(btnOk, SIGNAL(clicked()), SLOT(accept()));
    connect(btnCancel, SIGNAL(clicked()), SLOT(reject()));

    QHBoxLayout* btnLayout = new QHBoxLayout;
    btnLayout->addWidget(btnOk);
    btnLayout->addWidget(btnCancel);

    QVBoxLayout* labelLayout = new QVBoxLayout;
    labelLayout->addWidget(label);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(labelLayout);
    mainLayout->addLayout(btnLayout);
    setLayout(mainLayout);
}
