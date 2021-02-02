/***************************************************************************
  reosparameter.cpp - ReosParameter

 ---------------------
 begin                : 22.1.2021
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
#include "reosparameter.h"


ReosParameter::ReosParameter( const QString &name, QObject *parent ):
  QObject( parent )
  , mName( name )
{

}

QString ReosParameter::name() const
{
  return mName;
}

bool ReosParameter::isDerivable() const
{
  return mIsDerivable;
}

bool ReosParameter::isDerived() const
{
  return mIsDerived;
}

void ReosParameter::askForDerivation()
{
  emit needDerivation();
}

void ReosParameter::encode( ReosEncodedElement &element ) const
{
  element.addData( QStringLiteral( "name" ), mName );
  element.addData( QStringLiteral( "is-derived" ), mIsDerived );
}

void ReosParameter::decode( const ReosEncodedElement &element, bool isDerivable )
{
  mIsDerivable = isDerivable;

  if ( !element.getData( QStringLiteral( "name" ), mName ) )
    return;

  if ( !element.getData( QStringLiteral( "is-derived" ), mIsDerived ) )
    return;
}


ReosParameterArea::ReosParameterArea( const QString &name, QObject *parent ):
  ReosParameter( name, parent )
{}

void ReosParameterArea::setValue( const ReosArea &area )
{
  mValue = area;
  mIsDerived = false;
}

void ReosParameterArea::setDerivedValue( const ReosArea &area )
{
  mValue = area;
  mIsDerived = true;
  emit valueChanged();
}

void ReosParameterArea::changeUnit( ReosArea::Unit unit )
{
  mValue.setUnit( unit );
}

ReosEncodedElement ReosParameterArea::encode() const
{
  ReosEncodedElement element( QStringLiteral( "area-parameter" ) );
  ReosParameter::encode( element );

  element.addEncodedData( QStringLiteral( "area-value" ), mValue.encode() );

  return element;
}

ReosParameterArea *ReosParameterArea::decode( const ReosEncodedElement &element, bool isDerivable, QObject *parent )
{
  ReosParameterArea *ret = new ReosParameterArea( QString(), parent );

  if ( element.description() != QStringLiteral( "area-parameter" ) )
    return ret;

  ret->ReosParameter::decode( element, isDerivable );

  ret->mValue = ReosArea::decode( element.getEncodedData( QStringLiteral( "area-value" ) ) );

  return ret;
}

void ReosParameter::setDerivable( bool b )
{
  mIsDerivable = b;
}

ReosParameterSlope::ReosParameterSlope( const QString &name, QObject *parent ):
  ReosParameter( name, parent )
{

}

void ReosParameterSlope::setValue( double slope )
{
  mSlope = slope;
  mIsDerived = false;
  emit valueChanged();
}

void ReosParameterSlope::setDerivedValue( double slope )
{
  mSlope = slope;
  mIsDerived = true;
  emit valueChanged();
}

ReosEncodedElement ReosParameterSlope::encode() const
{
  ReosEncodedElement element( QStringLiteral( "slope-parameter" ) );
  ReosParameter::encode( element );

  element.addData( QStringLiteral( "slope-value" ), mSlope );

  return element;
}

ReosParameterSlope *ReosParameterSlope::decode( const ReosEncodedElement &element, bool isDerivable, QObject *parent )
{
  ReosParameterSlope *ret = new ReosParameterSlope( tr( "Average slope" ), parent );

  if ( element.description() != QStringLiteral( "slope-parameter" ) )
    return ret;

  ret->ReosParameter::decode( element, isDerivable );

  if ( !element.getData( QStringLiteral( "slope-value" ), ret->mSlope ) )
    ret->mSlope = 0;

  return ret;
}

ReosParameterString::ReosParameterString( const QString &name, QObject *parent ):
  ReosParameter( name, parent )
{
  setDerivable( false );
}

void ReosParameterString::setValue( const QString &string )
{
  mValue = string;
  emit valueChanged();
}

ReosEncodedElement ReosParameterString::encode() const
{
  ReosEncodedElement element( QStringLiteral( "string-parameter" ) );
  ReosParameter::encode( element );

  element.addData( QStringLiteral( "string-value" ), mValue );

  return element;
}

ReosParameterString *ReosParameterString::decode( const ReosEncodedElement &element, bool isDerivable, QObject *parent )
{
  ReosParameterString *ret = new ReosParameterString( QString(), parent );

  if ( element.description() != QStringLiteral( "string-parameter" ) )
    return ret;

  ret->ReosParameter::decode( element, isDerivable );

  element.getData( QStringLiteral( "string-value" ), ret->mValue );

  return ret;
}

ReosParameterDuration::ReosParameterDuration( const QString &name, QObject *parent ):
  ReosParameter( name, parent )
{

}

void ReosParameterDuration::setValue( const ReosDuration &duration )
{
  mDuration = duration;
  mIsDerived = false;
  emit valueChanged();
}

void ReosParameterDuration::setDerivedValue( const ReosDuration &duration )
{
  mDuration = duration;
  mIsDerived = true;
  emit valueChanged();
}

void ReosParameterDuration::changeUnit( ReosDuration::Unit unit )
{
  mDuration.setUnit( unit );
  emit unitChanged();
}

ReosDuration ReosParameterDuration::value() const {return mDuration;}

ReosEncodedElement ReosParameterDuration::encode() const
{
  ReosEncodedElement element( QStringLiteral( "duration-parameter" ) );
  ReosParameter::encode( element );

  element.addEncodedData( QStringLiteral( "duration-value" ), mDuration.encode() );

  return element;
}

ReosParameterDuration *ReosParameterDuration::decode( const ReosEncodedElement &element, bool isDerivable, QObject *parent )
{
  ReosParameterDuration *ret = new ReosParameterDuration( QString(), parent );

  if ( element.description() != QStringLiteral( "duration-parameter" ) )
    return ret;

  ret->ReosParameter::decode( element, isDerivable );

  ret->mDuration = ReosDuration::decode( element.getEncodedData( QStringLiteral( "duration-value" ) ) );

  return ret;
}

ReosParameterDateTime::ReosParameterDateTime( const QString &name, QObject *parent ):
  ReosParameter( name, parent )
{
  mDateTime.setDate( QDate( QDate::currentDate().year(), 1, 1 ) );
}

void ReosParameterDateTime::setValue( const QDateTime &dt )
{
  mDateTime = dt;
  mIsDerived = false;
  emit valueChanged();
}

void ReosParameterDateTime::setDerivedValue( const QDateTime &dt )
{
  mDateTime = dt;
  mIsDerived = true;
  emit valueChanged();
}

ReosEncodedElement ReosParameterDateTime::encode() const
{
  ReosEncodedElement element( QStringLiteral( "datetime-parameter" ) );
  ReosParameter::encode( element );

  element.addData( QStringLiteral( "date-time-value" ), mDateTime );

  return element;
}

ReosParameterDateTime *ReosParameterDateTime::decode( const ReosEncodedElement &element, bool isDerivable, QObject *parent )

{
  ReosParameterDateTime *ret = new ReosParameterDateTime( tr( "Date/Time" ), parent );

  if ( element.description() != QStringLiteral( "datetime-parameter" ) )
    return ret;

  ret->ReosParameter::decode( element, isDerivable );

  if ( !element.getData( QStringLiteral( "date-time-value" ), ret->mDateTime ) )
    ret->mDateTime = QDateTime();

  return ret;
}
