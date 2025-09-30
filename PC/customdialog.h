#ifndef CUSTOMDIALOG_H
#define CUSTOMDIALOG_H

#include "qeventloop.h"
#include <QDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>

class CustomDialog : public QDialog {
    Q_OBJECT
public:
    explicit CustomDialog(QWidget *parent,
                          const QString &title,
                          const QString &message,
                          const QString &acceptText,
                          const QString &rejectText = QString())
        : QDialog(parent)
    {
        setWindowTitle(title);
        setWindowModality(Qt::NonModal);
        QVBoxLayout *layout = new QVBoxLayout(this);

        QLabel *label = new QLabel(message, this);
        layout->addWidget(label);

        QHBoxLayout *buttonLayout = new QHBoxLayout();
        QPushButton *btnAccept = new QPushButton(acceptText, this);
        buttonLayout->addWidget(btnAccept);
        connect(btnAccept, &QPushButton::clicked, this, [this]() {
            acceptedFlag = true;
            accept(); // закрываем диалог
        });

        if (!rejectText.isEmpty()) {
            QPushButton *btnReject = new QPushButton(rejectText, this);
            buttonLayout->addWidget(btnReject);
            connect(btnReject, &QPushButton::clicked, this, [this]() {
                acceptedFlag = false;
                reject(); // закрываем диалог
            });
        }

        layout->addLayout(buttonLayout);
    }

    bool execDialog(){
        acceptedFlag = false;

        QEventLoop loop;
        connect(this, &QDialog::finished, &loop, &QEventLoop::quit);

        this->show();
        loop.exec();

        return acceptedFlag;
    }

private:
    bool acceptedFlag = false;
};

#endif // CUSTOMDIALOG_H
