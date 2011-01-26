/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef LISTCLIENTSDIALOG_H
#define LISTCLIENTSDIALOG_H

#include <QSortFilterProxyModel>

#include "ListClientsDialogModel.h"
#include "ListClientsSortFilter.h"
#include "ui_ListClientsDialog.h"

class ListClientsDialog : public QDialog, private Ui::ListClientsDialog {
    Q_OBJECT
public:
    static ListClientsDialog *getInstance(bool createIfNoInstance = true, QWidget *parent = 0);
    void destroyInstance();
    void refresh();

public slots:
    void performSearch();
    void newMapPosition();
    void pingReceived(QString, int);

private slots:
    void on_pbPingVoiceServers_clicked();
    void on_pbPingServers_clicked();
    void modelSelected(const QModelIndex& index);
    void on_editFilter_textChanged(QString str);
    void serverClicked(int row, int col);
    void newFilter();

private:
    ListClientsDialog(QWidget *parent);

    ListClientsDialogModel listClientsModel;
    ListClientsSortFilter *listClientsSortModel;
    QColor mapPingToColor(int ms);
    QStack< QString > pingStack;
    void pingNextFromStack();

    QTimer searchTimer;
};

#endif // LISTCLIENTSDIALOG_H
