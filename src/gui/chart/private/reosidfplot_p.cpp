/***************************************************************************
  reosidfplot_p.cpp - ReosIdfPlot_p

 ---------------------
 begin                : 5.2.2021
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

#include <QPainterPath>

#include "qwt_date.h"
#include "qwt_scale_map.h"
#include "qwt_plot.h"
#include "qwt_painter.h"

#include "reosidfplot_p.h"
#include "reosidfcurves.h"


ReosIdfPlot_p::ReosIdfPlot_p( ReosIntensityDurationCurve *curve ) :
  QwtPlotItem()
  , mCurve( curve )
{
  setZ( 15 );
  setItemAttribute( QwtPlotItem::Legend, true );
}

void ReosIdfPlot_p::draw( QPainter *painter, const QwtScaleMap &xMap, const QwtScaleMap &yMap, const QRectF &canvasRect ) const
{
  if ( mCurve.isNull() || mCurve->intervalCount() == 0 )
    return;

  painter->save();

  painter->setRenderHint( QPainter::Antialiasing,
                          true );

  for ( int i = 0; i < mCurve->intervalCount(); ++i )
  {
    if ( !mCurve->intervalIsValid( i ) )
      continue;

    QPair<ReosDuration, ReosDuration> timeInter = mCurve->timeInterval( i );

    double start = timeInter.first.valueUnit( mUnit );
    double end = timeInter.second.valueUnit( mUnit );
    if ( ( start < canvasRect.x() && end < canvasRect.x() ) ||
         ( start > canvasRect.x() + canvasRect.width() && end > canvasRect.x() + canvasRect.width() ) )
      continue;

    int pointCount = 20;

    double startPlot = start;
    double endPlot = end;

    if ( startPlot < canvasRect.x() )
      startPlot = canvasRect.x();

    if ( endPlot > canvasRect.x() + canvasRect.width() )
      endPlot = canvasRect.x() + canvasRect.width();

    double x1 = start;
    double y1 = mCurve->firstIntensity( i );

    double dx = ( endPlot - startPlot ) / ( pointCount - 1 );

    QPen pen;
    pen.setColor( mColor );
    pen.setWidthF( 3 );
    painter->setPen( pen );

    for ( int i = 1; i < pointCount - 1; ++i )
    {
      double x2 = startPlot + ( i ) * dx;
      double y2 = mCurve->intensity( ReosDuration( x2, mUnit ) );

      double xc1 = xMap.transform( x1 );
      double yc1 = yMap.transform( y1 );
      double xc2 = xMap.transform( x2 );
      double yc2 = yMap.transform( y2 );

      const QLineF line( xc1, yc1, xc2, yc2 );
      painter->drawLine( line );

      x1 = x2;
      y1 = y2;
    }

    double x2 = end;
    double y2 = mCurve->lastIntensity( i );

    double xc1 = xMap.transform( x1 );
    double yc1 = yMap.transform( y1 );
    double xc2 = xMap.transform( x2 );
    double yc2 = yMap.transform( y2 );
    const QLineF line( xc1, yc1, xc2, yc2 );
    painter->drawLine( line );

    x1 = start;
    y1 = mCurve->intensity( ReosDuration( x1, mUnit ) );
    xc1 = xMap.transform( x1 );
    yc1 = yMap.transform( y1 );

    QBrush brush;
    brush.setStyle( Qt::SolidPattern );
    brush.setColor( Qt::gray );
    pen.setWidthF( 1 );
    painter->setPen( pen );
    painter->setBrush( brush );
    painter->drawEllipse( xc1 - 2.5, yc1 - 2.5, 5, 5 );
    painter->drawEllipse( xc2 - 2.5, yc2 - 2.5, 5, 5 );
  }
  painter->restore();
}

void ReosIdfPlot_p::fullExtent()
{
  if ( !plot() )
    return;

  QRectF ext = extent();

  if ( !ext.isValid() )
    return;

  double dx = 0.1 *  ext.width() ;
  plot()->setAxisScale( QwtPlot::xBottom, ext.left() - dx, ext.right() + dx );
  double dy = 0.1 * ( ext.height() );
  plot()->setAxisScale( QwtPlot::yLeft, ext.top() - dy, ext.bottom() + dy );
  plot()->replot();
}

QRectF ReosIdfPlot_p::extent() const
{
  if ( mCurve.isNull() )
    return QRectF();

  return mCurve->extent( mUnit );
}

void ReosIdfPlot_p::setColor( const QColor &color )
{
  mColor = color;
}

QwtGraphic ReosIdfPlot_p::legendIcon( int index, const QSizeF & ) const
{
  Q_UNUSED( index );

  QwtGraphic graphic;
  graphic.setRenderHint( QwtGraphic::RenderPensUnscaled, true );

  QPainter painter( &graphic );
  painter.setRenderHint( QPainter::Antialiasing,
                         testRenderHint( QwtPlotItem::RenderAntialiased ) );

  QPen pen;
  pen.setColor( mColor );
  pen.setWidthF( 3 );
  pen.setCapStyle( Qt::FlatCap );

  painter.setPen( pen );
  QwtPainter::drawLine( &painter, 0.0, 0.0, 10.0, 10.0 );


  return graphic;
}



