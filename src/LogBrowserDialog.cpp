/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "LogBrowserDialog.h"

#include "_pch.h"

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

LogBrowserDialog::LogBrowserDialog(QWidget *parent) : QDialog(parent) {
    resize(800, 400);
    setWindowTitle("Debug log");
    setWindowFlags(Qt::Tool);
    setWindowFlags(windowFlags() ^= Qt::WindowContextHelpButtonHint);
    setWindowFlags(windowFlags() |= Qt::WindowMaximizeButtonHint);

    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);

    layout->addWidget(new QLabel(QString("Debug messages are shown here while this dialog is open."), this));

    browser = new QTextBrowser(this);
    // adding log.txt contents to the browser (log.txt contains all debug messages up to this time)
    QFile logFile(Settings::applicationDataDirectory("log.txt"));
    browser->append(QString("<b><img src=\":/icons/images/log16.png\"/> reading %1:</b>")
                    .arg(logFile.fileName()));
    if (logFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&logFile);
        while (!in.atEnd())
            outputMessage(in.readLine());
        logFile.close();
    } else
        qWarning("Unable to read log file");
    browser->append("<b><img src=\":/icons/images/refresh16.png\"/> Debug log receiving live debug messages:</b>");

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
}


LogBrowserDialog::~LogBrowserDialog() {
}


void LogBrowserDialog::outputMessage(const QString &msg) {
    if (msg.startsWith("0:"))
        browser->append("<img src=\":/icons/images/info16.png\"/> " + msg);
    else if (msg.startsWith("1:"))
        browser->append("<img src=\":/icons/images/warn16.png\"/> " + msg);
    else if (msg.startsWith("2:"))
        browser->append("<img src=\":/icons/images/error16.png\"/> " + msg);
    else if (msg.startsWith("3:"))
        browser->append("<img src=\":/icons/images/fatal16.png\"/> " + msg);
    else
        browser->append("<i>" + msg + "</i>");
    browser->repaint();
}


void LogBrowserDialog::slotCopy() {
    QApplication::clipboard()->setText(browser->toPlainText());
}

void LogBrowserDialog::slotSave() {
    QString saveFileName = QFileDialog::getSaveFileName(this, "Save Log Output",
                "log.txt", "Text Files (*.txt);;All Files (*)");
    if(saveFileName.isEmpty())
        return;
    QFile file(saveFileName);
    if(file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);
        stream << browser->toPlainText();
        file.close();
    } else
        GuiMessages::criticalUserInteraction(QString("File '%1' cannot be written. The log could not be saved!")
                                            .arg(saveFileName), "Debug Log");
}
