#ifndef AIRPORTDETAILS_H_
#define AIRPORTDETAILS_H_

#include "ui_AirportDetails.h"

#include "ClientDetails.h"
#include "../Airport.h"
#include "../models/MetarModel.h"
#include "../models/AirportDetailsAtcModel.h"
#include "../models/AirportDetailsArrivalsModel.h"
#include "../models/AirportDetailsDeparturesModel.h"

class AirportDetails
    : public ClientDetails, private Ui::AirportDetails {
    Q_OBJECT
    public:
        static AirportDetails* instance(bool createIfNoInstance = true, QWidget* parent = 0);
        void destroyInstance();
        void refresh(Airport* _airport = 0);

    protected:
        void closeEvent(QCloseEvent* event);

    private slots:
        void onGotMetar(const QString &airportLabel, const QString &encoded, const QString &humanHtml);
        void refreshMetar();
        void togglePlotRoutes(bool checked);
        void toggleShowOtherAtc(bool checked);
        void atcSelected(const QModelIndex &index);
        void arrivalSelected(const QModelIndex &index);
        void departureSelected(const QModelIndex &index);

    private:
        AirportDetails(QWidget* parent);

        AirportDetailsAtcModel _atcModel;
        AirportDetailsArrivalsModel _arrivalsModel;
        AirportDetailsDeparturesModel _departuresModel;
        Airport* _airport;
        QSortFilterProxyModel* _atcSortModel, * _arrivalsSortModel, * _departuresSortModel;

        QSet<Controller*> checkSectors() const;

        MetarModel* _metarModel;

        constexpr static char m_preferencesName[] = "airportDetails";
};
#endif /*AIRPORTDETAILS_H_*/
