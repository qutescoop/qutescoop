/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "LogBrowserDialog.h"

#include <QtGui>

#include "Window.h"
#include "helpers.h"
#include "Settings.h"

LogBrowserDialog *logbrowserDialogInstance = 0;

LogBrowserDialog *LogBrowserDialog::getInstance(bool createIfNoInstance, QWidget *parent) {
    if(logbrowserDialogInstance == 0)
        if (createIfNoInstance) {
            if (parent == 0) parent = Window::getInstance(true);
            logbrowserDialogInstance = new LogBrowserDialog(parent);
        }
    return logbrowserDialogInstance;
}

LogBrowserDialog::LogBrowserDialog(QWidget *parent)
    : QDialog(parent)
{
    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);

    layout->addWidget(new QLabel(QString("Debug messages are shown here while this dialog is open.\nComplete log in '%1'.")
                                 .arg(Settings::applicationDataDirectory("log.txt")), this));

    browser = new QTextBrowser(this);
    browser->append(QString("LogBrowser on %1").arg(VERSION_STRING));
    browser->append(QString("Expecting application data directory at %1 (gets calculated on each start)")
                    .arg(Settings::applicationDataDirectory()));
    layout->addWidget(browser);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    layout->addLayout(buttonLayout);

    buttonLayout->addStretch(10);

    clearButton = new QPushButton(this);
    clearButton->setText("Clear");
    buttonLayout->addWidget(clearButton);
    connect(clearButton, SIGNAL(clicked()), browser, SLOT(clear()));

    saveButton = new QPushButton(this);
    saveButton->setText("Save output");
    buttonLayout->addWidget(saveButton);
    connect(saveButton, SIGNAL(clicked()), this, SLOT(slotSave()));

    copyButton = new QPushButton(this);
    copyButton->setText("Copy to clipboard");
    buttonLayout->addWidget(copyButton);
    connect(copyButton, SIGNAL(clicked()), this, SLOT(slotCopy()));

    resize(600, 400);

    connect(this, SIGNAL(hasGuiMessage(QString,GuiMessage::GuiMessageType,QString,int,int)),
            qobject_cast<Window *>(this->parent()), SLOT(showGuiMessage(QString,GuiMessage::GuiMessageType,QString,int,int)));
}


LogBrowserDialog::~LogBrowserDialog()
{
}


void LogBrowserDialog::outputMessage(QtMsgType type, const QString &msg)
{
    switch (type) {
    case QtDebugMsg:
        browser->append(msg);
        break;

    case QtWarningMsg:
        browser->append(tr("-- WARNING: %1").arg(msg));
        break;

    case QtCriticalMsg:
        browser->append(tr("-- CRITICAL: %1").arg(msg));
        break;

    case QtFatalMsg:
        browser->append(tr("-- FATAL: %1").arg(msg));
        break;
    }
}


void LogBrowserDialog::slotCopy()
{
    QApplication::clipboard()->setText(browser->toPlainText());
}

void LogBrowserDialog::slotSave()
{
    QString saveFileName = QFileDialog::getSaveFileName(
                this,
                tr("Save Log Output"),
                tr("log.txt"),
                tr("Text Files (*.txt);;All Files (*)")
                );

    if(saveFileName.isEmpty())
        return;

    QFile file(saveFileName);
    if(!file.open(QIODevice::WriteOnly)) {
        emit hasGuiMessage(QString("File '%1' cannot be written. The log could not be saved!")
                           .arg(saveFileName), GuiMessage::CriticalUserInteraction);
        return;
    }

    QTextStream stream(&file);
    stream << browser->toPlainText();
    file.close();
}
