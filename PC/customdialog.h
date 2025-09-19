#ifndef CUSTOMDIALOG_H
#define CUSTOMDIALOG_H

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
        setWindowModality(Qt::WindowModal); // блокирует алгоритм, но не весь GUI
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

    bool execDialog() {
        acceptedFlag = false;
        QDialog::exec(); // блокирует алгоритм
        return acceptedFlag;
    }

private:
    bool acceptedFlag = false;
};

#endif // CUSTOMDIALOG_H
