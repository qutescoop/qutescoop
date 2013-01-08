/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef LOGBROWSERDIALOG_H
#define LOGBROWSERDIALOG_H

#include "_pch.h"

#include "GuiMessage.h"

class QTextBrowser;
class QPushButton;

class LogBrowserDialog : public QDialog {
        Q_OBJECT
    public:
        static LogBrowserDialog *instance(bool createIfNoInstance = true, QWidget *parent = 0);
        ~LogBrowserDialog();
    public slots:
        void on_message_new(const QString &msg);
    protected slots:
        void on_btnSave_clicked();
        void on_btnCopy_clicked();
    protected:
        QTextBrowser *_browser;
        QPushButton *_btnClear, *_btnSave, *_btnCopy;

    private:
        LogBrowserDialog(QWidget *parent = 0);
};

#endif // LOGBROWSERDIALOG_H
