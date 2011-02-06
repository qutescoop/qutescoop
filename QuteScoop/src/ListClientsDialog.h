/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef LISTCLIENTSDIALOG_H
#define LISTCLIENTSDIALOG_H

#include <QSortFilterProxyModel>
#include <QStack>

#include "ListClientsDialogModel.h"
#include "ui_ListClientsDialog.h"

class ListClientsDialog : public QDialog, private Ui::ListClientsDialog {
    Q_OBJECT
public:
    static ListClientsDialog *getInstance(bool createIfNoInstance = true, QWidget *parent = 0);
    void destroyInstance();

public slots:
    void refresh();
    void performSearch();
    void pingReceived(QString, int);

private slots:
    void on_pbPingVoiceServers_clicked();
    void on_pbPingServers_clicked();
    void modelSelected(const QModelIndex& index);
    void on_editFilter_textChanged(QString str);
    void voiceServerClicked(int row, int col);

private:
    ListClientsDialog(QWidget *parent);

    ListClientsDialogModel *clientsModel;
    //ListClientsSortFilter *listClientsSortModel;
    QSortFilterProxyModel *clientsProxyModel;
    QColor mapPingToColor(int ms);
    QStack <QString> pingStack;
    void pingNextFromStack();

    QTimer editFilterTimer;
};

#endif // LISTCLIENTSDIALOG_H
