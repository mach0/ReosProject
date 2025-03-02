/***************************************************************************
  reosgribprecipitationwidget.h - ReosGribPrecipitationWidget

 ---------------------
 begin                : 16.11.2022
 copyright            : (C) 2022 by Vincent Cloarec
 email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef REOSGRIBPRECIPITATIONWIDGET_H
#define REOSGRIBPRECIPITATIONWIDGET_H

#include "reosdataprovidergui.h"
#include "reosmapitem.h"
#include "reosgribprovider.h"
#include "reosgriddedrainfallselectorwidget.h"

class ReosGriddedRainfall;
class ReosGribGriddedRainfallProvider;

namespace Ui
{
  class ReosGribPrecipitationWidget;
}

class ReosGribPrecipitationWidget :  public ReosGriddedRainDataProviderSelectorWidget
{
    Q_OBJECT

  public:
    explicit ReosGribPrecipitationWidget( QWidget *parent = nullptr );
    ~ReosGribPrecipitationWidget();

    virtual QVariantMap selectedMetadata() const override;

    /**
     * Creates an return a pointer to a new data object with the current selected data,
     * \a parent take ownership of the object, or the caller need to take it if \a parent is nullptr
     * Default implementation return a null pointer
     */
    virtual ReosGriddedRainfall *createData( QObject *parent = nullptr ) const override;

    //! Returns a pointer to the current selected object, default implementation return a null pointer.
    virtual ReosDataObject *selectedData() const override;

    ReosGriddedRainfallProvider::FileDetails setSource(const QString &source , ReosModule::Message &message) override;

  private slots:
    void onPathChanged();
    void updateRainfall();

  private:
    Ui::ReosGribPrecipitationWidget *ui;
    QString mSource;
    QString mCurrentVariable;
    ReosGriddedRainfallProvider::FileDetails mDetails;
    std::unique_ptr<ReosGriddedRainfall> mCurrentRainfall;
    std::unique_ptr<ReosGribGriddedRainfallProvider> mProvider;
    bool mCurrentSourceIsValid = false;
    QString mCurrentDataUri;
};


class ReosGribGuiFactory : public ReosDataProviderGuiFactory
{
  public:
    GuiCapabilities capabilities() const override;
    QString key() const override;
    ReosGribPrecipitationWidget *createProviderSelectorWidget( ReosMap *map, const QString &dataType, QWidget *parent = nullptr ) const override;
    ReosDataProviderSettingsWidget *createProviderSettingsWidget( ReosDataProvider *provider, QWidget *parent = nullptr ) const override;
    QString dataType() const override;

    QString displayText() const override;
};

#endif // REOSGRIBPRECIPITATIONWIDGET_H
