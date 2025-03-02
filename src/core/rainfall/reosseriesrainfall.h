/***************************************************************************
  reosseriesrainfall.h - ReosSeriesRainfall

 ---------------------
 begin                : 22.11.2022
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
#ifndef REOSSERIESRAINFALL_H
#define REOSSERIESRAINFALL_H


#include "reoscore.h"
#include "reostimeseries.h"
#include "reosprocess.h"
#include "reosmemoryraster.h"

class ReosWatershed;
class ReosGriddedRainfall;
class ReosGriddedRainfallProvider;

class REOSCORE_EXPORT ReosSeriesRainfall : public ReosTimeSeriesConstantInterval
{
    Q_OBJECT
  public:
    ReosSeriesRainfall( QObject *parent = nullptr, const QString &providerKey = QString(), const QString &dataSource = QString() );

    QString type() const override {return staticType();}

#ifndef SIP_RUN

    ReosEncodedElement encode( const ReosEncodeContext &context ) const;
    //! Creates new instance from the encoded element
    static ReosSeriesRainfall *decode( const ReosEncodedElement &element, const ReosEncodeContext &context, QObject *parent = nullptr );
    static QString staticType();

  protected:
    explicit ReosSeriesRainfall( const ReosEncodedElement &element, const ReosEncodeContext &context, QObject *parent = nullptr );

#endif // No SIP_RUN
  private:
    void setupData();

};


class REOSCORE_EXPORT ReosSeriesRainfallFromGriddedOnWatershed : public ReosSeriesRainfall
{
    Q_OBJECT
  public:
    ReosSeriesRainfallFromGriddedOnWatershed( ReosWatershed *watershed, ReosGriddedRainfall *griddedRainfall, QObject *parent = nullptr );
    ~ReosSeriesRainfallFromGriddedOnWatershed();

    //! Returns a new created instance from \a watershed and \a gridded rainfall. Caller takes ownership.
    static ReosSeriesRainfallFromGriddedOnWatershed *create( ReosWatershed *watershed, ReosGriddedRainfall *griddedRainfall ) SIP_FACTORY;

    double valueAt( int i ) const override;

  signals:
    void calculationFinished();

#ifndef SIP_RUN

  protected:
    void updateData() const override;

#endif // No SIP_RUN

  private slots:
    void onWatershedGeometryChanged();
  private:

    class AverageCalculation : public ReosProcess
    {
      public:
        std::unique_ptr<ReosGriddedRainfallProvider> griddedRainfallProvider;
        QPolygonF watershedPolygon;
        ReosDuration timeStep;
        bool usePrecision = false;
        void start() override;

        ReosRasterMemory<double> rasterizedWatershed;
        ReosRasterExtent rasterizedExtent;
        int xOri = -1;
        int yOri = -1;
    };

    mutable AverageCalculation *mCurrentCalculation = nullptr;

    QPointer<ReosWatershed> mWatershed;
    QPointer<ReosGriddedRainfall> mGriddedRainfall;

    ReosRasterMemory<double> mRasterizedWatershed;
    ReosRasterExtent mRasterizedExtent;
    int mXOri = -1;
    int mYOri = -1;

    void launchCalculation();
    AverageCalculation *getCalculationProcess() const;

};


#endif // REOSSERIESRAINFALL_H
