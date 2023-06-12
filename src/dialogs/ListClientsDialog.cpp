#include "ListClientsDialog.h"

#include "Window.h"
#include "../Controller.h"
#include "../models/ListClientsDialogModel.h"
#include "../Pilot.h"
#include "../Ping.h"
#include "../Settings.h"
#include "../Whazzup.h"

// singleton instance
ListClientsDialog* listClientsDialogInstance = 0;
ListClientsDialog* ListClientsDialog::instance(bool createIfNoInstance, QWidget* parent) {
    if (listClientsDialogInstance == 0 && createIfNoInstance) {
        if (parent == 0) {
            parent = Window::instance();
        }
        listClientsDialogInstance = new ListClientsDialog(parent);
    }
    return listClientsDialogInstance;
}

// destroys a singleton instance
void ListClientsDialog::destroyInstance() {
    delete listClientsDialogInstance;
    listClientsDialogInstance = 0;
}

ListClientsDialog::ListClientsDialog(QWidget* parent)
    : QDialog(parent) {
    setupUi(this);
    setWindowFlags(windowFlags() ^= Qt::WindowContextHelpButtonHint);

    // clients
    _clientsModel = new ListClientsDialogModel;
    _clientsProxyModel = new QSortFilterProxyModel;
    _clientsProxyModel->setDynamicSortFilter(true);
    _clientsProxyModel->setSourceModel(_clientsModel);
    treeListClients->setUniformRowHeights(true);
    treeListClients->setModel(_clientsProxyModel);
    treeListClients->sortByColumn(0, Qt::AscendingOrder);

    treeListClients->setColumnWidth(0, 100);
    treeListClients->setColumnWidth(1, 100);
    treeListClients->setColumnWidth(2, 200);
    treeListClients->setColumnWidth(3, 100);
    treeListClients->setColumnWidth(12, 800); // FP remarks
    treeListClients->setColumnWidth(13, 800); // controller info
    treeListClients->header()->setMinimumHeight(fontMetrics().lineSpacing() * 3);

    connect(treeListClients, &QAbstractItemView::clicked, this, &ListClientsDialog::modelSelected);

    // servers
    QStringList serverHeaders;
    serverHeaders << "ident" << "URL" << "ping[ms]" << "ping[ms]" << "ping[ms]" << QString::fromUtf8("Ø ping[ms]")
                  << "connected\nclients" << "location" << "description";
    serversTable->setColumnCount(serverHeaders.size());
    serversTable->setHorizontalHeaderLabels(serverHeaders);

    // General
    QFont font = lblStatusInfo->font();
    font.setPointSize(lblStatusInfo->fontInfo().pointSize() - 1);
    lblStatusInfo->setFont(font); //make it a bit smaller than standard text

    connect(&_editFilterTimer, &QTimer::timeout, this, &ListClientsDialog::performSearch);

    QTimer::singleShot(100, this, SLOT(refresh())); // delayed insertion of clients to open the window now

    auto preferences = Settings::dialogPreferences(m_preferencesName);
    if (!preferences.size.isNull()) {
        resize(preferences.size);
    }
    if (!preferences.pos.isNull()) {
        move(preferences.pos);
    }
    if (!preferences.geometry.isNull()) {
        restoreGeometry(preferences.geometry);
    }
}

void ListClientsDialog::refresh() {
    qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
    const WhazzupData &data = Whazzup::instance()->whazzupData();

    // Clients
    QHash<QString, int> serversConnected;
    QList<Client*> clients;
    foreach (Pilot* p, data.pilots) {
        clients << dynamic_cast<Client*> (p);
        if (p != 0) {
            serversConnected[p->server] = serversConnected.value(p->server, 0) + 1; // count clients
        }
    }
    foreach (Controller* c, data.controllers) {
        clients << dynamic_cast<Client*> (c);
        if (c != 0) {
            serversConnected[c->server] = serversConnected.value(c->server, 0) + 1; // count clients
        }
    }
    _clientsModel->setClients(clients);

    // Servers
    serversTable->clearContents();
    serversTable->setRowCount(data.servers.size());
    for (int row = 0; row < data.servers.size(); row++) {
        for (int col = 0; col < serversTable->columnCount(); col++) {
            switch (col) {
                case 0: serversTable->setItem(row, col, new QTableWidgetItem(data.servers[row][0]));
                    break; // ident
                case 1: serversTable->setItem(row, col, new QTableWidgetItem(data.servers[row][1]));
                    break; // hostname_or_IP
                case 6: serversTable->setItem(
                        row, col, new QTableWidgetItem(
                            QString::number(
                                serversConnected[data.servers[row][0]]
                            )
                        )
                );
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

    // Status
    QString msg = QString("Whazzup %1 updated")
        .arg(
        data.whazzupTime.date() == QDateTime::currentDateTimeUtc().date() // is today?
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

    performSearch();
    qApp->restoreOverrideCursor();
}

void ListClientsDialog::on_editFilter_textChanged(QString) {
    _editFilterTimer.start(1000);
}

void ListClientsDialog::performSearch() {
    _editFilterTimer.stop();
    qApp->setOverrideCursor(QCursor(Qt::WaitCursor));

    qDebug() << "ListClientsDialog::performSearch()";
    QRegExp regex;
    // @todo this tries to cater for both ways (wildcards and regexp) but it does a bad job at that.
    QStringList tokens = editFilter->text()
        .replace(QRegExp("\\*"), ".*")
        .split(QRegExp("[ \\,]+"), Qt::SkipEmptyParts);
    if (tokens.size() == 1) {
        regex = QRegExp("^" + tokens.first() + ".*", Qt::CaseInsensitive);
    } else if (tokens.size() == 0) {
        regex = QRegExp("");
    } else {
        QString regExpStr = "^(" + tokens.first();
        for (int i = 1; i < tokens.size(); i++) {
            regExpStr += "|" + tokens[i];
        }
        regExpStr += ".*)";
        regex = QRegExp(regExpStr, Qt::CaseInsensitive);
    }

    _clientsProxyModel->setFilterRegExp(regex);
    _clientsProxyModel->setFilterKeyColumn(-1);

    // General
    boxResults->setTitle(QString("Results (%1)").arg(_clientsProxyModel->rowCount()));
    qApp->restoreOverrideCursor();
    qDebug() << "ListClientsDialog::performSearch() -- finished";
}

void ListClientsDialog::modelSelected(const QModelIndex& index) {
    _clientsModel->modelSelected(_clientsProxyModel->mapToSource(index));
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
                        if (serversTable->item(row, pingCol)->data(Qt::DisplayRole).toInt() == 0) {
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
}

void ListClientsDialog::on_pbPingServers_clicked() {
    for (int row = 0; row < serversTable->rowCount(); row++) {
        // reset Ping columns
        for (int col = 2; col < 6; col++) {
            serversTable->item(row, col)->setData(Qt::DisplayRole, QVariant());
            serversTable->item(row, col)->setBackground(QBrush());
        }
        _pingStack.prepend(serversTable->item(row, 1)->data(Qt::DisplayRole).toString());
        _pingStack.prepend(serversTable->item(row, 1)->data(Qt::DisplayRole).toString());
        _pingStack.prepend(serversTable->item(row, 1)->data(Qt::DisplayRole).toString());
    }
    pingNextFromStack();
}

void ListClientsDialog::pingNextFromStack() {
    if (!_pingStack.empty()) {
        Ping* ping = new Ping();
        connect(ping, &Ping::havePing, this, &ListClientsDialog::pingReceived);
        ping->startPing(_pingStack.pop());
    }
}

QColor ListClientsDialog::mapPingToColor(int ms) {
#define BEST 50 // ping in ms we consider best
#define WORST 250

    if (ms == -1) { // "n/a"
        return QColor(255, 0, 0, 70);
    }

    int red = qMin(230, qMax(0, (ms - BEST) * 255 / (WORST - BEST)));
    return QColor(red, 230 - red, 0, 70);
}

void ListClientsDialog::closeEvent(QCloseEvent* event) {
    Settings::setDialogPreferences(
        m_preferencesName,
        Settings::DialogPreferences {
            .size = size(),
            .pos = pos(),
            .geometry = saveGeometry()
        }
    );
    event->accept();
}

void ListClientsDialog::showEvent(QShowEvent* event) {
    editFilter->setFocus();
    event->accept();
}
