#include "reoshecrasproject.h"

#include <QTextStream>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTemporaryFile>

#include "reosdssutils.h"

ReosHecRasProject::ReosHecRasProject( const QString &fileName ):
  mFileName( fileName )
{
  parseProjectFile();
}

QString ReosHecRasProject::currentPlanId() const
{
  return mCurrentPlan;
}

QStringList ReosHecRasProject::planIds() const
{
  return mPlans.keys();
}

QString ReosHecRasProject::planTitle( const QString &id ) const
{
  return mPlans.value( id ).title();
}

QDir ReosHecRasProject::directory() const
{
  QFileInfo fileInfo( mFileName );
  return fileInfo.dir();
}

ReosHecRasPlan ReosHecRasProject::currentPlan() const
{
  return mPlans.value( mCurrentPlan );
}

ReosHecRasPlan ReosHecRasProject::plan( const QString &planId ) const
{
  return mPlans.value( planId );
}

int ReosHecRasProject::GeometriesCount() const
{
  return mGeometries.count();
}

QStringList ReosHecRasProject::geometryIds() const
{
  return mGeometries.keys();
}

ReosHecRasGeometry ReosHecRasProject::geometry( const QString &id ) const
{
  return mGeometries.value( id );
}

ReosHecRasGeometry ReosHecRasProject::currentGeometry() const
{
  const ReosHecRasPlan &currentPlan = mPlans.value( mCurrentPlan );
  return mGeometries.value( currentPlan.geometryFile() );
}

QStringList ReosHecRasProject::flowIds() const
{
  return mFlows.keys();
}

ReosHecRasFlow ReosHecRasProject::flow( const QString &id ) const
{
  return mFlows.value( id );
}

ReosHecRasFlow ReosHecRasProject::currentFlow() const
{
  const ReosHecRasPlan &currentPlan = mPlans.value( mCurrentPlan );
  return mFlows.value( currentPlan.flowFile() );
}

void ReosHecRasProject::parseProjectFile()
{
  QFile file( mFileName );
  if ( !file.open( QIODevice::ReadOnly ) )
    return;

  QFileInfo fileInfo( mFileName );
  QString projectName = fileInfo.baseName();
  QDir projectDir = fileInfo.dir();

  QTextStream stream( &file );

  while ( !stream.atEnd() )
  {
    const QString line = stream.readLine();
    if ( line.startsWith( QStringLiteral( "Geom File=" ) ) )
    {
      QString geomFile = line;
      geomFile.remove( QStringLiteral( "Geom File=" ) );
      mGeometries.insert( geomFile, ReosHecRasGeometry( projectDir.filePath( projectName + '.' + geomFile ) ) );
    }

    if ( line.startsWith( QStringLiteral( "Plan File=" ) ) )
    {
      QString planFile = line;
      planFile.remove( QStringLiteral( "Plan File=" ) );
      mPlans.insert( planFile, ReosHecRasPlan( projectDir.filePath( projectName + '.' + planFile ) ) );
    }

    if ( line.startsWith( QStringLiteral( "Unsteady File=" ) ) )
    {
      QString flowFile = line;
      flowFile.remove( QStringLiteral( "Unsteady File=" ) );
      mFlows.insert( flowFile, ReosHecRasFlow( projectDir.filePath( projectName + '.' + flowFile ) ) );
    }

    if ( line.startsWith( QStringLiteral( "Current Plan=" ) ) )
    {
      QString plan = line;
      plan.remove( QStringLiteral( "Current Plan=" ) );
      mCurrentPlan = plan.trimmed();
    }
  }
}

ReosHecRasGeometry::ReosHecRasGeometry( const QString &fileName )
  : mFileName( fileName )
{
  parseGeometryFile();
}

int ReosHecRasGeometry::area2dCount() const
{
  return mAreas2D.count();
}

QString ReosHecRasGeometry::area2dName( int i ) const
{
  return mAreas2D.at( i ).name;
}

ReosHecRasGeometry::FlowArea2D ReosHecRasGeometry::area2d( int i ) const
{
  return mAreas2D.at( i );
}

QList<ReosHecRasGeometry::BoundaryCondition> ReosHecRasGeometry::boundariesConditions( const QString &area2dName ) const
{
  return mBoundariesConditions.value( area2dName );
}

QList<ReosHecRasGeometry::BoundaryCondition> ReosHecRasGeometry::allBoundariesConditions() const
{
  QList<ReosHecRasGeometry::BoundaryCondition> ret;
  for ( const FlowArea2D &area : mAreas2D )
    ret.append( boundariesConditions( area.name ) );

  return ret;
}

void ReosHecRasGeometry::parseGeometryFile()
{
  QFile file( mFileName );
  if ( !file.open( QIODevice::ReadOnly ) )
    return;

  QTextStream stream( &file );

  while ( !stream.atEnd() )
  {
    const QString line = stream.readLine();
    if ( line.startsWith( QStringLiteral( "Geom Title=" ) ) )
    {
      mTitle = line;
      mTitle.remove( QStringLiteral( "Geom Title=" ) );
    }

    if ( line.startsWith( QStringLiteral( "Storage Area=" ) ) )
    {
      QString name = line;
      name.remove( QStringLiteral( "Storage Area=" ) );
      name = name.split( ',' ).at( 0 );
      parseStorageArea( stream, name.trimmed() );
    }
    if ( line.startsWith( QStringLiteral( "BC Line Name=" ) ) )
    {
      QString name = line;
      name.remove( QStringLiteral( "BC Line Name=" ) );
      parseBoundaryCondition( stream, name.trimmed() );
    }
  }
}

void ReosHecRasGeometry::parseStorageArea( QTextStream &stream, const QString storageName )
{
  QPolygonF surface;
  bool is2D = false;
  while ( !stream.atEnd() )
  {
    const QString line = stream.readLine();
    if ( line.startsWith( QStringLiteral( "Storage Area Surface Line=" ) ) )
    {
      QString countString = line;
      countString.remove( QStringLiteral( "Storage Area Surface Line=" ) );
      countString.remove( ' ' );
      int pointCount = countString.toInt() - 1;
      surface.resize( pointCount );
      for ( int i = 0; i < pointCount; ++i )
      {
        QString pointLine = stream.readLine();
        QString partx = pointLine;
        partx.truncate( 16 );
        partx.remove( ' ' );
        QString party = pointLine.mid( 16 );
        party.remove( ' ' );
        surface[i] = QPointF( partx.toDouble(), party.toDouble() );
      }
    }

    if ( line.startsWith( QStringLiteral( "Storage Area Is2D=" ) ) )
    {
      QString is2DString = line;
      is2DString.remove( QStringLiteral( "Storage Area Is2D=" ) );
      is2D = is2DString.toInt() != 0;
    }

    if ( line.isEmpty() )
      break;

  }

  if ( is2D )
  {
    mAreas2D.append( {storageName, surface} );
  }
}

void ReosHecRasGeometry::parseBoundaryCondition( QTextStream &stream, const QString &bcName )
{
  BoundaryCondition bc;
  bc.name = bcName;
  while ( !stream.atEnd() )
  {
    const QString line = stream.readLine();

    if ( line.startsWith( QStringLiteral( "BC Line Storage Area=" ) ) )
    {
      bc.area = line;
      bc.area.remove( "BC Line Storage Area=" );
      bc.area = bc.area.trimmed();
    }

    if ( line.startsWith( QStringLiteral( "BC Line Middle Position=" ) ) )
    {
      QString midPositionString = line;
      midPositionString.remove( QStringLiteral( "BC Line Middle Position=" ) );
      midPositionString.remove( ' ' );
      QStringList coordStr = midPositionString.split( ',' );
      if ( coordStr.count() != 2 )
        return;
      bc.middlePosition = QPointF( coordStr.at( 0 ).toDouble(), coordStr.at( 1 ).toDouble() );
    }

    if ( line.startsWith( QStringLiteral( "BC Line Text Position=" ) ) )
      break;
  }

  if ( mBoundariesConditions.contains( bc.area ) )
    mBoundariesConditions[bc.area].append( bc );
  else
  {
    QList<BoundaryCondition> bcs;
    bcs << bc;
    mBoundariesConditions.insert( bc.area, bcs );
  }

}

ReosHecRasPlan::ReosHecRasPlan( const QString &fileName )
  : mFileName( fileName )
{
  parsePlanFile();
}

QString ReosHecRasPlan::geometryFile() const
{
  return mGeometryFile;
}

QString ReosHecRasPlan::flowFile() const
{
  return mFlowFile;
}

const QString &ReosHecRasPlan::title() const
{
  return mTitle;
}

const QDateTime &ReosHecRasPlan::startTime() const
{
  return mStartTime;
}

const QDateTime &ReosHecRasPlan::endTime() const
{
  return mEndTime;
}

void ReosHecRasPlan::changeSimulationTimeInFile( const QDateTime &startTime, const QDateTime &endTime ) const
{
  QFile file( mFileName );
  if ( !file.open( QIODevice::ReadOnly ) )
    return;
  QTextStream stream( &file );

  QTemporaryFile tempFile;
  tempFile.open();
  QTextStream outputStream( &tempFile );

  while ( !stream.atEnd() )
  {
    QString inputLine = stream.readLine();
    if ( inputLine.startsWith( "Simulation Date=" ) )
    {
      outputStream << QStringLiteral( "Simulation Date=%1,%2,%3,%4" ).
                   arg( ReosHecRasProject::dateToHecRasDate( startTime.date() ),
                        startTime.time().toString( QStringLiteral( "HH:mm" ) ),
                        ReosHecRasProject::dateToHecRasDate( endTime.date() ),
                        endTime.time().toString( QStringLiteral( "HH:mm" ) ) )
                   << "\r\n";
    }
    else
      outputStream << inputLine << "\r\n";
  }

  file.close();
  QFile::remove( mFileName );
  tempFile.copy( mFileName );
}

void ReosHecRasPlan::parsePlanFile()
{
  QFile file( mFileName );
  if ( !file.open( QIODevice::ReadOnly ) )
    return;

  QTextStream stream( &file );

  while ( !stream.atEnd() )
  {
    const QString line = stream.readLine();
    if ( line.startsWith( QStringLiteral( "Plan Title=" ) ) )
    {
      mTitle = line;
      mTitle.remove( QStringLiteral( "Plan Title=" ) );
      mTitle = mTitle.trimmed();
    }

    if ( line.startsWith( QStringLiteral( "Geom File=" ) ) )
    {
      mGeometryFile = line;
      mGeometryFile.remove( QStringLiteral( "Geom File=" ) );
      mGeometryFile = mGeometryFile.trimmed();
    }

    if ( line.startsWith( QStringLiteral( "Flow File=" ) ) )
    {
      mFlowFile = line;
      mFlowFile.remove( QStringLiteral( "Flow File=" ) );
      mFlowFile = mFlowFile.trimmed();
    }

    if ( line.startsWith( QStringLiteral( "Simulation Date=" ) ) )
    {
      QString dateLine = line;
      dateLine.remove( QStringLiteral( "Simulation Date=" ) );
      QStringList splitDates = dateLine.trimmed().split( ',' );
      if ( splitDates.count() == 4 )
      {
        bool ok = true;
        QDate startDate = ReosHecRasProject::hecRasDateToDate( splitDates.at( 0 ) );
        ok &= startDate.isValid();
        QTime startTime = QTime::fromString( splitDates.at( 1 ) );
        ok &= startTime.isValid();
        QDate endDate = ReosHecRasProject::hecRasDateToDate( splitDates.at( 2 ) );
        ok &= endDate.isValid();
        QTime endTime = QTime::fromString( splitDates.at( 3 ) );
        ok &= endTime.isValid();

        if ( ok )
        {
          mStartTime = QDateTime( startDate, startTime, Qt::UTC );
          mEndTime = QDateTime( endDate, endTime, Qt::UTC );
        }

      }
    }
  }
}

QDate ReosHecRasProject::hecRasDateToDate( const QString &hecrasDate )
{
  if ( hecrasDate.size() != 9 )
    return QDate();

  QString dayStr = hecrasDate;
  dayStr.resize( 2 );
  bool ok = false;
  int d = dayStr.toInt( &ok );
  if ( !ok )
    return QDate();

  QString monthStr = hecrasDate.mid( 2, 3 ).toLower();
  int m = -1;
  if ( monthStr == QStringLiteral( "jan" ) )
    m = 1;
  if ( monthStr == QStringLiteral( "feb" ) )
    m = 2;
  if ( monthStr == QStringLiteral( "mar" ) )
    m = 3;
  if ( monthStr == QStringLiteral( "apr" ) )
    m = 4;
  if ( monthStr == QStringLiteral( "may" ) )
    m = 5;
  if ( monthStr == QStringLiteral( "jun" ) )
    m = 6;
  if ( monthStr == QStringLiteral( "jul" ) )
    m = 7;
  if ( monthStr == QStringLiteral( "aug" ) )
    m = 8;
  if ( monthStr == QStringLiteral( "sep" ) )
    m = 9;
  if ( monthStr == QStringLiteral( "oct" ) )
    m = 10;
  if ( monthStr == QStringLiteral( "nov" ) )
    m = 11;
  if ( monthStr == QStringLiteral( "dec" ) )
    m = 12;

  if ( m == -1 )
    return QDate();

  QString yearStr = hecrasDate.mid( 5 );
  int y = yearStr.toInt( &ok );
  if ( !ok )
    return QDate();

  return QDate( y, m, d );
}

QString ReosHecRasProject::dateToHecRasDate( const QDate &date )
{

  if ( date.isNull() || !date.isValid() )
    return QString();

  QString monthStr;
  switch ( date.month() )
  {
    case 1:
      monthStr = QStringLiteral( "jan" );
      break;
    case 2:
      monthStr = QStringLiteral( "feb" );
      break;
    case 3:
      monthStr = QStringLiteral( "mar" );
      break;
    case 4:
      monthStr = QStringLiteral( "apr" );
      break;
    case 5:
      monthStr = QStringLiteral( "may" );
      break;
    case 6:
      monthStr = QStringLiteral( "jun" ) ;
      break;
    case 7:
      monthStr = QStringLiteral( "jul" );
      break;
    case 8:
      monthStr = QStringLiteral( "aug" ) ;
      break;
    case 9:
      monthStr = QStringLiteral( "sep" );
      break;
    case 10:
      monthStr = QStringLiteral( "oct" ) ;
      break;
    case 11:
      monthStr = QStringLiteral( "nov" ) ;
      break;
    case 12:
      monthStr = QStringLiteral( "dec" ) ;
      break;
  }

  QString day = QString::number( date.day() );
  if ( day.count() == 1 )
    day.prepend( '0' );

  QString year = QString::number( date.year() );

  return day + monthStr + year;
}

ReosHecRasFlow::ReosHecRasFlow( const QString &fileName )
  : mFileName( fileName )
{
  parseFlowFile();
}

const QString &ReosHecRasFlow::title() const
{
  return mTitle;
}

int ReosHecRasFlow::boundariesCount() const
{
  return mBoundaries.count();
}

const ReosHecRasFlow::BoundaryFlow &ReosHecRasFlow::boundary( int index ) const
{
  return mBoundaries.at( index );
}

ReosHecRasFlow::BoundaryFlow ReosHecRasFlow::boundary( const QString &area, const QString &boundaryLine, bool &found ) const
{
  found = false;
  for ( const BoundaryFlow &bf : mBoundaries )
  {
    if ( bf.area == area && bf.boundaryConditionLine == boundaryLine )
    {
      found = true;
      return bf;
    }
  }
  return BoundaryFlow();
}

bool ReosHecRasFlow::applyBoudaryFlow( const QList<BoundaryFlow> &flows )
{
  QFile file( mFileName );
  if ( !file.open( QIODevice::ReadOnly ) )
    return false;
  QTextStream stream( &file );

  QTemporaryFile tempFile;
  tempFile.open();
  QTextStream outputStream( &tempFile );

  auto foundFlow = [&]( const QString & area, const QString & boundaryLine, bool & found ) -> const BoundaryFlow
  {
    found = false;
    for ( const BoundaryFlow &bf : flows )
    {
      if ( bf.area == area && bf.boundaryConditionLine == boundaryLine )
      {
        found = true;
        return bf;
      }
    }
    return BoundaryFlow();
  };

  BoundaryFlow currentFlow;
  bool isInBoundaryToTreat = false;
  bool isStageHyd = false;
  while ( !stream.atEnd() )
  {
    QString inputLine = stream.readLine();
    if ( inputLine.startsWith( QStringLiteral( "Boundary Location=" ) ) )
    {
      QString area;
      QString boundaryLine;
      isInBoundaryToTreat = false;
      if ( parseLocation( inputLine, area, boundaryLine ) )
      {
        bool found = false;
        currentFlow = foundFlow( area, boundaryLine, found );
        isInBoundaryToTreat = found && currentFlow.isDss;
      }
      outputStream << inputLine << "\r\n";

      if ( currentFlow.type == Type::FlowHydrograph )
      {
        outputStream << QStringLiteral( "Flow Hydrograph= 0 " ) << "\r\n";
        isStageHyd = false;
      }
      else if ( currentFlow.type == Type::StageHydrograph )
      {
        outputStream << QStringLiteral( "Stage Hydrograph= 0 " ) << "\r\n";
        outputStream << QStringLiteral( "DSS File=%1" ).arg( currentFlow.dssFile ) << "\r\n";
        outputStream << QStringLiteral( "DSS Path=%1" ).arg( currentFlow.dssPath ) << "\r\n";
        outputStream << QStringLiteral( "Use DSS=True" ) << "\r\n";
        isStageHyd = true;
      }

    }
    else if ( isInBoundaryToTreat &&
              ( inputLine.startsWith( QStringLiteral( "Flow Hydrograph=" ) ) ||
                inputLine.startsWith( QStringLiteral( "Stage Hydrograph=" ) ) ) )
    {
      int valueCount = 0;
      QStringList part = inputLine.split( '=' );
      if ( part.count() > 1 )
      {
        bool ok = false;
        valueCount = part.at( 1 ).trimmed().toInt( &ok );
        if ( !ok )
          valueCount = 0;
      }
      int rowCount = valueCount / 10 + 1;
      for ( int i = 0; i < rowCount; ++i )
        stream.readLine();
    }
    else if ( isInBoundaryToTreat && inputLine.startsWith( QStringLiteral( "DSS File=" ) ) )
      continue;
    else if ( isInBoundaryToTreat && !isStageHyd && inputLine.startsWith( QStringLiteral( "DSS Path=" ) ) )
    {
      outputStream << QStringLiteral( "DSS File=%1" ).arg( currentFlow.dssFile ) << "\r\n";
      outputStream << QStringLiteral( "DSS Path=%1" ).arg( currentFlow.dssPath ) << "\r\n";
      outputStream << QStringLiteral( "Use DSS=True" ) << "\r\n";
    }
    else if ( isInBoundaryToTreat && inputLine.startsWith( QStringLiteral( "Use DSS=" ) ) )
      continue;
    else
      outputStream << inputLine << "\r\n";
  }

  file.close();
  QFile::remove( mFileName );
  tempFile.copy( mFileName );

  return true;
}

void ReosHecRasFlow::parseFlowFile()
{
  QFile file( mFileName );
  if ( !file.open( QIODevice::ReadOnly ) )
    return;

  QTextStream stream( &file );

  QString lastReadenLine;

  while ( !stream.atEnd() )
  {
    QString line;
    if ( lastReadenLine.isEmpty() )
      line = stream.readLine();
    else
    {
      line = lastReadenLine;
      lastReadenLine.clear();
    }

    if ( line.startsWith( QStringLiteral( "Flow Title=" ) ) )
    {
      mTitle = line;
      mTitle.remove( QStringLiteral( "Flow Title=" ) );
      mTitle = mTitle.trimmed();
    }

    if ( line.startsWith( QStringLiteral( "Boundary Location=" ) ) )
    {
      lastReadenLine = parseBoundary( stream, line );
    }
  }
}

QString ReosHecRasFlow::parseBoundary( QTextStream &stream, const QString &firstLine )
{
  BoundaryFlow boundary;
  if ( !parseLocation( firstLine, boundary.area, boundary.boundaryConditionLine ) )
    return QString();

  QString line;
  while ( !stream.atEnd() )
  {
    line = stream.readLine();

    if ( line.startsWith( QStringLiteral( "Boundary Location=" ) ) ||
         line.startsWith( QStringLiteral( "Met Point Raster Parameters=" ) ) ) //supposed to be the line after just after the last boundary
      break;

    if ( line.startsWith( QStringLiteral( "Interval=" ) ) )
    {
      QString strInterval = line;
      strInterval.remove( QStringLiteral( "Interval=" ) );
      strInterval = strInterval.trimmed();
      boundary.interval = ReosDssUtils::dssIntervalToDuration( strInterval );
    }

    // hydrograph or waterlevel boundary condition
    if ( line.startsWith( QStringLiteral( "Flow Hydrograph=" ) ) )
    {
      boundary.type = Type::FlowHydrograph;
      boundary.values = parseValues( stream, line );
    }

    if ( line.startsWith( QStringLiteral( "Stage Hydrograph=" ) ) )
    {
      boundary.type = Type::StageHydrograph;
      boundary.values = parseValues( stream, line );
    }

    if ( line.startsWith( QStringLiteral( "DSS File=" ) ) )
    {
      boundary.dssFile = line;
      boundary.dssFile.remove( QStringLiteral( "DSS File=" ) );
      boundary.dssFile = boundary.dssFile.trimmed();
    }

    if ( line.startsWith( QStringLiteral( "DSS Path=" ) ) )
    {
      boundary.dssPath = line;
      boundary.dssPath.remove( QStringLiteral( "DSS Path=" ) );
      boundary.dssPath = boundary.dssPath.trimmed();
    }

    if ( line.startsWith( QStringLiteral( "Use DSS=" ) ) )
    {
      QString value = line.mid( 8 );
      boundary.isDss = value == QStringLiteral( "True" );
    }

    // normal depth boundary condition
    if ( line.startsWith( QStringLiteral( "Friction Slope=" ) ) && boundary.type != Type::StageHydrograph )
    {
      boundary.type = Type::NormalDepth;
    }
  }

  mBoundaries.append( boundary );

  return line;
}

QVector<double> ReosHecRasFlow::parseValues( QTextStream &stream, const QString &firstLine )
{
  QVector<double> ret;
  QStringList parts = firstLine.split( '=' );
  if ( parts.count() != 2 )
    return ret;

  QString strCount = parts.at( 1 ).trimmed();
  bool ok = false;
  int count = strCount.toInt( &ok );
  if ( !ok )
    return ret;

  ret.resize( count );
  int rowCount = 0;
  if ( count != 0 )
    rowCount = count / 10 + 1;
  int i = 0;
  for ( int r = 0; r < rowCount; ++r )
  {
    const QString line = stream.readLine();
    int colCount = std::min( count - i, 10 );
    for ( int c = 0; c < colCount; ++c )
    {
      const QString strVal = line.mid( c * 8, 8 );
      double val = strVal.toDouble( &ok );
      if ( !ok )
        return QVector<double>();
      ret[i] = val;
      i++;
    }
  }

  return ret;

}

bool ReosHecRasFlow::parseLocation( const QString &locationLine, QString &area, QString &boundaryLine ) const
{
  QString first = locationLine;
  first.remove( QStringLiteral( "Boundary Location=" ) );
  QStringList locationParts = first.split( ',' );

  if ( locationParts.count() != 9 )
    return false;

  area = locationParts.at( 5 ).trimmed();
  boundaryLine = locationParts.at( 7 ).trimmed();

  return true;
}

static QString intToString( int value, int stringSize )
{
  QString ret = QString::number( value );
  QString prefix( ' ', stringSize - ret.size() );
  ret.prepend( prefix );

  return ret;
}

static QString doubleToString( double value, int stringSize )
{
  QString ret = QString::number( value );
  QString prefix( ' ', stringSize - ret.size() );
  ret.prepend( prefix );

  return ret;
}