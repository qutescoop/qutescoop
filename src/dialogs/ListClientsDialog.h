#ifndef LISTCLIENTSDIALOG_H_
#define LISTCLIENTSDIALOG_H_

#include "ui_ListClientsDialog.h"

#include "../models/ListClientsDialogModel.h"

class ListClientsDialog
    : public QDialog, private Ui::ListClientsDialog {
    Q_OBJECT
    public:
        static ListClientsDialog* instance(bool createIfNoInstance = true, QWidget* parent = 0);
        void destroyInstance();

    public slots:
        void refresh();
        void performSearch();
        void pingReceived(QString, int);

    protected:
        void closeEvent(QCloseEvent* event);
        void showEvent(QShowEvent* event);

    private slots:
        void on_pbPingServers_clicked();
        void modelSelected(const QModelIndex& index);
        void on_editFilter_textChanged(QString str);

    private:
        ListClientsDialog(QWidget* parent);

        ListClientsDialogModel* _clientsModel;
        QSortFilterProxyModel* _clientsProxyModel;
        QColor mapPingToColor(int ms);
        QStack <QString> _pingStack;
        void pingNextFromStack();

        QTimer _editFilterTimer;

        constexpr static char m_preferencesName[] = "listClientsDialog";
};

#endif // LISTCLIENTSDIALOG_H
