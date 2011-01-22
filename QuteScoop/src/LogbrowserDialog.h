#ifndef LOGBROWSERDIALOG_H
#define LOGBROWSERDIALOG_H

#include <QDialog>

class QTextBrowser;
class QPushButton;

class LogBrowserDialog : public QDialog
{
    Q_OBJECT

public:
    LogBrowserDialog(QWidget *parent = 0);
    ~LogBrowserDialog();

public slots:
    void outputMessage( QtMsgType type, const QString &msg );

protected slots:
    void slotSave();
    void slotCopy();

protected:
    virtual void keyPressEvent( QKeyEvent *e );
    virtual void closeEvent( QCloseEvent *e );

    QTextBrowser *browser;
    QPushButton *clearButton, *saveButton, *copyButton;
};

#endif // LOGBROWSERDIALOG_H
