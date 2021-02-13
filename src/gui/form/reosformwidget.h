/***************************************************************************
  reosformwidget.h - ReosFormWidget

 ---------------------
 begin                : 25.1.2021
 copyright            : (C) 2021 by Vincent Cloarec
 email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef REOSFORMWIDGET_H
#define REOSFORMWIDGET_H

#include <QWidget>
#include <QTableView>
#include <QDialog>
#include <QComboBox>


class QLayoutItem;
class QBoxLayout;
class ReosParameter;
class ReosDataObject;
class ReosTimeSerieConstantInterval;
class ReosTimeSerieConstantIntervalModel;
class ReosChicagoRainfall;
class ReosDoubleTriangleRainfall;
class ReosIntensityDurationSelectedCurveWidget;


class ReosFormWidget : public QWidget
{
    Q_OBJECT
  public:
    explicit ReosFormWidget( QWidget *parent = nullptr, Qt::Orientation orientation = Qt::Vertical, bool withSpacer = true );

    void addText( const QString &text, int position = -1 );
    void addParameter( ReosParameter *parameter, int position = -1 );
    void addParameters( QList<ReosParameter *> parameters );
    void addData( ReosDataObject *data, int position = -1 );
    void addWidget( QWidget *widget, int position = -1 );
    void addItem( QLayoutItem *item, int position = -1 );

    static ReosFormWidget *createDataWidget( ReosDataObject *dataObject, QWidget *parent = nullptr );

  signals:
    void parametersChanged();

  private:
    int mParamCount = 0;
    QBoxLayout *mMainLayout = nullptr;

};


class ReosFormDialog : public QDialog
{
    Q_OBJECT
  public:
    explicit ReosFormDialog( QWidget *parent = nullptr );
    void addParameter( ReosParameter *parameter );
    void addData( ReosDataObject *data );
    void addText( const QString &text );

  private:
    ReosFormWidget *mForm = nullptr;

};

//! Table view widget custmed to display/edit time series with constant time step
class ReosTimeSerieConstantIntervalView: public QTableView
{
    Q_OBJECT
  public:
    ReosTimeSerieConstantIntervalView( QWidget *parent = nullptr );

  signals:
    void pastDataFromClipboard( const QModelIndex &index, const QList<double> &data );
    void insertRow( const QModelIndex &fromIndex, int count );
    void deleteRows( const QModelIndex &fromIndex, int count );
    void insertRowFromClipboard( const QModelIndex &index, const QList<double> &data );

  protected:
    void keyPressEvent( QKeyEvent *event ) override;
    void contextMenuEvent( QContextMenuEvent *event ) override;

  private:
    QList<double> clipboardToValues();
};

//! Widget that can be uses to display/edits paramters of a ReosTimeSerieConstantInterval
class ReosTimeSerieConstantIntervalWidget: public ReosFormWidget
{
  public:
    explicit ReosTimeSerieConstantIntervalWidget( ReosTimeSerieConstantInterval *timeSerie, QWidget *parent );

  private:
    ReosTimeSerieConstantIntervalModel *mModel = nullptr;
    QComboBox *mValueModeComboBox = nullptr;
    QComboBox *mIntensityUnitComboBox = nullptr;
};

//! Widget that can be uses to display/edits paramters of a Chicago rainfall
class ReosChicagoRainfallWidget: public ReosTimeSerieConstantIntervalWidget
{
  public:
    explicit ReosChicagoRainfallWidget( ReosChicagoRainfall *rainfall, QWidget *parent );

  private:
    ReosIntensityDurationSelectedCurveWidget *mIdfWidget = nullptr;
};

//! Widget that can be uses to display/edits paramters of a Chicago rainfall
class ReosDoubleTriangleRainfallWidget: public ReosTimeSerieConstantIntervalWidget
{
  public:
    explicit ReosDoubleTriangleRainfallWidget( ReosDoubleTriangleRainfall *rainfall, QWidget *parent );

  private:
    ReosIntensityDurationSelectedCurveWidget *mIntenseIdfWidget = nullptr;
    ReosIntensityDurationSelectedCurveWidget *mTotalIdfWidget = nullptr;
};

#endif // REOSFORMWIDGET_H