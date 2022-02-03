/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/
#include "SectorView.h"

Sectorview* sectorViewInstance = 0;
Sectorview* Sectorview::instance(bool createIfNoInstance, QWidget *parent) {
    if (sectorViewInstance == 0 && createIfNoInstance)
        sectorViewInstance = new Sectorview(parent);
    return sectorViewInstance;
}

Sectorview::Sectorview(QWidget *parent) :
        QDialog(parent) {
    setupUi(this);
    sectorsHash = NavData::instance()->sectors;
    loadSectorList();
}

Sectorview::~Sectorview() {
    for(int i = 0; i < listeView_sectors->count(); i++) {
        QListWidgetItem *item = listeView_sectors->currentItem();
        listeView_sectors->removeItemWidget(item);
        delete item;
    }
    MapScreen::instance()->glWidget->renderStaticSectors(false);
}

void Sectorview::loadSectorList() {
    qDebug() << "Sectorview::loadSectorList -- started";
    QList<Sector*> list = sectorsHash.values();
    for(int i = 0; i < list.size() ; i++) {
        QListWidgetItem *item = new QListWidgetItem();
        QString itemText = list[i]->icao;
        itemText.append("(");
        itemText.append( list[i]->id);
        itemText.append(")");
        item->setText( itemText);
        item->setCheckState(Qt::Checked);
        item->setCheckState(Qt::Unchecked);

        listeView_sectors->addItem(item);
        listeView_sectors->sortItems();
    }
    qDebug() << "Sectorview::loadSectorList -- finsished";
}

void Sectorview::on_bt_apply_clicked() {
    QStringList CheckedSectorList;
    QList<Sector*> renderSectors;

    for(int i = listeView_sectors->count()-1; i >= 0 ; i--) {
        if(listeView_sectors->item(i)->checkState() == Qt::Checked) {
            QString text = listeView_sectors->item(i)->text();
            CheckedSectorList.append(text.split("(").first());
        }
    }

    for(int i = CheckedSectorList.count()-1; i >=0 ; i--)
        renderSectors.append(sectorsHash[CheckedSectorList[i]]);

    MapScreen::instance()->glWidget->renderStaticSectors(true);
    MapScreen::instance()->glWidget->createStaticSectorLists(renderSectors);
    MapScreen::instance()->glWidget->update();

}
void Sectorview::on_bt_close_clicked() {
    MapScreen::instance()->glWidget->renderStaticSectors(false);
}



