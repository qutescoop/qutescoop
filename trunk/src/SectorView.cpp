/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/


#include "SectorView.h"
#include "ui_sectorview.h"


Sectorview* instance = 0;

Sectorview::Sectorview(QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);
    sectorsHash = NavData::getInstance(true)->sectors;
    loadSectorList();
}

Sectorview::~Sectorview()
{

    for(int i = 0; i < listeView_sectors->count(); i++){
        QListWidgetItem *item = listeView_sectors->currentItem();
        listeView_sectors->removeItemWidget(item);
        delete item;
    }
    MapScreen::getInstance(true)->glWidget->renderStaticSectors(false);
}

Sectorview* Sectorview::getInstance(bool createIfNoInstance, QWidget *parent) {
    if(instance == 0)
        if (createIfNoInstance)
            instance = new Sectorview(parent);
    return instance;
}

void Sectorview::loadSectorList(){
    qDebug() << "Sectorview::loadSectorList -- started";
    QList<Sector*> list = sectorsHash.values();
    for(int i = 0; i < list.size() ; i++)
    {
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

void Sectorview::on_bt_apply_clicked()
{
    QStringList CheckedSectorList;
    QList<Sector*> renderSectors;

    for(int i = listeView_sectors->count()-1; i >= 0 ; i--)
    {
        if(listeView_sectors->item(i)->checkState() == Qt::Checked){
            QString text = listeView_sectors->item(i)->text();
            CheckedSectorList.append(text.split("(").first());
        }
    }

    for(int i = CheckedSectorList.count()-1; i >=0 ; i--){
        renderSectors.append(sectorsHash[CheckedSectorList[i]]);
    }

    MapScreen::getInstance(true)->glWidget->renderStaticSectors(true);
    MapScreen::getInstance(true)->glWidget->createSaticSectorLists(renderSectors);
    MapScreen::getInstance(true)->glWidget->update();

}
void Sectorview::on_bt_close_clicked()
{
    MapScreen::getInstance(true)->glWidget->renderStaticSectors(false);
}



