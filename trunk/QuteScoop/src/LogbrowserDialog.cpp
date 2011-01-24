/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "Logbrowserdialog.h"

#include <QApplication>
#include <QtGui>
#include <QtCore>

LogBrowserDialog *logbrowserDialogInstance = 0;

LogBrowserDialog *LogBrowserDialog::getInstance(bool createIfNoInstance) {
    if(logbrowserDialogInstance == 0)
        if (createIfNoInstance) logbrowserDialogInstance = new LogBrowserDialog();
    return logbrowserDialogInstance;
}

LogBrowserDialog::LogBrowserDialog(QWidget *parent)
    : QDialog(parent)
{
    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);

    layout->addWidget(new QLabel(tr("Debug messages are shown here while this dialog is open. See also 'log.txt'."), this));

    browser = new QTextBrowser(this);
    layout->addWidget(browser);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    layout->addLayout(buttonLayout);

    buttonLayout->addStretch(10);

    clearButton = new QPushButton(this);
    clearButton->setText("clear");
    buttonLayout->addWidget(clearButton);
    connect(clearButton, SIGNAL(clicked()), browser, SLOT(clear()));

    saveButton = new QPushButton(this);
    saveButton->setText("save output");
    buttonLayout->addWidget(saveButton);
    connect(saveButton, SIGNAL(clicked()), this, SLOT(slotSave()));

    copyButton = new QPushButton(this);
    copyButton->setText("copy to clipboard");
    buttonLayout->addWidget(copyButton);
    connect(copyButton, SIGNAL(clicked()), this, SLOT(slotCopy()));

    resize(600, 400);
}


LogBrowserDialog::~LogBrowserDialog()
{
    qInstallMsgHandler(0);
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
                tr("%1/logfile.txt").arg(QDir::homePath()),
                tr("Text Files (*.txt);;All Files (*)")
                );

    if(saveFileName.isEmpty())
        return;

    QFile file(saveFileName);
    if(!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(
                    this,
                    tr("Error"),
                    QString(tr("<nobr>File '%1'<br/>cannot be opened for writing.<br/><br/>"
                               "The log output could <b>not</b> be saved!</nobr>"))
                    .arg(saveFileName));
        return;
    }

    QTextStream stream(&file);
    stream << browser->toPlainText();
    file.close();
}
