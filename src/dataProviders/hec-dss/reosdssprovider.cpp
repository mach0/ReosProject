/***************************************************************************
  reosdssprovider.cpp - ReosDssProvider

 ---------------------
 begin                : 17.10.2022
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
#include "reosdssprovider.h"

#include <QFileInfo>

#include "reostimeseries.h"
#include "reosgriddedrainitem.h"
#include "reosdssutils.h"

REOSEXTERN ReosDataProviderFactory *providerFactory()
{
  return new ReosDssProviderFactory();
}

ReosDssProviderBase::ReosDssProviderBase()
{

}

QString ReosDssProviderBase::staticKey()
{
  return ReosDssUtils::dssProviderKey();
}



ReosDssProviderBase::~ReosDssProviderBase() = default;

QString ReosDssProviderTimeSerieConstantTimeStep::key() const
{
  return ReosDssProviderBase::staticKey() + QStringLiteral( "::" ) + dataType();
}

void ReosDssProviderTimeSerieConstantTimeStep::load()
{
  if ( mFile )
    mFile->close();

  mTimeStep = ReosDuration();
  mReferenceTime = QDateTime();
  mValues.clear();
  mDirty = false;
  const QString uri = dataSource();
  const QString fileName = ReosDssUtils::dssFileFromUri( uri );
  mFile.reset( new ReosDssFile( fileName, false ) );

  if ( !mFile->isValid() )
  {
    mFile.reset();
    return;
  }

  ReosDssPath path = ReosDssUtils::dssPathFromUri( uri );
  ReosDuration intervalInUri = path.timeIntervalDuration();
  bool hasInterval = intervalInUri != ReosDuration();

  QList<ReosDssPath> pathes = mFile->searchRecordsPath( path, hasInterval );

  if ( pathes.isEmpty() )
    return;

  if ( hasInterval )
    mTimeStep = ReosDssUtils::timeStepFromUri( uri );
  else
  {
    mTimeStep = pathes.first().timeIntervalDuration();
    for ( const ReosDssPath &pth : pathes )
    {
      if ( pth.timeIntervalDuration() != mTimeStep )
        return;
    }
  }

  path.setTimeInterval( mTimeStep );
  mFile->getSeries( path, mValues, mTimeStep, mReferenceTime );
}

QStringList ReosDssProviderTimeSerieConstantTimeStep::fileSuffixes() const
{
  QStringList ret;
  ret << QStringLiteral( "dss" );

  return ret;
}

QDateTime ReosDssProviderTimeSerieConstantTimeStep::referenceTime() const
{
  return mReferenceTime;
}

int ReosDssProviderTimeSerieConstantTimeStep::valueCount() const {return mValues.count();}

double ReosDssProviderTimeSerieConstantTimeStep::value( int i ) const
{
  if ( i < 0 || i > mValues.count() )
    return std::numeric_limits<double>::quiet_NaN();

  return mValues.at( i );
}

double ReosDssProviderTimeSerieConstantTimeStep::firstValue() const
{
  if ( mValues.count() > 0 )
    return mValues.first();

  return std::numeric_limits<double>::quiet_NaN();
}

double ReosDssProviderTimeSerieConstantTimeStep::lastValue() const
{
  if ( mValues.count() > 0 )
    return mValues.last();

  return std::numeric_limits<double>::quiet_NaN();
}

double *ReosDssProviderTimeSerieConstantTimeStep::data() {return mValues.data();}

const QVector<double> &ReosDssProviderTimeSerieConstantTimeStep::constData() const {return mValues;}

bool ReosDssProviderTimeSerieConstantTimeStep::createNewSerie( const ReosDssPath &path, ReosDssFile &dssFile, QString &error ) const
{
  return dssFile.createConstantIntervalSeries( path, error );
}

ReosDuration ReosDssProviderTimeSerieConstantTimeStep::timeStep() const
{
  return mTimeStep;
}

bool ReosDssProviderTimeSerieConstantTimeStep::isTimeStepCompatible( const ReosDuration &timeStep ) const
{
  return ReosDssUtils::closestValidInterval( timeStep ) == timeStep;
}

void ReosDssProviderTimeSerieConstantTimeStep::setReferenceTime( const QDateTime &referenceTime )
{
  mDirty = true;
  mReferenceTime = referenceTime;
}

void ReosDssProviderTimeSerieConstantTimeStep::setTimeStep( const ReosDuration &timeStep )
{
  if ( !isTimeStepCompatible( timeStep ) )
    return;
  mDirty = true;
  mTimeStep = timeStep;
}

void ReosDssProviderTimeSerieConstantTimeStep::resize( int size )
{
  mDirty = true;
  mValues.resize( size );
}

void ReosDssProviderTimeSerieConstantTimeStep::appendValue( double value )
{
  mDirty = true;
  mValues.append( value );
}

void ReosDssProviderTimeSerieConstantTimeStep::prependValue( double value )
{
  mDirty = true;
  mValues.prepend( value );
}

void ReosDssProviderTimeSerieConstantTimeStep::insertValue( int pos, double value )
{
  mDirty = true;
  mValues.insert( pos, value );
}

void ReosDssProviderTimeSerieConstantTimeStep::setValue( int index, double value )
{
  mDirty = true;
  mValues.replace( index, value );
}

void ReosDssProviderTimeSerieConstantTimeStep::removeValues( int from, int count )
{
  mDirty = true;
  int maxCount = std::min( count, mValues.count() - from );
  QVector<double>::iterator itStart = mValues.begin() + from;
  QVector<double>::iterator itEnd = itStart + maxCount;
  mValues.erase( itStart, itEnd );
}

void ReosDssProviderTimeSerieConstantTimeStep::clear()
{
  mDirty = true;
  mValues.clear();
}

bool ReosDssProviderTimeSerieConstantTimeStep::persistData( QString &error )
{
  const QString &uri = dataSource();
  if ( !mFile || !mFile->isValid() )
  {
    error = QObject::tr( "DSS file not opened or not valid, Unable to write on file." );
    return false;
  }

  const ReosDssPath path = ReosDssUtils::dssPathFromUri( uri );

  bool res = mFile->writeConstantIntervalSeries( path, mReferenceTime, mTimeStep, mValues, error );

  //we have to change the path accordingly of the time step
  QString filePath = ReosDssUtils::dssFileFromUri( uri );
  ReosDssPath newPath = path;
  newPath.setTimeInterval( mTimeStep );
  setDataSource( ReosDssUtils::uri( filePath, newPath ), false );

  if ( res )
    load();

  return res;
}

QString ReosDssProviderTimeSerieConstantTimeStep::dataType()
{
  return ReosTimeSeriesConstantInterval::staticType();
}

ReosDataProvider *ReosDssProviderFactory::createProvider( const QString &dataType ) const
{
  if ( dataType.contains( ReosDssProviderTimeSerieConstantTimeStep::dataType() ) )
    return new ReosDssProviderTimeSerieConstantTimeStep();

  if ( dataType.contains( ReosDssProviderTimeSerieVariableTimeStep::dataType() ) )
    return new ReosDssProviderTimeSerieVariableTimeStep();

  if ( dataType.contains( ReosDssProviderGriddedRainfall::dataType() ) )
    return new ReosDssProviderGriddedRainfall();

  return nullptr;
}

bool ReosDssProviderFactory::createNewDataSource( const QString &uri, const QString &dataType, QString &error )
{
  const QString fileName = ReosDssUtils::dssFileFromUri( uri );
  const QFileInfo fileInfo( fileName );
  const ReosDssPath path = ReosDssUtils::dssPathFromUri( uri );

  if ( !path.isValid() )
  {
    error = QObject::tr( "The DSS path is invalid \"%1\"." );
    return false;
  }

  ReosDssFile dssFile( fileName, !fileInfo.exists() );

  if ( !dssFile.isValid() )
  {
    error = QObject::tr( "Unable to create or open file \"%1\"." ).arg( fileName );
    return false;
  }

  std::unique_ptr<ReosDssProviderBase> provider;

  if ( dataType.contains( ReosDssProviderTimeSerieConstantTimeStep::dataType() ) )
    provider.reset( new ReosDssProviderTimeSerieConstantTimeStep() );

  if ( provider )
    return provider->createNewSerie( path, dssFile, error );

  error = QObject::tr( "Data type no supported by DSS provider." );

  return false;
}

QString ReosDssProviderFactory::key() const
{
  return ReosDssProviderBase::staticKey();
}

bool ReosDssProviderFactory::hasCapabilities( const QString &dataType, ReosDataProvider::Capabilities capabilities ) const
{
  ReosDataProvider::Capabilities cap;
  if ( dataType.contains( ReosDssProviderTimeSerieConstantTimeStep::dataType() ) )
    cap = ReosDataProvider::File ;

  if ( dataType.contains( ReosDssProviderTimeSerieVariableTimeStep::dataType() ) )
    cap = ReosDataProvider::File ;

  if ( dataType.contains( ReosDssProviderGriddedRainfall::dataType() ) )
    cap = ReosDataProvider::Capabilities( ReosDataProvider::File | ReosDataProvider::CanWrite | ReosDataProvider::Spatial );

  return ( cap & capabilities ) == capabilities;
}

bool ReosDssProviderFactory::supportType( const QString &dataType ) const
{
  return dataType.contains( ReosGriddedRainfall::staticType() ) ||
         dataType.contains( ReosTimeSeriesConstantInterval::staticType() ) ||
         dataType.contains( ReosTimeSeriesVariableTimeStep::staticType() );
}

QVariantMap ReosDssProviderFactory::uriParameters( const QString &dataType ) const
{
  QVariantMap ret;

  if ( supportType( dataType ) )
  {
    ret.insert( QStringLiteral( "file-path" ), QObject::tr( "DSS file where are stored the data" ) );
    ret.insert( QStringLiteral( "dss-path" ), QObject::tr( "DSS Path of the data : /group/location/parameter/start-date/time-interval/version/" ) );
  }

  return ret;
}

QString ReosDssProviderFactory::buildUri( const QString &dataType, const QVariantMap &parameters, bool &ok ) const
{
  if ( supportType( dataType ) &&
       parameters.contains( QStringLiteral( "file-path" ) ) &&
       parameters.contains( QStringLiteral( "dss-path" ) ) )
  {
    const QString filePath = parameters.value( QStringLiteral( "file-path" ) ).toString();
    const QString dssPathString = parameters.value( QStringLiteral( "dss-path" ) ).toString();
    const ReosDssPath dssPath( dssPathString );

    if ( dssPath.isValid() )
    {
      ok = true;
      return ReosDssUtils::uri( filePath, dssPath );
    }
  }
  ok = false;
  return QString();
}

QString ReosDssProviderTimeSerieVariableTimeStep::key() const
{
  return ReosDssProviderBase::staticKey() + QStringLiteral( "::" ) + dataType();
}

QStringList ReosDssProviderTimeSerieVariableTimeStep::fileSuffixes() const
{
  QStringList ret;
  ret << QStringLiteral( "dss" );

  return ret;
}

void ReosDssProviderTimeSerieVariableTimeStep::load()
{
  if ( mFile )
    mFile->close();

  mReferenceTime = QDateTime();
  mValues.clear();
  //mDirty = false;
  const QString uri = dataSource();
  const QString fileName = ReosDssUtils::dssFileFromUri( uri );
  mFile.reset( new ReosDssFile( fileName, false ) );

  if ( !mFile->isValid() )
  {
    mFile.reset();
    return;
  }

  ReosDssPath path = ReosDssUtils::dssPathFromUri( uri );
  ReosDuration intervalInUri = path.timeIntervalDuration();
  bool hasInterval = intervalInUri != ReosDuration();

  QList<ReosDssPath> pathes = mFile->searchRecordsPath( path, hasInterval );

  if ( pathes.isEmpty() )
    return;

  ReosDuration timeStep = pathes.first().timeIntervalDuration();
  for ( const ReosDssPath &pth : pathes )
  {
    if ( pth.timeIntervalDuration() != timeStep )
      return;
  }
  path.setTimeInterval( pathes.first().timeInterval() );

  ReosTimeWindow tw = ReosDssUtils::timeWindowFromUri( uri );

  mFile->getSeries( path, tw, mValues, timeStep, mReferenceTime );

  mTimeValues.resize( mValues.size() );
  for ( int i = 0; i < mTimeValues.count(); ++i )
    mTimeValues[i] = timeStep * i;
}

QDateTime ReosDssProviderTimeSerieVariableTimeStep::referenceTime() const
{
  return mReferenceTime;
}

int ReosDssProviderTimeSerieVariableTimeStep::valueCount() const
{
  return mValues.count();
}

double ReosDssProviderTimeSerieVariableTimeStep::value( int i ) const
{
  if ( i < 0 || i > mValues.count() )
    return std::numeric_limits<double>::quiet_NaN();
  return mValues.at( i );
}

double ReosDssProviderTimeSerieVariableTimeStep::firstValue() const
{
  if ( mValues.isEmpty() )
    return std::numeric_limits<double>::quiet_NaN();
  return mValues.first();
}

double ReosDssProviderTimeSerieVariableTimeStep::lastValue() const
{
  if ( mValues.isEmpty() )
    return std::numeric_limits<double>::quiet_NaN();
  return mValues.last();
}

double *ReosDssProviderTimeSerieVariableTimeStep::data()
{
  return mValues.data();
}

const QVector<double> &ReosDssProviderTimeSerieVariableTimeStep::constData() const
{
  return mValues;
}

ReosDuration ReosDssProviderTimeSerieVariableTimeStep::relativeTimeAt( int i ) const
{
  if ( i < 0 || i > mValues.count() )
    return ReosDuration();
  return mTimeValues.at( i );
}

ReosDuration ReosDssProviderTimeSerieVariableTimeStep::lastRelativeTime() const
{
  if ( mValues.isEmpty() )
    return ReosDuration();
  return mTimeValues.last();
}

const QVector<ReosDuration> &ReosDssProviderTimeSerieVariableTimeStep::constTimeData() const
{
  return mTimeValues;
}

QString ReosDssProviderTimeSerieVariableTimeStep::dataType()
{
  return ReosTimeSeriesVariableTimeStep::staticType();
}

void ReosDssProviderGriddedRainfall::load()
{
  mFilePath = ReosDssUtils::dssFileFromUri( dataSource() );
  mPath = ReosDssUtils::dssPathFromUri( dataSource() );

  mFile.reset( new ReosDssFile( mFilePath ) );

  mIsValid = mFile->isValid();
  if ( !mIsValid )
    return;

  QList<ReosDssPath> recordPathes = mFile->searchRecordsPath( mPath, false );

  mIsValid = !recordPathes.isEmpty();
  if ( !mIsValid )
    return;

  for ( const ReosDssPath &recordPath : recordPathes )
  {
    DssGrid grid;
    grid.path = recordPath;
    grid.startTime = dssStrToDateTime( recordPath.startDate() );
    grid.endTime = dssStrToDateTime( recordPath.timeInterval() );
    mGrids.append( grid );
  }
  mExtent = mFile->gridExtent( recordPathes.at( 0 ) );

  std::sort( mGrids.begin(), mGrids.end(), []( const DssGrid & f1, const DssGrid & f2 )->bool
  {
    return f1.startTime < f2.startTime;
  } );

  emit dataReset();
  emit loadingFinished();
}

QString ReosDssProviderGriddedRainfall::key() const
{
  return ReosDssProviderBase::staticKey() + QStringLiteral( "::" ) + dataType();
}

QStringList ReosDssProviderGriddedRainfall::fileSuffixes() const
{
  QStringList ret;
  ret << QStringLiteral( "dss" );
  return ret;
}

bool ReosDssProviderGriddedRainfall::canReadUri( const QString &uri ) const
{
  ReosDssFile file( uri );

  if ( !file.isValid() )
    return false;

  QList<ReosDssPath> pathes = file.allPathes();

  return !pathes.isEmpty();
}

QString ReosDssProviderGriddedRainfall::htmlDescription() const
{
  QString htmlText = QStringLiteral( "<html>\n<body>\n" );
  htmlText += QLatin1String( "<table class=\"list-view\">\n" );


  htmlText += QStringLiteral( "<h2>" ) + tr( "Gridded Precipitation" ) + QStringLiteral( "</h2>\n<hr>\n" );

  htmlText += QStringLiteral( "<tr><td class=\"highlight\">" )
              + QStringLiteral( "<b>%1</b>" ).arg( tr( "Format" ) ) + QStringLiteral( "</td><td>" )
              + QStringLiteral( "HEC DSS" ) + QStringLiteral( "</td></tr>\n" );

  htmlText += QStringLiteral( "<tr><td class=\"highlight\">" )
              + QStringLiteral( "<b>%1</b>" ).arg( tr( "Source" ) ) + QStringLiteral( "</td><td>" )
              + mFilePath + QStringLiteral( "</td></tr>\n" );

  htmlText += QStringLiteral( "<tr><td class=\"highlight\">" )
              + QStringLiteral( "<b>%1</b>" ).arg( tr( "DSS Path" ) ) + QStringLiteral( "</td><td>" )
              + mPath.string() + QStringLiteral( "</td></tr>\n" );

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

ReosGriddedRainfallProvider::FileDetails ReosDssProviderGriddedRainfall::details( const QString &uri, ReosModule::Message &message ) const
{
  QString fileName = ReosDssUtils::dssFileFromUri( uri );
  QList<ReosDssPath> pathes = griddedRainfallPathes( fileName, message );

  FileDetails ret;
  for ( const ReosDssPath &path : pathes )
    ret.availableVariables.append( path.string() );

  QFileInfo fileInfo( fileName );
  ret.deducedName = fileInfo.baseName();

  return ret;
}

ReosGriddedRainfallProvider *ReosDssProviderGriddedRainfall::clone() const
{
  std::unique_ptr<ReosDssProviderGriddedRainfall> other = std::make_unique<ReosDssProviderGriddedRainfall>();
  other->setDataSource( dataSource() );

  return other.release();
}

bool ReosDssProviderGriddedRainfall::isValid() const
{
  return mIsValid;
}

int ReosDssProviderGriddedRainfall::count() const
{
  return mGrids.count();
}

QDateTime ReosDssProviderGriddedRainfall::startTime( int index ) const
{
  if ( index < 0 || index > mGrids.count() )
    return QDateTime();

  return mGrids.at( index ).startTime;
}

QDateTime ReosDssProviderGriddedRainfall::endTime( int index ) const
{
  if ( index < 0 || index > mGrids.count() )
    return QDateTime();

  return mGrids.at( index ).endTime;
}

const QVector<double> ReosDssProviderGriddedRainfall::data( int index ) const
{
  if ( index < 0 || index > mGrids.count() )
    return QVector<double>();

  if ( mFile && mFile->isValid() )
  {
    int xCount = 0;
    int yCount = 0;
    QVector<double> vals = mFile->gridValues( mGrids.at( index ).path, xCount, yCount );
    if ( xCount == mExtent.xCellCount() && yCount == mExtent.yCellCount() )
      return vals;
  }

  return QVector<double>();
}

ReosRasterExtent ReosDssProviderGriddedRainfall::extent() const
{
  return mExtent;
}

ReosEncodedElement ReosDssProviderGriddedRainfall::encode( const ReosEncodeContext &context ) const
{
  ReosEncodedElement element( QStringLiteral( "dss-gridded-precipitation" ) );

  QString uriToEncode = dataSource();

  QString sourcePath = ReosDssUtils::dssFileFromUri( uriToEncode );
  sourcePath = context.pathToEncode( sourcePath );
  uriToEncode = ReosDssUtils::uri( sourcePath, ReosDssUtils::dssPathFromUri( uriToEncode ) );
  element.addData( QStringLiteral( "data-source" ), uriToEncode );

  return element;
}

void ReosDssProviderGriddedRainfall::decode( const ReosEncodedElement &element, const ReosEncodeContext &context )
{
  if ( element.description() != QStringLiteral( "dss-gridded-precipitation" ) )
    return;

  QString source;
  if ( element.getData( QStringLiteral( "data-source" ), source ) )
  {
    QString sourcePath = ReosDssUtils::dssFileFromUri( source );
    sourcePath = context.resolvePath( sourcePath );
    source = ReosDssUtils::uri( sourcePath, ReosDssUtils::dssPathFromUri( source ) );
    setDataSource( source );
  }
}

QString ReosDssProviderGriddedRainfall::dataType()
{
  return ReosGriddedRainfall::staticType();
}

QList<ReosDssPath> ReosDssProviderGriddedRainfall::griddedRainfallPathes( const QString &filePath, ReosModule::Message &message ) const
{
  QList<ReosDssPath> ret;
  ReosDssFile file( filePath );
  if ( !file.isValid() )
  {
    message.type = ReosModule::Error;
    message.text = tr( "Unable to open the dss file \"%1\"." ).arg( filePath );
    return ret;
  }

  const QList<ReosDssPath> allPathes = file.allPathes();

  for ( const ReosDssPath &path : allPathes )
  {
    ReosDssFile::RecordInfo info = file.recordInformation( path );

    if ( info.recordType == 430 )
    {
      bool isPresent = false;
      for ( const ReosDssPath &pp : std::as_const( ret ) )
      {
        if ( pp.isEquivalent( path ) )
        {
          isPresent = true;
          break;
        }
      }
      if ( !isPresent )
      {
        ReosDssPath  effPath = path;
        effPath.setStartDate( QString() );
        effPath.setTimeInterval( QString() );
        ret.append( effPath );
      }
    }
  }

  if ( ret.isEmpty() )
  {
    message.type = ReosModule::Error;
    message.text = tr( "Unable to find grid data in the dss file \"%1\"." ).arg( filePath );
  }
  else
    message.type = ReosModule::Simple;

  return ret;
}

QString ReosDssProviderGriddedRainfall::staticKey()
{
  return ReosDssProviderBase::staticKey() + QStringLiteral( "::" ) + ReosGriddedRainfall::staticType();
}

bool ReosDssProviderGriddedRainfall::getDirectMinMax( double &min, double &max ) const
{
  if ( mHasMinMaxCalculated )
  {
    min = mMin;
    max = mMax;
  }
  return mHasMinMaxCalculated;
}

void ReosDssProviderGriddedRainfall::calculateMinMax( double &min, double &max ) const
{
  if ( !mFile->isValid() )
    return;
  mMin = std::numeric_limits<double>::max();
  mMax = -std::numeric_limits<double>::max();
  for ( const  DssGrid &gridInfo : mGrids )
  {
    double m = 0;
    double M = 0;
    if ( !mFile->gridMinMax( gridInfo.path, m, M ) )
      continue;
    if ( mMin > m )
      mMin = m;
    if ( mMax < M )
      mMax = M;
  }
  min = mMin;
  max = mMax;
  mHasMinMaxCalculated = true;
}

bool ReosDssProviderGriddedRainfall::hasData( const QString &uri, const ReosTimeWindow &timeWindow ) const
{
  const QString filePath = ReosDssUtils::dssFileFromUri( uri );
  const ReosDssPath path = ReosDssUtils::dssPathFromUri( uri );

  ReosDssFile file( filePath );
  if ( !file.isValid() )
    return false;

  const QList<ReosDssPath> recordPathes = file.searchRecordsPath( path, false );

  if ( recordPathes.isEmpty() )
    return false;

  if ( !timeWindow.isValid() )
    return true;

  QList<ReosTimeWindow> timeWindows;

  for ( const ReosDssPath &recordPath : recordPathes )
  {
    ReosTimeWindow tw;
    tw.setStart( dssStrToDateTime( recordPath.startDate() ) );
    tw.setStart( dssStrToDateTime( recordPath.startDate() ) );
    tw.setEnd( dssStrToDateTime( recordPath.timeInterval() ) );
    timeWindows.append( tw );
  }

  if ( timeWindows.isEmpty() )
    return false;

  std::sort( timeWindows.begin(), timeWindows.end(), []( const ReosTimeWindow & tw1, const ReosTimeWindow & tw2 )
  {
    return tw1.start() < tw2.start();
  } );

  int i = 1;
  while ( i < timeWindows.count() )
  {
    if ( timeWindows.at( i - 1 ).end() == timeWindows.at( i ).start() )
    {
      ReosTimeWindow &tw1 = timeWindows[i - 1 ];
      tw1.setEnd( timeWindows.at( i ).end() );
      timeWindows.removeAt( i );
    }
    else
    {
      ++i;
    }
  }

  for ( const ReosTimeWindow &tw : std::as_const( timeWindows ) )
    if ( tw.intersect( timeWindow ) )
      return true;

  return false;
}

bool ReosDssProviderGriddedRainfall::write( ReosGriddedRainfall *rainfall, const QString &uri, const ReosRasterExtent &destination, const ReosTimeWindow &timeWindow ) const
{
  QString fileName = ReosDssUtils::dssFileFromUri( uri );
  ReosDssPath path = ReosDssUtils::dssPathFromUri( uri );
  ReosDssFile file( fileName, true );
  if ( !file.isValid() )
    return false;

  double resolution = std::min( destination.yCellSize(), destination.xCellSize() );
  return file.writeGriddedData( rainfall, path, destination, resolution, timeWindow );
}

QDateTime ReosDssProviderGriddedRainfall::dssStrToDateTime( const QString &str )
{
  const QStringList parts = str.split( ':' );
  if ( parts.count() != 2 )
    return QDateTime();
  QDate date = ReosDssUtils::dssDateToDate( parts.at( 0 ) );
  QTime time = ReosDssUtils::dssTimeToTime( parts.at( 1 ) );
  return QDateTime( date, time, Qt::UTC );
}
