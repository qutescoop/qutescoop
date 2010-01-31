/**************************************************************************
 *  This file is part of QuteScoop.
 *  Copyright (C) 2007-2009 Martin Domig <martin@domig.net>
 *
 *  QuteScoop is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  QuteScoop is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with QuteScoop.  If not, see <http://www.gnu.org/licenses/>
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
    static ListClientsDialog *getInstance(bool createIfNoInstance = true);
    void refresh();

public slots:
    void newMapPosition();
    void pingReceived(QString, int);

private slots:
    void on_pbPingVoiceServers_clicked();
    void on_pbPingServers_clicked();
    void modelSelected(const QModelIndex& index);
    void on_editFilter_textChanged(QString str);
    void newFilter();
    void serverClicked(int row, int column);

private:
    ListClientsDialog();

    ListClientsDialogModel listClientsModel;
    ListClientsSortFilter *listClientsSortModel;
    QColor mapPingToColor(int ms);
};

#endif // LISTCLIENTSDIALOG_H
