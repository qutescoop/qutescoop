/********************************************************************************
** Form generated from reading ui file 'MetarDialog.ui'
**
** Created: Wed Dec 5 23:02:02 2007
**      by: Qt User Interface Compiler version 4.3.2
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_METARDIALOG_H
#define UI_METARDIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QTextBrowser>
#include <QtGui/QVBoxLayout>

class Ui_MetarDialog
{
public:
    QVBoxLayout *vboxLayout;
    QTextBrowser *metarText;

    void setupUi(QDialog *MetarDialog)
    {
    if (MetarDialog->objectName().isEmpty())
        MetarDialog->setObjectName(QString::fromUtf8("MetarDialog"));
    MetarDialog->resize(410, 220);
    vboxLayout = new QVBoxLayout(MetarDialog);
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    vboxLayout->setContentsMargins(4, 4, 4, 4);
    metarText = new QTextBrowser(MetarDialog);
    metarText->setObjectName(QString::fromUtf8("metarText"));
    metarText->setAcceptRichText(true);
    metarText->setOpenExternalLinks(true);

    vboxLayout->addWidget(metarText);


    retranslateUi(MetarDialog);

    QMetaObject::connectSlotsByName(MetarDialog);
    } // setupUi

    void retranslateUi(QDialog *MetarDialog)
    {
    MetarDialog->setWindowTitle(QApplication::translate("MetarDialog", "Dialog", 0, QApplication::UnicodeUTF8));
    metarText->setHtml(QApplication::translate("MetarDialog", "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'Lucida Grande'; font-size:13pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">bla</p></body></html>", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(MetarDialog);
    } // retranslateUi

};

namespace Ui {
    class MetarDialog: public Ui_MetarDialog {};
} // namespace Ui

#endif // UI_METARDIALOG_H
