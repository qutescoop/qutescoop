/**************************************************************************
 *  This file is part of QuteScoop.
 *  Copyright (C) 2007-2008 Martin Domig <martin@domig.net>
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

#include "ListClientsDialog.h"
#include "ListClientsDialogModel.h"
#include "ListClientsSortFilter.h"
#include "Settings.h"
#include "Controller.h"
#include "Pilot.h"
#include "Whazzup.h"
#include "Window.h"

ListClientsDialog *listClientsDialog = 0;

ListClientsDialog *ListClientsDialog::getInstance() {
    if(listClientsDialog == 0)
        listClientsDialog = new ListClientsDialog();
    return listClientsDialog;
}


ListClientsDialog::ListClientsDialog() :
    QDialog(Window::getInstance())
{
    setupUi(this);
//    setWindowFlags(Qt::Tool);

    listClientsSortModel = new ListClientsSortFilter;
    listClientsSortModel->setDynamicSortFilter(true);
    listClientsSortModel->setSourceModel(&listClientsModel);
    treeListClients->setModel(listClientsSortModel);

    treeListClients->header()->setResizeMode(QHeaderView::Interactive);
    treeListClients->sortByColumn(0, Qt::AscendingOrder);

    connect(treeListClients->header(), SIGNAL(sectionClicked(int)), treeListClients, SLOT(sortByColumn(int)));
    connect(treeListClients, SIGNAL(clicked(const QModelIndex&)), this, SLOT(modelSelected(const QModelIndex&)));

    QFont font = lblStatusInfo->font();
    font.setPointSize(lblStatusInfo->fontInfo().pointSize() - 1);
    lblStatusInfo->setFont(font); //make it a bit smaller than standard text

    connect(Window::getInstance()->glWidget, SIGNAL(newPosition()), this, SLOT(newMapPosition()));

    refresh();
}

void ListClientsDialog::refresh() {
    QList<Client*> clients;
    QList<Pilot*> ps = Whazzup::getInstance()->realWhazzupData().getPilots();
    for (int i = 0; i < ps.size(); i++) {
        clients << dynamic_cast<Client*> (ps[i]);
    }

    QList<Controller*> cs = Whazzup::getInstance()->realWhazzupData().getControllers();
    for (int i = 0; i < cs.size(); i++) {
        clients << dynamic_cast<Client*> (cs[i]);
    }

    listClientsModel.setClients(clients);
    listClientsSortModel->invalidate();
    treeListClients->header()->resizeSections(QHeaderView::ResizeToContents);

    newFilter();

    const WhazzupData &data = Whazzup::getInstance()->realWhazzupData();

    QString msg = QString("Whazzup %1 updated")
                  .arg(data.timestamp().date() == QDateTime::currentDateTime().toUTC().date() // is today?
                        ? QString("today %1").arg(data.timestamp().time().toString("HHmm'z'"))
                        : (data.timestamp().isValid()
                           ? data.timestamp().toString("ddd yyyy/MM/dd HHmm'z'")
                           : "never")
                        );
    lblStatusInfo->setText(msg);
}

void ListClientsDialog::on_editFilter_textChanged(QString searchStr)
{
    QRegExp regex;
    QStringList tokens = searchStr.trimmed().replace(QRegExp("\\*"), ".*").split(QRegExp("[ \\,]+"), QString::SkipEmptyParts);
    if(tokens.size() == 1) {
        regex = QRegExp("^" + tokens.first() + ".*", Qt::CaseInsensitive);
    } else if(tokens.size() == 0) {
        regex = QRegExp("");
    }
    else {
        QString regExpStr = "^(" + tokens.first();
        for(int i = 1; i < tokens.size(); i++)
            regExpStr += "|" + tokens[i];
        regExpStr += ".*)";
        regex = QRegExp(regExpStr, Qt::CaseInsensitive);
    }

    listClientsSortModel->setFilterRegExp(regex);
    listClientsSortModel->setFilterKeyColumn(-1);
    treeListClients->header()->resizeSections(QHeaderView::ResizeToContents);
    newFilter();
}

void ListClientsDialog::modelSelected(const QModelIndex& index) {
    listClientsModel.modelSelected(listClientsSortModel->mapToSource(index));
}

void ListClientsDialog::newFilter() {
    boxResults->setTitle(QString("Results (%1)").arg(listClientsSortModel->rowCount()));
}

void ListClientsDialog::newMapPosition() {
    if(this->isVisible()) listClientsSortModel->invalidate();
    treeListClients->header()->resizeSections(QHeaderView::ResizeToContents);
}
