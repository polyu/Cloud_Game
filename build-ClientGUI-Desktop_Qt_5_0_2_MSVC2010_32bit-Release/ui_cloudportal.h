/********************************************************************************
** Form generated from reading UI file 'cloudportal.ui'
**
** Created by: Qt User Interface Compiler version 5.0.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CLOUDPORTAL_H
#define UI_CLOUDPORTAL_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_CloudPortal
{
public:
    QPushButton *playpushButton;
    QPushButton *pushButton_2;
    QLabel *imglabel_2;
    QLabel *label_3;
    QPlainTextEdit *gameTextEdit;
    QComboBox *qualitycomboBox;
    QLabel *label_2;
    QWidget *widget;
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QListWidget *gamelistWidget;

    void setupUi(QDialog *CloudPortal)
    {
        if (CloudPortal->objectName().isEmpty())
            CloudPortal->setObjectName(QStringLiteral("CloudPortal"));
        CloudPortal->resize(760, 395);
        playpushButton = new QPushButton(CloudPortal);
        playpushButton->setObjectName(QStringLiteral("playpushButton"));
        playpushButton->setGeometry(QRect(540, 350, 81, 23));
        pushButton_2 = new QPushButton(CloudPortal);
        pushButton_2->setObjectName(QStringLiteral("pushButton_2"));
        pushButton_2->setGeometry(QRect(630, 350, 81, 23));
        imglabel_2 = new QLabel(CloudPortal);
        imglabel_2->setObjectName(QStringLiteral("imglabel_2"));
        imglabel_2->setGeometry(QRect(200, 50, 531, 191));
        imglabel_2->setPixmap(QPixmap(QString::fromUtf8(":/img/welcome.jpg")));
        imglabel_2->setScaledContents(true);
        label_3 = new QLabel(CloudPortal);
        label_3->setObjectName(QStringLiteral("label_3"));
        label_3->setGeometry(QRect(200, 20, 61, 16));
        gameTextEdit = new QPlainTextEdit(CloudPortal);
        gameTextEdit->setObjectName(QStringLiteral("gameTextEdit"));
        gameTextEdit->setGeometry(QRect(200, 260, 531, 81));
        gameTextEdit->setReadOnly(true);
        qualitycomboBox = new QComboBox(CloudPortal);
        qualitycomboBox->setObjectName(QStringLiteral("qualitycomboBox"));
        qualitycomboBox->setGeometry(QRect(313, 350, 211, 22));
        label_2 = new QLabel(CloudPortal);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(220, 350, 71, 21));
        widget = new QWidget(CloudPortal);
        widget->setObjectName(QStringLiteral("widget"));
        widget->setGeometry(QRect(10, 20, 181, 351));
        verticalLayout = new QVBoxLayout(widget);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        label = new QLabel(widget);
        label->setObjectName(QStringLiteral("label"));

        verticalLayout->addWidget(label);

        gamelistWidget = new QListWidget(widget);
        new QListWidgetItem(gamelistWidget);
        new QListWidgetItem(gamelistWidget);
        gamelistWidget->setObjectName(QStringLiteral("gamelistWidget"));

        verticalLayout->addWidget(gamelistWidget);


        retranslateUi(CloudPortal);
        QObject::connect(pushButton_2, SIGNAL(clicked()), CloudPortal, SLOT(close()));

        QMetaObject::connectSlotsByName(CloudPortal);
    } // setupUi

    void retranslateUi(QDialog *CloudPortal)
    {
        CloudPortal->setWindowTitle(QApplication::translate("CloudPortal", "Cloud Gaming", 0));
        playpushButton->setText(QApplication::translate("CloudPortal", "\345\274\200\345\247\213\347\216\251", 0));
        pushButton_2->setText(QApplication::translate("CloudPortal", "\351\200\200\345\207\272", 0));
        imglabel_2->setText(QString());
        label_3->setText(QApplication::translate("CloudPortal", "\346\270\270\346\210\217\344\273\213\347\273\215:", 0));
        gameTextEdit->setPlainText(QApplication::translate("CloudPortal", "Welcome to our cloud gaming client!", 0));
        qualitycomboBox->clear();
        qualitycomboBox->insertItems(0, QStringList()
         << QApplication::translate("CloudPortal", "\351\253\230\346\270\205", 0)
         << QApplication::translate("CloudPortal", "\346\240\207\345\207\206", 0)
         << QApplication::translate("CloudPortal", "\346\265\201\347\225\205", 0)
        );
        label_2->setText(QApplication::translate("CloudPortal", "\350\247\206\351\242\221\350\264\250\351\207\217\357\274\232", 0));
        label->setText(QApplication::translate("CloudPortal", "\346\270\270\346\210\217\345\210\227\350\241\250:", 0));

        const bool __sortingEnabled = gamelistWidget->isSortingEnabled();
        gamelistWidget->setSortingEnabled(false);
        QListWidgetItem *___qlistwidgetitem = gamelistWidget->item(0);
        ___qlistwidgetitem->setText(QApplication::translate("CloudPortal", "Dota", 0));
        QListWidgetItem *___qlistwidgetitem1 = gamelistWidget->item(1);
        ___qlistwidgetitem1->setText(QApplication::translate("CloudPortal", "Directx\346\265\213\350\257\225\347\250\213\345\272\217", 0));
        gamelistWidget->setSortingEnabled(__sortingEnabled);

    } // retranslateUi

};

namespace Ui {
    class CloudPortal: public Ui_CloudPortal {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CLOUDPORTAL_H
