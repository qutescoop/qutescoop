/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "LogBrowserDialog.h"

#include "_pch.h"

#include "Window.h"
#include "helpers.h"
#include "Settings.h"

LogBrowserDialog *logBrowserDialogInstance = 0;

LogBrowserDialog *LogBrowserDialog::instance(bool createIfNoInstance, QWidget *parent) {
    if(logBrowserDialogInstance == 0)
        if (createIfNoInstance) {
            if (parent == 0) parent = Window::instance();
            logBrowserDialogInstance = new LogBrowserDialog(parent);
        }
    return logBrowserDialogInstance;
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

    _browser = new QTextBrowser(this);
    // adding log.txt contents to the browser (log.txt contains all debug messages up to this time)
    QFile logFile(Settings::applicationDataDirectory("log.txt"));
    _browser->append(QString("<b><img src=\":/icons/images/log16.png\"/> reading %1:</b>")
                    .arg(logFile.fileName()));
    if (logFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&logFile);
        while (!in.atEnd())
            on_message_new(in.readLine());
        logFile.close();
    } else
        qWarning("Unable to read log file");
    _browser->append("<b><img src=\":/icons/images/refresh16.png\"/> Debug log receiving live debug messages:</b>");

    layout->addWidget(_browser);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    layout->addLayout(buttonLayout);

    buttonLayout->addStretch(10);

    _btnClear = new QPushButton(this);
    _btnClear->setText("Clear");
    buttonLayout->addWidget(_btnClear);
    connect(_btnClear, SIGNAL(clicked()), _browser, SLOT(clear()));

    _btnSave = new QPushButton(this);
    _btnSave->setText("Save output");
    buttonLayout->addWidget(_btnSave);
    connect(_btnSave, SIGNAL(clicked()), this, SLOT(on_btnSave_clicked()));

    _btnCopy = new QPushButton(this);
    _btnCopy->setText("Copy to clipboard");
    buttonLayout->addWidget(_btnCopy);
    connect(_btnCopy, SIGNAL(clicked()), this, SLOT(on_btnCopy_clicked()));
}


LogBrowserDialog::~LogBrowserDialog() {
}


void LogBrowserDialog::on_message_new(const QString &msg) {
    if (msg.startsWith("0:"))
        _browser->append("<img src=\":/icons/images/info16.png\"/> " + msg);
    else if (msg.startsWith("1:"))
        _browser->append("<img src=\":/icons/images/warn16.png\"/> " + msg);
    else if (msg.startsWith("2:"))
        _browser->append("<img src=\":/icons/images/error16.png\"/> " + msg);
    else if (msg.startsWith("3:"))
        _browser->append("<img src=\":/icons/images/fatal16.png\"/> " + msg);
    else
        _browser->append("<i>" + msg + "</i>");
    _browser->repaint();
}


void LogBrowserDialog::on_btnCopy_clicked() {
    QApplication::clipboard()->setText(_browser->toPlainText());
}

void LogBrowserDialog::on_btnSave_clicked() {
    QString saveFileName = QFileDialog::getSaveFileName(this, "Save Log Output",
                "log.txt", "Text Files (*.txt);;All Files (*)");
    if(saveFileName.isEmpty())
        return;
    QFile file(saveFileName);
    if(file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);
        stream << _browser->toPlainText();
        file.close();
    } else
        GuiMessages::criticalUserInteraction(QString("File '%1' cannot be written. The log could not be saved!")
                                            .arg(saveFileName), "Debug Log");
}
