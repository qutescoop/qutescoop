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
    resize(600, 400);
    setWindowTitle("Debug Log");
    setWindowFlags(Qt::Tool);

    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);

    layout->addWidget(new QLabel(QString("Debug messages are shown here while this dialog is open."), this));

    browser = new QTextBrowser(this);
    // adding log.txt contents to the browser (log.txt contains all debug messages up to this time)
    QFile logFile (Settings::applicationDataDirectory("log.txt"));
    browser->append(QString("-> CONTENTS of %1: <-\n").arg(logFile.fileName()));
    if (logFile.open(QIODevice::ReadOnly)) {
        browser->append(logFile.readAll());
        logFile.close();
    }
    browser->append("-> LogBrowserDialog receiving live debug messages: <-\n");

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

    connect(this, SIGNAL(hasGuiMessage(QString,GuiMessage::GuiMessageType,QString,int,int)),
            qobject_cast<Window *>(this->parent()), SLOT(showGuiMessage(QString,GuiMessage::GuiMessageType,QString,int,int)));
}


LogBrowserDialog::~LogBrowserDialog()
{
}


void LogBrowserDialog::outputMessage(const QString &msg)
{
    browser->append(msg);
    browser->repaint();
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
