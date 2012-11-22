/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#include "ListClientsDialog.h"

#include "ListClientsDialogModel.h"
#include "Settings.h"
#include "Controller.h"
#include "Pilot.h"
#include "Whazzup.h"
#include "Window.h"
#include "Ping.h"

// singleton instance
ListClientsDialog *listClientsDialog = 0;
ListClientsDialog *ListClientsDialog::getInstance(bool createIfNoInstance, QWidget *parent) {
    if(listClientsDialog == 0 && createIfNoInstance) {
        if (parent == 0) parent = Window::getInstance(true);
        listClientsDialog = new ListClientsDialog(parent);
    }
    return listClientsDialog;
}

// destroys a singleton instance
void ListClientsDialog::destroyInstance() {
    delete listClientsDialog;
    listClientsDialog = 0;
}

ListClientsDialog::ListClientsDialog(QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);
    setWindowFlags(windowFlags() ^= Qt::WindowContextHelpButtonHint);
    //    setWindowFlags(Qt::Tool);

    // clients
    clientsModel = new ListClientsDialogModel;
    clientsProxyModel = new QSortFilterProxyModel;
    clientsProxyModel->setDynamicSortFilter(true);
    clientsProxyModel->setSourceModel(clientsModel);
    treeListClients->setUniformRowHeights(true);
    treeListClients->setModel(clientsProxyModel);
    treeListClients->sortByColumn(0, Qt::AscendingOrder);

    treeListClients->setColumnWidth(0, 100);
    treeListClients->setColumnWidth(1, 100);
    treeListClients->setColumnWidth(2, 200);
    treeListClients->setColumnWidth(3, 100);

    connect(treeListClients, SIGNAL(clicked(const QModelIndex&)),
            this, SLOT(modelSelected(const QModelIndex&)));

    // servers
    QStringList serverHeaders;
    serverHeaders << "ident" << "URL" << "ping" << "ping" << "ping" << QString::fromUtf8("Ø ping")
                  << "connected\nclients" << "location" << "description";
    serversTable->setColumnCount(serverHeaders.size());
    serversTable->setHorizontalHeaderLabels(serverHeaders);
    connect(voiceServersTable, SIGNAL(cellClicked(int, int)), this, SLOT(voiceServerClicked(int, int)));

    // voiceServers
    QStringList voiceServerHeaders;
    voiceServerHeaders << "URL" << "ping" << "ping" << "ping" << QString::fromUtf8("Ø ping")
                       << "connected\ncontrollers" << "location" << "description";
    voiceServersTable->setColumnCount(voiceServerHeaders.size());
    voiceServersTable->setHorizontalHeaderLabels(voiceServerHeaders);

    // General
    QFont font = lblStatusInfo->font();
    font.setPointSize(lblStatusInfo->fontInfo().pointSize() - 1);
    lblStatusInfo->setFont(font); //make it a bit smaller than standard text

    connect(&editFilterTimer, SIGNAL(timeout()), this, SLOT(performSearch()));

    QTimer::singleShot(100, this, SLOT(refresh())); // delayed insertion of clients to open the window now
}

void ListClientsDialog::refresh() {
    if (!Settings::listClientsDialogSize().isNull()) resize(Settings::listClientsDialogSize());
    if (!Settings::listClientsDialogPos().isNull()) move(Settings::listClientsDialogPos());
    if (!Settings::listClientsDialogGeometry().isNull()) restoreGeometry(Settings::listClientsDialogGeometry());

    qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
    const WhazzupData &data = Whazzup::getInstance()->whazzupData();

    // Clients
    QHash<QString, int> serversConnected;
    QList<Client*> clients;
    foreach(Pilot *p, data.pilots) {
        clients << dynamic_cast<Client*> (p);
        if (p != 0)
            serversConnected[p->server] = serversConnected.value(p->server, 0) + 1; // count clients
    }
    QHash<QString, int> voiceServerChannels;
    foreach(Controller *c, data.controllers) {
        clients << dynamic_cast<Client*> (c);
        if (c != 0) {
            serversConnected[c->server] = serversConnected.value(c->server, 0) + 1; // count clients
            QString server = c->voiceChannel.section("/", 0, 0).toLower();
            if (!server.isEmpty())
                voiceServerChannels[server] = voiceServerChannels.value(server, 0) + 1;
        }
    }
    clientsModel->setClients(clients);

    // Servers
    serversTable->clearContents();
    serversTable->setRowCount(data.servers.size());
    for (int row = 0; row < data.servers.size(); row++) {
        for (int col = 0; col < serversTable->columnCount(); col++) {
            switch(col) {
                case 0: serversTable->setItem(row, col, new QTableWidgetItem(data.servers[row][0]));
                    break; // ident
                case 1: serversTable->setItem(row, col, new QTableWidgetItem(data.servers[row][1]));
                    break; // hostname_or_IP
                case 6: serversTable->setItem(row, col, new QTableWidgetItem(QString::number(
                                                                                 serversConnected[data.servers[row][0]])));
                    break; // connected
                case 7: serversTable->setItem(row, col, new QTableWidgetItem(data.servers[row][2]));
                    break; // location
                case 8: serversTable->setItem(row, col, new QTableWidgetItem(data.servers[row][3]));
                    break; // name
                default: serversTable->setItem(row, col, new QTableWidgetItem());
            }
        }
        // styles
        QFont font; font.setBold(true);
        serversTable->item(row, 5)->setData(Qt::FontRole, font);
        serversTable->item(row, 2)->setTextAlignment(Qt::AlignCenter);
        serversTable->item(row, 3)->setTextAlignment(Qt::AlignCenter);
        serversTable->item(row, 4)->setTextAlignment(Qt::AlignCenter);
        serversTable->item(row, 5)->setTextAlignment(Qt::AlignCenter);
        serversTable->item(row, 6)->setTextAlignment(Qt::AlignCenter);
    }
    serversTable->resizeColumnsToContents();

    // VoiceServers
    voiceServersTable->clearContents();
    voiceServersTable->setRowCount(data.voiceServers.size());
    for (int row = 0; row < data.voiceServers.size(); row++) {
        for (int col=0; col < voiceServersTable->columnCount(); col++) {
            switch(col) {
                case 0: voiceServersTable->setItem(row, col, new QTableWidgetItem(data.voiceServers[row][0]));
                    break; // hostname_or_IP
                case 5: voiceServersTable->setItem(row, col, new QTableWidgetItem(QString::number(
                                                                                      voiceServerChannels[data.voiceServers[row][0].toLower()])));
                    break; // channels
                case 6: voiceServersTable->setItem(row, col, new QTableWidgetItem(data.voiceServers[row][1]));
                    break; // location
                case 7: voiceServersTable->setItem(row, col, new QTableWidgetItem(data.voiceServers[row][2]));
                    break; // name
                default: voiceServersTable->setItem(row, col, new QTableWidgetItem());
            }
        }
        voiceServerChannels.remove(data.voiceServers[row][0]); // we remove all we have displayed
        // styles
        QFont font; font.setBold(true);
        voiceServersTable->item(row, 4)->setData(Qt::FontRole, font);
        voiceServersTable->item(row, 1)->setTextAlignment(Qt::AlignCenter);
        voiceServersTable->item(row, 2)->setTextAlignment(Qt::AlignCenter);
        voiceServersTable->item(row, 3)->setTextAlignment(Qt::AlignCenter);
        voiceServersTable->item(row, 4)->setTextAlignment(Qt::AlignCenter);
        voiceServersTable->item(row, 5)->setTextAlignment(Qt::AlignCenter);
    }
    foreach(const QString server, voiceServerChannels.keys()) { // remaining voice servers not included in Whazzup
        voiceServersTable->setRowCount(voiceServersTable->rowCount() + 1);
        int row = voiceServersTable->rowCount() - 1;
        voiceServersTable->setItem(row, 0, new QTableWidgetItem(server));
        voiceServersTable->setItem(row, 1, new QTableWidgetItem());
        voiceServersTable->setItem(row, 2, new QTableWidgetItem());
        voiceServersTable->setItem(row, 3, new QTableWidgetItem());
        voiceServersTable->setItem(row, 4, new QTableWidgetItem());
        voiceServersTable->setItem(row, 5, new QTableWidgetItem(QString::number(
                                                                    voiceServerChannels[server])));
        voiceServersTable->setItem(row, 6, new QTableWidgetItem(
                                       "(not advertised in Whazzup)"));
        // styles
        voiceServersTable->item(row, 1)->setTextAlignment(Qt::AlignCenter);
        voiceServersTable->item(row, 2)->setTextAlignment(Qt::AlignCenter);
        voiceServersTable->item(row, 3)->setTextAlignment(Qt::AlignCenter);
        voiceServersTable->item(row, 4)->setTextAlignment(Qt::AlignCenter);
        voiceServersTable->item(row, 5)->setTextAlignment(Qt::AlignCenter);
        QFont font; font.setItalic(true);
        voiceServersTable->item(row, 6)->setData(Qt::FontRole, font);
    }
    voiceServersTable->resizeColumnsToContents();

    // Status
    QString msg = QString("Whazzup %1 updated")
            .arg(data.whazzupTime.date() == QDateTime::currentDateTimeUtc().date() // is today?
                 ? QString("today %1").arg(data.whazzupTime.time().toString("HHmm'z'"))
                 : (data.whazzupTime.isValid()
                    ? data.whazzupTime.toString("ddd yyyy/MM/dd HHmm'z'")
                    : "never")
                   );
    lblStatusInfo->setText(msg);

    // Set Item Titles
    toolBox->setItemText(0, QString("C&lients (%1)").arg(clients.size()));

    toolBox->setItemText(1, QString("&Servers (%1)").arg(data.servers.size()));
    toolBox->setItemEnabled(1, data.servers.size() > 0);

    toolBox->setItemText(2, QString("&Voice Servers (%1)").arg(data.voiceServers.size()));
    toolBox->setItemEnabled(2, data.voiceServers.size() > 0);

    performSearch();
    qApp->restoreOverrideCursor();
}

void ListClientsDialog::on_editFilter_textChanged(QString searchStr) {
    editFilterTimer.start(1000);
}

void ListClientsDialog::performSearch() {
    editFilterTimer.stop();
    qApp->setOverrideCursor(QCursor(Qt::WaitCursor));

    qDebug() << "ListClientsDialog::performSearch()";
    QRegExp regex;
    QStringList tokens = editFilter->text().trimmed().replace(QRegExp("\\*"), ".*").split(QRegExp("[ \\,]+"), QString::SkipEmptyParts);
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

    clientsProxyModel->setFilterRegExp(regex);
    clientsProxyModel->setFilterKeyColumn(-1);

    // General
    boxResults->setTitle(QString("Results (%1)").arg(clientsProxyModel->rowCount()));
    qApp->restoreOverrideCursor();
    qDebug() << "ListClientsDialog::performSearch() -- finished";
}

void ListClientsDialog::modelSelected(const QModelIndex& index) {
    clientsModel->modelSelected(clientsProxyModel->mapToSource(index));
}

void ListClientsDialog::pingReceived(QString server, int ms) {
    // Servers
    for (int row = 0; row < serversTable->rowCount(); row++) {
        if (serversTable->item(row, 1)->data(Qt::DisplayRole) == QVariant(server)) {
            for (int col = 2; col < 5; col++) {
                if (serversTable->item(row, col)->data(Qt::DisplayRole).isNull()) { // has no data
                    serversTable->item(row, col)->setBackground(QBrush(mapPingToColor(ms)));
                    serversTable->item(row, col)->setData(Qt::DisplayRole, (ms == -1? QVariant("n/a"): QVariant(ms)));

                    pingNextFromStack();

                    int addForAverage = 0;
                    for (int pingCol = 2; pingCol <= col; pingCol++) {
                        if(serversTable->item(row, pingCol)->data(Qt::DisplayRole).toInt() == 0) {
                            serversTable->item(row, 5)->setBackground(QBrush(mapPingToColor(-1)));
                            serversTable->item(row, 5)->setData(Qt::DisplayRole, QVariant("n/a"));
                            break;
                        } else {
                            addForAverage += serversTable->item(row, pingCol)->data(Qt::DisplayRole).toInt();
                        }
                        int average = addForAverage / (pingCol - 1);
                        serversTable->item(row, 5)->setBackground(QBrush(mapPingToColor(average)));
                        serversTable->item(row, 5)->setData(Qt::DisplayRole, QString("%1").arg(average));
                    }
                    break;
                }
            }
        }
    }

    // voiceServers
    for (int row = 0; row < voiceServersTable->rowCount(); row++) {
        if (voiceServersTable->item(row, 0)->data(Qt::DisplayRole) == QVariant(server)) {
            for (int col = 1; col < 4; col++) {
                if (voiceServersTable->item(row, col)->data(Qt::DisplayRole).isNull()) { // has no data
                    voiceServersTable->item(row, col)->setBackground(QBrush(mapPingToColor(ms)));
                    voiceServersTable->item(row, col)->setData(Qt::DisplayRole, (ms == -1? QVariant("n/a"): QVariant(ms)));

                    pingNextFromStack();

                    int addForAverage = 0;
                    for (int pingCol = 1; pingCol <= col; pingCol++) {
                        if(voiceServersTable->item(row, pingCol)->data(Qt::DisplayRole).toInt() == 0) {
                            voiceServersTable->item(row, 4)->setBackground(QBrush(mapPingToColor(-1)));
                            voiceServersTable->item(row, 4)->setData(Qt::DisplayRole, QVariant("n/a"));
                            break;
                        } else {
                            addForAverage += voiceServersTable->item(row, pingCol)->data(Qt::DisplayRole).toInt();
                        }
                        int average = addForAverage / pingCol;

                        voiceServersTable->item(row, 4)->setBackground(QBrush(mapPingToColor(average)));
                        voiceServersTable->item(row, 4)->setData(Qt::DisplayRole, QString("%1").arg(average));
                    }
                    break;
                }
            }
        }
    }
}

void ListClientsDialog::on_pbPingServers_clicked()
{
    for (int row = 0; row < serversTable->rowCount(); row++) {
        // reset Ping columns
        for (int col = 2; col < 6; col++) {
            serversTable->item(row, col)->setData(Qt::DisplayRole, QVariant());
            serversTable->item(row, col)->setBackground(QBrush());
        }
        pingStack.prepend(serversTable->item(row, 1)->data(Qt::DisplayRole).toString());
        pingStack.prepend(serversTable->item(row, 1)->data(Qt::DisplayRole).toString());
        pingStack.prepend(serversTable->item(row, 1)->data(Qt::DisplayRole).toString());
    }
    pingNextFromStack();
}

void ListClientsDialog::on_pbPingVoiceServers_clicked()
{
    for (int row = 0; row < voiceServersTable->rowCount(); row++) {
        // reset Ping columns
        for (int col = 1; col < 5; col++) {
            voiceServersTable->item(row, col)->setData(Qt::DisplayRole, QVariant());
            voiceServersTable->item(row, col)->setBackground(QBrush());
        }
        pingStack.prepend(voiceServersTable->item(row, 0)->data(Qt::DisplayRole).toString());
        pingStack.prepend(voiceServersTable->item(row, 0)->data(Qt::DisplayRole).toString());
        pingStack.prepend(voiceServersTable->item(row, 0)->data(Qt::DisplayRole).toString());
    }
    pingNextFromStack();
}

void ListClientsDialog::pingNextFromStack() {
    if (!pingStack.empty()) {
        Ping *ping = new Ping();
        connect(ping, SIGNAL(havePing(QString,int)), SLOT(pingReceived(QString,int)));
        ping->startPing(pingStack.pop());
    }
}

QColor ListClientsDialog::mapPingToColor(int ms) {
#define BEST 50 // ping in ms we consider best
#define WORST 250

    if(ms == -1) // "n/a"
        return QColor(255, 0, 0, 70);

    int red = qMin(230, qMax(0, (ms - BEST) * 255 / (WORST - BEST)));
    return QColor(red, 230 - red, 0, 70);
}

void ListClientsDialog::voiceServerClicked(int row, int col) {
    QUrl url = QUrl(
                QString("http://%1:18009/?opts=-R-D")
                .arg(voiceServersTable->item(row, 0)->data(Qt::DisplayRole).toString())
                , QUrl::TolerantMode);
    if(QMessageBox::question(this,
                             tr("Question"),
                             tr("Open %1 in your browser?")
                             .arg(url.toString()),
                             QMessageBox::Yes | QMessageBox::No
                             ) == QMessageBox::Yes) {
        if (url.isValid()) {
            if(!QDesktopServices::openUrl(url))
                QMessageBox::critical(this, tr("Error"), tr("Could not invoke browser"));
        } else
            QMessageBox::critical(this, tr("Error"), tr("URL %1 is invalid").arg(url.toString()));
    }
}

void ListClientsDialog::closeEvent(QCloseEvent *event){
    Settings::setListClientsDialogPos(pos());
    Settings::setListClientsDialogSize(size());
    Settings::setListClientsDialogGeometry(saveGeometry());
    event->accept();
}

void ListClientsDialog::showEvent(QShowEvent *event){
    editFilter->setFocus();
    event->accept();
}
