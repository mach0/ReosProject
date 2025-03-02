/***************************************************************************
  reosgribprovider.cpp - ReosGribProvider

 ---------------------
 begin                : 11.11.2022
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

#include <QFileInfo>
#include <QDir>
#include <QLocale>

#include "reosgribprovider.h"
#include "reosgriddedrainitem.h"
#include "reosgdalutils.h"

REOSEXTERN ReosDataProviderFactory *providerFactory()
{
  return new ReosGribProviderFactory();
}

ReosGribGriddedRainfallProvider::ReosGribGriddedRainfallProvider()
{
  mCache.setMaxCost( 20000000 );
}

ReosGriddedRainfallProvider *ReosGribGriddedRainfallProvider::clone() const
{
  std::unique_ptr<ReosGribGriddedRainfallProvider> other = std::make_unique<ReosGribGriddedRainfallProvider>();
  other->setDataSource( dataSource() );

  return other.release();
}

void ReosGribGriddedRainfallProvider::load()
{
  QString fileSource = sourcePathFromUri( dataSource() );
  QString varName = variableFromUri( dataSource() );
  mSourceValueType = valueTypeFromUri( dataSource() );
  mIsValid = false;

  QMap<qint64, GribFrame> pathes;

  if ( !sourceIsValid( fileSource, mLastMessage ) )
    return;
  QDir dir;
  QFileInfo fileInfo( fileSource );
  if ( fileInfo.isDir() )
  {
    QStringList filters;
    dir = QDir( fileSource );
    filters << QStringLiteral( "*.grib2" );
    filters << QStringLiteral( "*.grb2" );
    const QStringList files = dir.entryList( filters, QDir::Files );
    for ( const QString &file : files )
      parseFile( dir.filePath( file ), varName, mReferenceTime, pathes, mExtent );

  }
  else if ( fileInfo.isFile() )
  {
    parseFile( fileSource, varName, mReferenceTime, pathes, mExtent );
  }

  mFrames = pathes.values();
  mIsValid = true;

  emit dataReset();
  emit loadingFinished();
}

QStringList ReosGribGriddedRainfallProvider::fileSuffixes() const
{
  QStringList ret;
  ret << QStringLiteral( "grib2" )
      << QStringLiteral( "grb2" );

  return ret;
}

ReosGriddedRainfallProvider::FileDetails ReosGribGriddedRainfallProvider::details( const QString &source, ReosModule::Message &message ) const
{
  FileDetails ret;
  QDir dir;

  if ( !sourceIsValid( source, message ) )
    return ret;

  QStringList files = getFiles( source, dir );

  if ( files.isEmpty() )
  {
    message.type = ReosModule::Error;
    message.text = tr( "No Grib2 source found in \"%1\"." ).arg( source );
  }

  bool hasExtent = false;

  for ( const QString &file : std::as_const( files ) )
  {
    ReosGdalDataset dataset( dir.filePath( file ) );

    int bandCount = dataset.bandCount();

    if ( bandCount == 0 )
    {
      message.type = ReosModule::Error;
      message.text = tr( "No data found in \"%1\"." ).arg( source );
      return ret;
    }

    for ( int i = 1; i <= bandCount; ++i )
    {
      QMap<QString, QString> metadata = dataset.bandMetadata( i );
      auto it = metadata.find( QStringLiteral( "GRIB_COMMENT" ) );
      if ( it == metadata.end() )
        continue;

      if ( ret.availableVariables.contains( it.value() ) )
        continue;

      ret.availableVariables.append( it.value() );
      if ( !hasExtent )
        ret.extent = dataset.extent();
    }
  }

  if ( ret.availableVariables.isEmpty() )
  {
    message.type = ReosModule::Error;
    message.text = tr( "No data found in \"%1\"." ).arg( source );
  }

  ret.files = files;

  giveName( ret );
  return ret;
}

bool ReosGribGriddedRainfallProvider::isValid() const
{
  return mIsValid;
}

int ReosGribGriddedRainfallProvider::count() const
{
  return mFrames.count();
}

bool ReosGribGriddedRainfallProvider::canReadUri( const QString &path ) const
{
  QDir dir;
  ReosModule::Message message;
  if ( !sourceIsValid( path, message ) )
    return false;

  QStringList files = getFiles( path, dir );

  return !files.empty() && dir.exists();
}

QDateTime ReosGribGriddedRainfallProvider::startTime( int index ) const
{
  switch ( mSourceValueType )
  {
    case ValueType::CumulativeHeight:
      if ( index == 0 )
        return QDateTime::fromSecsSinceEpoch( mReferenceTime, Qt::UTC );
      else
        return QDateTime::fromSecsSinceEpoch( mFrames.at( index - 1 ).validTime, Qt::UTC );
      break;
    case ValueType::Height:
    case ValueType::Intensity:
      return QDateTime();
      break;
  }

  return QDateTime();
}

QDateTime ReosGribGriddedRainfallProvider::endTime( int index ) const
{
  switch ( mSourceValueType )
  {
    case ValueType::CumulativeHeight:
      return QDateTime::fromSecsSinceEpoch( mFrames.at( index ).validTime, Qt::UTC );
      break;
    case ValueType::Height:
    case ValueType::Intensity:
      return QDateTime();
      break;
  }

  return QDateTime();;
}

const QVector<double> ReosGribGriddedRainfallProvider::data( int index ) const
{
  if ( index < 0 )
    return QVector<double>();

  CacheValues *cache = mCache.object( index );

  if ( cache && cache->typeCalculatedFrom == mSourceValueType )
    return cache->values;

  if ( cache )
    mCache.remove( index );

  ReosGdalDataset dataset( mFrames.at( index ).file );
  if ( !dataset.isValid() )
    return QVector<double>();

  ReosRasterMemory<double> raster = dataset.values( mFrames.at( index ).bandNo );
  ReosDuration duration = intervalDuration( index );

  switch ( mSourceValueType )
  {
    case ValueType::CumulativeHeight:
    {
      QVector<double> ret( raster.values().count(),  std::numeric_limits<double>::quiet_NaN() ) ;

      if ( index == 0 )
      {
        for ( int i = 0; i < ret.count(); ++i )
        {
          if ( raster.values().at( i ) != 0.0 )
            ret[i] = raster.values().at( i ) / duration.valueHour();
        }
        return ret;;
      }

      ReosGdalDataset prevDataset( mFrames.at( index - 1 ).file );
      ReosRasterMemory<double> prevRaster = prevDataset.values( mFrames.at( index - 1 ).bandNo );
      const QVector<double> &prevIndex = prevRaster.values();
      const QVector<double> &currentIndex = raster.values();
      Q_ASSERT( prevIndex.count() == currentIndex.count() );


      for ( int i = 0; i < ret.count(); ++i )
      {
        double val =  std::max( ( currentIndex.at( i ) - prevIndex.at( i ) ) / duration.valueHour(), 0.0 );
        if ( val != 0.0 )
          ret[i] = val;
      }

      return ret;
    }
    break;
    case ValueType::Height:
    case ValueType::Intensity:
    {
      QVector<double> ret( raster.values().count(),  std::numeric_limits<double>::quiet_NaN() ) ;
      for ( int i = 0; i < ret.count(); ++i )
      {
        if ( raster.values().at( i ) != 0.0 )
          ret[i] = raster.values().at( i ) ;
      }
      return ret;
    }
    break;
  }

  return QVector<double>();
}

bool ReosGribGriddedRainfallProvider::getDirectMinMax( double &min, double &max ) const
{
  if ( mHasMinMaxCalculated )
  {
    min = mMin;
    max = mMax;
  }
  return mHasMinMaxCalculated;
}

void ReosGribGriddedRainfallProvider::calculateMinMax( double &min, double &max ) const
{
  mMin = std::numeric_limits<double>::max();
  mMax = -std::numeric_limits<double>::max();

  int gridCount = count();

  for ( int i = 0; i < gridCount; ++i )
  {
    const QVector<double> vals = data( i );
    for ( const double &v : vals )
    {
      if ( v < mMin )
        mMin = v;
      if ( v > mMax )
        mMax = v;
    }
  }
  min = mMin;
  max = mMax;
  mHasMinMaxCalculated = true;
}

QString ReosGribGriddedRainfallProvider::htmlDescription() const
{
  QString htmlText = QStringLiteral( "<html>\n<body>\n" );
  htmlText += QLatin1String( "<table class=\"list-view\">\n" );

  htmlText += QStringLiteral( "<h2>" ) + tr( "Gridded Precipitation" ) + QStringLiteral( "</h2>\n<hr>\n" );

  htmlText += QStringLiteral( "<tr><td class=\"highlight\">" )
              + QStringLiteral( "<b>%1</b>" ).arg( tr( "Format" ) ) + QStringLiteral( "</td><td>" )
              + QStringLiteral( "GRIB 2" ) + QStringLiteral( "</td></tr>\n" );

  htmlText += QStringLiteral( "<tr><td class=\"highlight\">" )
              + QStringLiteral( "<b>%1</b>" ).arg( tr( "Source" ) ) + QStringLiteral( "</td><td>" )
              + sourcePathFromUri( dataSource() ) + QStringLiteral( "</td></tr>\n" );

  htmlText += QStringLiteral( "<tr><td class=\"highlight\">" )
              + QStringLiteral( "<b>%1</b>" ).arg( tr( "Variable" ) ) + QStringLiteral( "</td><td>" )
              + variableFromUri( dataSource() ) + QStringLiteral( "</td></tr>\n" );

  if ( count() > 0 )
  {
    htmlText += QStringLiteral( "<tr><td class=\"highlight\">" )
                +  QStringLiteral( "<b>%1</b>" ).arg( tr( "Start date" ) ) + QStringLiteral( "</td><td>" )
                + startTime( 0 ).toString( QLocale().dateTimeFormat() )
                + QStringLiteral( "</td></tr>\n" );

    htmlText += QStringLiteral( "<tr><td class=\"highlight\">" )
                +  QStringLiteral( "<b>%1</b>" ).arg( tr( "End date" ) ) + QStringLiteral( "</td><td>" )
                + endTime( count() - 1 ).toString( QLocale().dateTimeFormat() )
                + QStringLiteral( "</td></tr>\n" );
  }

  return htmlText;
}

ReosRasterExtent ReosGribGriddedRainfallProvider::extent() const
{
  return mExtent;
}

QString ReosGribGriddedRainfallProvider::dataType() {return ReosGriddedRainfall::staticType();}

QString ReosGribGriddedRainfallProvider::staticKey()
{
  return GRIB_KEY + QString( "::" ) + dataType();
}

QString ReosGribGriddedRainfallProvider::uri( const QString &sourcePath, const QString &variable, ValueType valueType )
{
  QString stringValueType;

  switch ( valueType )
  {
    case ValueType::CumulativeHeight:
      stringValueType = QStringLiteral( "cumulative" );
      break;
    case ValueType::Height:
      stringValueType = QStringLiteral( "height" );
    case ValueType::Intensity:
      stringValueType = QStringLiteral( "intensity" );
      break;
  }
  return QStringLiteral( "\"%1\"::%2::%3" ).arg( sourcePath, variable, stringValueType );
}

QString ReosGribGriddedRainfallProvider::sourcePathFromUri( const QString &uri )
{
  const QStringList part = uri.split( QStringLiteral( "::" ) );
  if ( part.count() == 0 )
    return QString();

  QString source = part.at( 0 );
  source.remove( QStringLiteral( "\"" ) );
  return source;
}

QString ReosGribGriddedRainfallProvider::variableFromUri( const QString &uri )
{
  const QStringList part = uri.split( QStringLiteral( "::" ) );
  if ( part.count() < 2 )
    return QString();

  return part.at( 1 );
}

ReosGriddedRainfallProvider::ValueType ReosGribGriddedRainfallProvider::valueTypeFromUri( const QString &uri )
{
  const QStringList part = uri.split( QStringLiteral( "::" ) );
  if ( part.count() < 3 )
    return ValueType::Height;

  if ( part.at( 2 ) == QStringLiteral( "cumulative" ) )
    return ValueType::CumulativeHeight;

  if ( part.at( 2 ) == QStringLiteral( "intensity" ) )
    return ValueType::Intensity;

  if ( part.at( 2 ) == QStringLiteral( "height" ) )
    return ValueType::Height;

  return ValueType::Height;
}

bool ReosGribGriddedRainfallProvider::sourceIsValid( const QString &source, ReosModule::Message &message ) const
{
  QFileInfo fileInfo( source );
  if ( fileInfo.isDir() )
  {
    QDir dir( source );
    if ( !dir.exists() )
    {
      message.type = ReosModule::Error;
      message.text = tr( "Folder \"%1\" does not exist." ).arg( source );
      return false;
    }
  }
  else if ( fileInfo.isFile() )
  {
    if ( !fileInfo.exists() )
    {
      message.type = ReosModule::Error;
      message.text = tr( "File \"%1\" does not exist." ).arg( source );
      return false;
    }
  }
  else
  {
    message.type = ReosModule::Error;
    message.text = tr( "Unknow source \"%1\"." ).arg( source );
    return false;
  }

  return true;

}

ReosEncodedElement ReosGribGriddedRainfallProvider::encode( const ReosEncodeContext &context ) const
{
  ReosEncodedElement element( QStringLiteral( "grib-gridded-precipitation" ) );

  QString uriToEncode = dataSource();

  QString sourcePath = sourcePathFromUri( uriToEncode );
  sourcePath = context.pathToEncode( sourcePath );
  uriToEncode = uri( sourcePath, variableFromUri( uriToEncode ), valueTypeFromUri( uriToEncode ) );
  element.addData( QStringLiteral( "data-source" ), uriToEncode );

  return element;
}

void ReosGribGriddedRainfallProvider::decode( const ReosEncodedElement &element, const ReosEncodeContext &context )
{
  if ( element.description() != QStringLiteral( "grib-gridded-precipitation" ) )
    return;
  QString source;
  if ( element.getData( QStringLiteral( "data-source" ), source ) )
  {
    QString sourcePath = sourcePathFromUri( source );
    sourcePath = context.resolvePath( sourcePath );
    source = uri( sourcePath, variableFromUri( source ), valueTypeFromUri( source ) );
    setDataSource( source );
  }

}

void ReosGribGriddedRainfallProvider::parseFile(
  const QString &fileName,
  const QString &varName,
  qint64 &referenceTime,
  QMap<qint64, GribFrame> &pathes,
  ReosRasterExtent &extent ) const
{
  ReosGdalDataset dataset( fileName );
  if ( !dataset.isValid() )
    return;

  int bandCount = dataset.bandCount();
  for ( int bi = 1; bi <= bandCount; ++bi )
  {
    const QMap<QString, QString> metadata = dataset.bandMetadata( bi );

    if ( metadata.value( QStringLiteral( "GRIB_COMMENT" ) ) == varName )
    {
      if ( pathes.isEmpty() )
        extent = dataset.extent();
      else
      {
        if ( extent != dataset.extent() )
          continue;
      }

      GribFrame path;
      path.file = fileName;
      path.bandNo = bi;

      QString strRefTime = metadata.value( QStringLiteral( "GRIB_REF_TIME" ) );
      if ( strRefTime.isEmpty() )
        continue;
      strRefTime = strRefTime.split( ' ', Qt::SplitBehaviorFlags::SkipEmptyParts ).at( 0 );
      bool ok = false;
      qint64 refTime = strRefTime.toInt( &ok );
      if ( !ok )
        continue;
      if ( mReferenceTime == -1 )
        referenceTime = refTime;
      else if ( refTime != mReferenceTime )
        continue;

      QString strValidTime = metadata.value( QStringLiteral( "GRIB_VALID_TIME" ) );
      if ( strValidTime.isEmpty() )
        continue;
      strValidTime = strValidTime.split( ' ', Qt::SplitBehaviorFlags::SkipEmptyParts ).at( 0 );
      ok = false;
      path.validTime = strValidTime.toInt( &ok );
      if ( !ok )
        continue;
      pathes.insert( path.validTime, path );
    }
  }
}

QStringList ReosGribGriddedRainfallProvider::getFiles( const QString &path, QDir &dir ) const
{
  QStringList files;
  QFileInfo fileInfo( path );
  if ( fileInfo.isDir() )
  {
    QStringList filters;
    dir = QDir( path );
    filters << QStringLiteral( "*.grib2" );
    filters << QStringLiteral( "*.grb2" );
    files = dir.entryList( filters, QDir::Files );
  }
  else if ( fileInfo.isFile() &&
            ( fileInfo.suffix() == QStringLiteral( "grib2" ) ||
              fileInfo.suffix() == QStringLiteral( "grb2" ) ) )
  {
    files << path;
    dir = fileInfo.dir();
  }

  return files;
}

void ReosGribGriddedRainfallProvider::giveName( FileDetails &details )
{
  QString name;
  auto baseName = []( const QString & string )->QString
  {
    QFileInfo fileInfo( string );
    return fileInfo.baseName();
  };
  const QStringList &files = details.files;

  if ( files.count() == 1 )
    name = baseName( files.first() );
  else
  {
    QString fileName1 = baseName( files.at( 0 ) );
    QString commonPart;
    for ( int i = 0; i < files.count() - 1; ++i )
    {
      commonPart.clear();
      const QString fileName2 = baseName( files.at( i + 1 ) );
      int cp2 = 0;
      QChar c2;
      bool common = false;
      for ( int cp1 = 0; cp1 < fileName1.count(); cp1++ )
      {
        const QChar c1 = fileName1.at( cp1 );
        c2 = fileName2.at( cp2 );

        if ( common && c1 != c2 )
          break;

        while ( c1 != c2 && cp2 < fileName2.count() - 1 )
        {
          cp2++;
          c2 = fileName2.at( cp2 );
        }

        common = c1 == c2;
        if ( !common )
        {
          if ( c2 == fileName2.count() )
          {
            if ( c1 == fileName1.count() )
              break;
            else
              cp2 = 0;
          }
        }
        else
        {
          commonPart.append( c1 );
          cp2++;
        }
        if ( cp2 == fileName2.count() )
          break;
      }
      if ( !commonPart.isEmpty() )
        fileName1 = commonPart;
    }

    if ( ! fileName1.isEmpty() )
      name = commonPart;
  }

  details.deducedName = name;
}

ReosGriddedRainfallProvider *ReosGribProviderFactory::createProvider( const QString &dataType ) const
{
  if ( dataType == ReosGribGriddedRainfallProvider::dataType() )
    return new ReosGribGriddedRainfallProvider;

  return nullptr;
}

QString ReosGribProviderFactory::key() const
{
  return GRIB_KEY;
}

bool ReosGribProviderFactory::supportType( const QString &dataType ) const
{
  return dataType.contains( ReosGriddedRainfall::staticType() );
}

QVariantMap ReosGribProviderFactory::uriParameters( const QString &dataType ) const
{
  QVariantMap ret;

  if ( supportType( dataType ) )
  {
    ret.insert( QStringLiteral( "file-or-dir-path" ), QObject::tr( "File or directory where are stored the data" ) );
    ret.insert( QStringLiteral( "variable" ), QObject::tr( "variable that store the pricipitation values" ) );
    ret.insert( QStringLiteral( "value-type" ), QObject::tr( "Type of the values: Intensity(0), Height for the time step (1) or Cummulative heigth (2) " ) );
  }

  return ret;
}

QString ReosGribProviderFactory::buildUri( const QString &dataType, const QVariantMap &parameters, bool &ok ) const
{
  if ( supportType( dataType ) &&
       parameters.contains( QStringLiteral( "file-or-dir-path" ) ) &&
       parameters.contains( QStringLiteral( "variable" ) ) &&
       parameters.contains( QStringLiteral( "value-type" ) ) )
  {
    const QString path = parameters.value( QStringLiteral( "file-or-dir-path" ) ).toString();
    const QString variable = parameters.value( QStringLiteral( "variable" ) ).toString();
    int typeInt = parameters.value( QStringLiteral( "value-type" ) ).toInt();

    if ( typeInt >= 0 && typeInt < 3 )
    {
      ReosGriddedRainfallProvider::ValueType type = static_cast<ReosGriddedRainfallProvider::ValueType>( typeInt );
      ok = true;
      return ReosGribGriddedRainfallProvider::uri( path, variable, type );
    }
  }
  ok = false;
  return QString();
}
