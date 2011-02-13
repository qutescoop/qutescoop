/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef LOGBROWSERDIALOG_H
#define LOGBROWSERDIALOG_H

#include "_pch.h"

#include "GuiMessage.h"

class QTextBrowser;
class QPushButton;

class LogBrowserDialog : public QDialog
{
    Q_OBJECT

public:
    static LogBrowserDialog *getInstance(bool createIfNoInstance = true, QWidget *parent = 0);
    ~LogBrowserDialog();

signals:
    void hasGuiMessage(QString, GuiMessage::GuiMessageType = GuiMessage::Temporary,
                       QString = QString(), int = 0, int = 0);

public slots:
    void outputMessage(const QString &msg);

protected slots:
    void slotSave();
    void slotCopy();

protected:
    QTextBrowser *browser;
    QPushButton *clearButton, *saveButton, *copyButton;

private:
    LogBrowserDialog(QWidget *parent = 0);
};

#endif // LOGBROWSERDIALOG_H
