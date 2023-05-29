/**************************************************************************
 *  This file is part of QuteScoop. See README for license
 **************************************************************************/

#ifndef SECTORVIEW_H
#define SECTORVIEW_H

#include "ui_SectorView.h"

#include "Sector.h"

class Sectorview : public QDialog, public Ui::Sectorview {
        Q_OBJECT
    public:
        explicit Sectorview(QWidget *parent = 0);
        static Sectorview* instance (bool createIfNoInstance = true, QWidget *parent = 0);
        ~Sectorview();
    protected slots:
        void on_bt_apply_clicked();
        void on_bt_close_clicked();
    private:
        void loadSectorList();
};


#endif // SECTORVIEW_H
