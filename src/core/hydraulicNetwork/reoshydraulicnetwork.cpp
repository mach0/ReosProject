/***************************************************************************
  reoshydraulicnetwork.cpp - ReosHydraulicNetwork

 ---------------------
 begin                : 20.5.2021
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
#include "reoshydraulicnetwork.h"
#include "reoshydraulicnode.h"
#include "reoshydrauliclink.h"
#include "reoshydraulicstructure2d.h"
#include "reoshydrographrouting.h"
#include "reoshydraulicstructureboundarycondition.h"
#include "reoshydraulicscheme.h"
#include "reosgisengine.h"
#include <QUuid>


QString ReosHydraulicNetworkElement::hydraulicStructure2DType()
{
  return ReosHydraulicStructure2D::staticType();
}

QString ReosHydraulicNetworkElement::hydrographJunction()
{
  return ReosHydrographJunction::staticType();
}


ReosHydraulicNetworkElement::ReosHydraulicNetworkElement( ReosHydraulicNetwork *parent ):
  ReosDataObject( parent )
  , mNetwork( parent )
  , mNameParameter( new ReosParameterString( tr( "Name" ), false, this ) )
{
  mConstantTimeStepInTable = new ReosParameterDuration( tr( "Constant time step" ) );
  mConstantTimeStepInTable->setValue( ReosDuration( 5, ReosDuration::minute ) );
  mUseConstantTimeStepInTable = new ReosParameterBoolean( tr( "Display constant time step" ) );
  init();
}

ReosHydraulicNetworkElement::ReosHydraulicNetworkElement( const ReosEncodedElement &encodedElement, ReosHydraulicNetwork *parent )
  : ReosDataObject( parent )
  , mNetwork( parent )
  , mNameParameter( ReosParameterString::decode( encodedElement.getEncodedData( QStringLiteral( "name-parameter" ) ), false, tr( "Name" ), this ) )
{
  ReosDataObject::decode( encodedElement );

  mConstantTimeStepInTable = ReosParameterDuration::decode( encodedElement.getEncodedData( QStringLiteral( "constant-time-step-in-table" ) ),
                             false,
                             tr( "Constant time step" ),
                             this );

  if ( !mConstantTimeStepInTable->isValid() )
    mConstantTimeStepInTable->setValue( ReosDuration( 5, ReosDuration::minute ) );

  mUseConstantTimeStepInTable = ReosParameterBoolean::decode( encodedElement.getEncodedData( QStringLiteral( "use-constant-time-step-in-table" ) ),
                                false,
                                tr( "Display constant time step" ),
                                this );
  if ( !mUseConstantTimeStepInTable->isValid() )
    mUseConstantTimeStepInTable->setValue( false );

  init();
}

void ReosHydraulicNetworkElement::init()
{
  connect( mConstantTimeStepInTable, &ReosParameter::valueChanged, this, &ReosHydraulicNetworkElement::dirtied );
  connect( mUseConstantTimeStepInTable, &ReosParameter::valueChanged, this, &ReosHydraulicNetworkElement::dirtied );
  connect( mNameParameter, &ReosParameter::valueChanged, this, &ReosHydraulicNetworkElement::dirtied );
}


ReosHydraulicNetworkElement::~ReosHydraulicNetworkElement()
{
}

void ReosHydraulicNetworkElement::destroy()
{
  if ( !mNetwork.isNull() )
    mNetwork->mElements.remove( id() );
  deleteLater();
}

void ReosHydraulicNetworkElement::positionChanged()
{
  if ( mNetwork )
    mNetwork->elemPositionChangedPrivate( this );
}

ReosParameterString *ReosHydraulicNetworkElement::elementNameParameter() const
{
  return mNameParameter;
}

QString ReosHydraulicNetworkElement::elementName() const {return mNameParameter->value();}

ReosParameterDuration *ReosHydraulicNetworkElement::constantTimeStepInTable() const
{
  return mConstantTimeStepInTable;
}

ReosParameterBoolean *ReosHydraulicNetworkElement::useConstantTimeStepInTable() const
{
  return mUseConstantTimeStepInTable;
}

ReosEncodedElement ReosHydraulicNetworkElement::encode( const ReosHydraulicNetworkContext &context ) const
{
  ReosEncodedElement element( type() );
  element.addEncodedData( QStringLiteral( "name-parameter" ), mNameParameter->encode() );
  element.addEncodedData( QStringLiteral( "constant-time-step-in-table" ), mConstantTimeStepInTable->encode() );
  element.addEncodedData( QStringLiteral( "use-constant-time-step-in-table" ), mUseConstantTimeStepInTable->encode() );

  encodeData( element, context );

  ReosDataObject::encode( element );

  return element;
}

void ReosHydraulicNetworkElement::notify( const ReosModule::Message &messageObject )
{
  mLastMessage = messageObject;
  if ( !messageObject.text.isEmpty() )
  {
    ReosModule::Message sendedMessage = messageObject;
    sendedMessage.prefixMessage( tr( "Routing %1: " ).arg( elementNameParameter()->value() ) );
    if ( mNetwork )
      mNetwork->message( sendedMessage );
  }
}

ReosModule::Message ReosHydraulicNetworkElement::lastMessage() const
{
  return mLastMessage;
}

void ReosHydraulicNetworkElement::saveConfiguration( ReosHydraulicScheme * ) const {}

void ReosHydraulicNetworkElement::restoreConfiguration( ReosHydraulicScheme * ) {}

QFileInfoList ReosHydraulicNetworkElement::cleanScheme( ReosHydraulicScheme *scheme )
{
  return QFileInfoList();
}

ReosDuration ReosHydraulicNetworkElement::currentElementTimeStep() const
{
  return ReosDuration( qint64( 0 ) );
}

ReosTimeWindow ReosHydraulicNetworkElement::timeWindow() const
{
  return ReosTimeWindow();
}

ReosDuration ReosHydraulicNetworkElement::mapTimeStep() const
{
  return ReosDuration();
}

ReosHydraulicNetwork *ReosHydraulicNetworkElement::network() const
{
  return mNetwork;
}

ReosHydraulicNetworkElementCompatibilty ReosHydraulicNetworkElement::checkCompatiblity( ReosHydraulicScheme * ) const
{return ReosHydraulicNetworkElementCompatibilty();}

void ReosHydraulicNetworkElement::calculationUpdated()
{
  setActualized();
  emit calculationIsUpdated( id(), QPrivateSignal() );
}

ReosMapExtent ReosHydraulicNetworkElement::extent() const
{
  return ReosMapExtent();
}

ReosHydraulicNetwork::ReosHydraulicNetwork( ReosModule *parent, ReosGisEngine *gisEngine, ReosWatershedModule *watershedModule )
  : ReosModule( staticName(), parent )
  , mGisEngine( gisEngine )
  , mWatershedModule( watershedModule )
  , mHydraulicSchemeCollection( new ReosHydraulicSchemeCollection( this ) )
{
  ReosHydrographRoutingMethodFactories::instantiate( this );
  ReosGmshEngine::instantiate( this );

  mElementFactories.emplace( ReosHydrographNodeWatershed::staticType(), new ReosHydrographNodeWatershedFactory );
  mElementFactories.emplace( ReosHydrographJunction::staticType(), new ReosHydrographJunctionFactory );

  mElementFactories.emplace( ReosHydrographRoutingLink::staticType(), new ReosHydrographRoutingLinkFactory );

  mElementFactories.emplace( ReosHydraulicStructure2D::staticType(), new ReosHydraulicStructure2dFactory );
  mElementFactories.emplace( ReosHydraulicStructureBoundaryCondition::staticType(), new ReosHydraulicStructureBoundaryConditionFactory );

  addNewScheme( tr( "Scheme 1" ) );
  mCurrentSchemeIndex = 0;

  connect( mHydraulicSchemeCollection, &ReosHydraulicSchemeCollection::dirtied, this, &ReosModule::dirtied );
}

QFileInfoList ReosHydraulicNetwork::uselessFiles( bool clean ) const
{
  QFileInfoList ret = ReosModule::uselessFiles( clean );
  ret.append( mUselessFile );
  if ( clean )
    mUselessFile.clear();
  return ret;
}

ReosHydraulicNetworkElement *ReosHydraulicNetwork::getElement( const QString &elemId ) const
{
  if ( mElements.contains( elemId ) )
    return mElements.value( elemId );
  else
    return nullptr;
}

int ReosHydraulicNetwork::elementsCount() const
{
  return mElements.count();
}

ReosHydraulicNetworkElement *ReosHydraulicNetwork::addElement( ReosHydraulicNetworkElement *elem, bool select )
{
  mElements.insert( elem->id(), elem );
  if ( !elem->elementNameParameter()->isValid() )
  {
    int index = mElementIndexesCounter.value( elem->type(), 0 ) + 1;
    mElementIndexesCounter[ elem->type()] = index;
    elem->elementNameParameter()->setValue( ( elem->defaultDisplayName() + QStringLiteral( " %1" ) ).arg( index ) );
  }
  emit elementAdded( elem, select );

  connect( elem, &ReosHydraulicNetworkElement::dirtied, this, &ReosModule::dirtied );
  connect( elem, &ReosHydraulicNetworkElement::timeStepChanged, this, &ReosHydraulicNetwork::timeStepChanged );
  if ( elem->type().contains( ReosHydraulicStructure2D::staticType() ) )
    connect( elem, &ReosHydraulicNetworkElement::timeWindowChanged, this, &ReosHydraulicNetwork::onMapTimeWindowChanged );
  emit timeStepChanged();

  if ( elem->type().contains( ReosHydraulicStructure2D::staticType() ) )
    onMapTimeWindowChanged();

  return elem;
}

void ReosHydraulicNetwork::removeElement( ReosHydraulicNetworkElement *elem )
{
  if ( !elem )
    return;
  emit elementWillBeRemoved( elem );

  ReosHydraulicNode *node = qobject_cast<ReosHydraulicNode *>( elem );
  if ( node )
  {
    const QList<ReosHydraulicLink *> links = node->links();
    for ( ReosHydraulicLink *link :  links )
      removeElement( link );
  }

  ReosHydraulicStructure2D *structure = qobject_cast<ReosHydraulicStructure2D *>( elem );
  if ( structure )
  {
    mUselessFile.append( QFileInfo( structure->structureDirectory().path() ) );
    const QList<ReosHydraulicStructureBoundaryCondition *> bcs = structure->boundaryConditions();
    for ( ReosHydraulicStructureBoundaryCondition *bc : bcs )
      removeElement( bc );
  }

  elem->destroy();
  emit elementRemoved();
  emit timeStepChanged();
  emit dirtied();
}

void ReosHydraulicNetwork::decode( const ReosEncodedElement &element, const QString &projectPath, const QString &projectFileName )
{
  clear();
  if ( element.description() != QStringLiteral( "hydraulic-network" ) )
    return;

  mProjectName = projectFileName;
  mProjectPath = projectPath;

  element.getData( QStringLiteral( "elements-counter" ), mElementIndexesCounter );

  QList<ReosEncodedElement> encodedElements = element.getListEncodedData( QStringLiteral( "hydraulic-element" ) );

  //here order of adding element is important (nodes before structures before links, links need taht nodes exists before)

  for ( const ReosEncodedElement &encodedElement : encodedElements )
  {
    if ( encodedElement.description().contains( ReosHydraulicNode::staticType() ) )
    {
      addEncodedElement( encodedElement );
    }
  }

  for ( const ReosEncodedElement &encodedElement : encodedElements )
  {
    if ( encodedElement.description().contains( ReosHydraulicStructure2D::staticType() ) )
    {
      addEncodedElement( encodedElement );
    }
  }

  for ( const ReosEncodedElement &encodedElement : encodedElements )
  {
    if ( encodedElement.description().contains( ReosHydraulicLink::staticType() ) )
    {
      addEncodedElement( encodedElement );
    }
  }

  for ( ReosHydraulicNetworkElement *elem : std::as_const( mElements ) )
    elemPositionChangedPrivate( elem );

  mHydraulicSchemeCollection->decode( element.getEncodedData( QStringLiteral( "hydraulic-scheme-collection" ) ), context() );

  if ( mHydraulicSchemeCollection->schemeCount() == 0 )
  {
    addNewScheme( tr( "Scheme 1" ) );
  }

  int currentSchemeIndex = -1;
  element.getData( QStringLiteral( "current-scheme-index" ), currentSchemeIndex );
  if ( currentSchemeIndex < 0 || currentSchemeIndex >= mHydraulicSchemeCollection->schemeCount() )
    currentSchemeIndex = mHydraulicSchemeCollection->schemeCount() > 0 ? 0 : -1;
  setCurrentScheme( currentSchemeIndex );

  onMapTimeWindowChanged();
  emit loaded();
}

ReosEncodedElement ReosHydraulicNetwork::encode( const QString &projectPath, const QString &projectFileName ) const
{
  ReosEncodedElement element( QStringLiteral( "hydraulic-network" ) );

  QList<ReosEncodedElement> encodedElements;
  mProjectName = projectFileName;
  mProjectPath = projectPath;

  for ( ReosHydraulicNetworkElement *elem : mElements )
  {
    encodedElements.append( elem->encode( context() ) );
  }

  element.addListEncodedData( QStringLiteral( "hydraulic-element" ), encodedElements );
  element.addData( QStringLiteral( "elements-counter" ), mElementIndexesCounter );

  ReosHydraulicScheme *currentScheme = mHydraulicSchemeCollection->scheme( mCurrentSchemeIndex );
  if ( currentScheme )
  {
    for ( ReosHydraulicNetworkElement *elem : std::as_const( mElements ) )
      elem->saveConfiguration( currentScheme );
  }

  element.addEncodedData( QStringLiteral( "hydraulic-scheme-collection" ), mHydraulicSchemeCollection->encode() );
  element.addData( QStringLiteral( "current-scheme-index" ), mCurrentSchemeIndex );

  return element;
}

void ReosHydraulicNetwork::clear()
{
  qDeleteAll( mElements );
  mElements.clear();
  mHydraulicSchemeCollection->clear();
  mElementIndexesCounter.clear();
  mCurrentSchemeIndex = -1;
  emit hasBeenReset();
}

void ReosHydraulicNetwork::reset()
{
  qDeleteAll( mElements );
  mElements.clear();
  ReosMeteorologicModel *meteoModel = nullptr;
  if ( mWatershedModule->meteoModelsCollection()->modelCount() > 0 )
    meteoModel = mWatershedModule->meteoModelsCollection()->meteorologicModel( 0 );

  mHydraulicSchemeCollection->reset( meteoModel );

  mElementIndexesCounter.clear();
  mCurrentSchemeIndex = 0;
  emit hasBeenReset();
}

ReosGisEngine *ReosHydraulicNetwork::gisEngine() const
{
  return mGisEngine;
}


void ReosHydraulicNetwork::elemPositionChangedPrivate( ReosHydraulicNetworkElement *elem )
{
  emit elementPositionHasChanged( elem );
  emit dirtied();
}

ReosHydraulicNetworkContext ReosHydraulicNetwork::context() const
{
  ReosHydraulicNetworkContext context;
  context.mWatershedModule = mWatershedModule;
  context.mNetwork = const_cast<ReosHydraulicNetwork *>( this );
  context.mProjectName = mProjectName;
  context.mProjectPath = mProjectPath;
  return context;
}

ReosCalculationContext ReosHydraulicNetwork::calculationContext() const
{
  ReosCalculationContext context;
  ReosHydraulicScheme *currentScheme = mHydraulicSchemeCollection->scheme( mCurrentSchemeIndex );
  if ( currentScheme )
  {
    context.setMeteorologicModel( currentScheme->meteoModel() );
    context.setSchemeId( currentScheme->id() );
  }

  return context;
}

ReosHydraulicSchemeCollection *ReosHydraulicNetwork::hydraulicSchemeCollection() const
{
  return mHydraulicSchemeCollection;
}

int ReosHydraulicNetwork::schemeCount() const
{
  return mHydraulicSchemeCollection->schemeCount();
}

void ReosHydraulicNetwork::changeScheme( int newSchemeIndex )
{
  if ( newSchemeIndex == mCurrentSchemeIndex )
    return;

  if ( mCurrentSchemeIndex >= 0 )
  {
    for ( ReosHydraulicNetworkElement *elem : std::as_const( mElements ) )
    {
      ReosHydraulicScheme *scheme = mHydraulicSchemeCollection->scheme( mCurrentSchemeIndex );
      if ( scheme )
        elem->saveConfiguration( scheme );
    }
  }

  setCurrentScheme( newSchemeIndex );
}

void ReosHydraulicNetwork::onMapTimeWindowChanged()
{
  if ( mGisEngine )
    mGisEngine->setMapTimeWindow( mapTimeWindow() );
}

int ReosHydraulicNetwork::currentSchemeIndex() const
{
  return mCurrentSchemeIndex;
}

ReosHydraulicScheme *ReosHydraulicNetwork::currentScheme() const
{
  return mHydraulicSchemeCollection->scheme( mCurrentSchemeIndex );
}

ReosHydraulicScheme *ReosHydraulicNetwork::scheme( const QString &schemeId ) const
{
  return mHydraulicSchemeCollection->scheme( schemeId );
}

ReosHydraulicScheme *ReosHydraulicNetwork::scheme( int index ) const
{
  return mHydraulicSchemeCollection->scheme( index );
}

ReosHydraulicScheme *ReosHydraulicNetwork::schemeByName( const QString &schemeName ) const
{
  return mHydraulicSchemeCollection->schemeByName( schemeName );
}

int ReosHydraulicNetwork::schemeIndex( const QString &schemeId ) const
{
  return mHydraulicSchemeCollection->schemeIndex( schemeId );
}

void ReosHydraulicNetwork::setCurrentScheme( int schemeIndex )
{
  ReosHydraulicScheme *currentScheme = mHydraulicSchemeCollection->scheme( mCurrentSchemeIndex );
  if ( currentScheme )
  {
    disconnect( currentScheme, &ReosDataObject::dataChanged, this, &ReosHydraulicNetwork::schemeChanged );
    disconnect( currentScheme, &ReosHydraulicScheme::meteoTimeWindowChanged, this, &ReosHydraulicNetwork::onMapTimeWindowChanged );
  }

  mCurrentSchemeIndex = schemeIndex;

  for ( ReosHydraulicNetworkElement *elem :  std::as_const( mElements ) )
    elem->restoreConfiguration( mHydraulicSchemeCollection->scheme( schemeIndex ) );

  currentScheme = mHydraulicSchemeCollection->scheme( mCurrentSchemeIndex );
  if ( currentScheme )
  {
    connect( currentScheme, &ReosDataObject::dataChanged, this, &ReosHydraulicNetwork::schemeChanged );
    connect( currentScheme, &ReosHydraulicScheme::meteoTimeWindowChanged, this, &ReosHydraulicNetwork::onMapTimeWindowChanged );
  }

  for ( ReosHydraulicNetworkElement *elem :  std::as_const( mElements ) )
    if ( elem->type().contains( ReosHydraulicStructure2D::staticType() ) )
      elem->updateCalculationContext( calculationContext() );

  onMapTimeWindowChanged();
  emit schemeChanged();
  emit dirtied();
}

void ReosHydraulicNetwork::setCurrentScheme( const QString &schemeId )
{
  int index = schemeIndex( schemeId );
  setCurrentScheme( index );
}

QString ReosHydraulicNetwork::currentSchemeId() const
{
  ReosHydraulicScheme *scheme = mHydraulicSchemeCollection->scheme( mCurrentSchemeIndex );
  if ( scheme )
    return scheme->id();

  return QString();
}

QString ReosHydraulicNetwork::currentSchemeName() const
{
  ReosHydraulicScheme *scheme = mHydraulicSchemeCollection->scheme( mCurrentSchemeIndex );
  if ( scheme )
    return scheme->schemeName()->value();

  return QString();
}

ReosHydraulicScheme *ReosHydraulicNetwork::addNewScheme( const QString &schemeName )
{
  return addNewScheme( schemeName, nullptr );
}

ReosHydraulicScheme *ReosHydraulicNetwork::addNewScheme( const QString &schemeName, ReosMeteorologicModel *meteoModel )
{
  ReosHydraulicScheme *scheme = new ReosHydraulicScheme( mHydraulicSchemeCollection );
  scheme->schemeName()->setValue( schemeName );
  if ( meteoModel )
    scheme->setMeteoModel( meteoModel );
  else if ( mWatershedModule && mWatershedModule->meteoModelsCollection() && mWatershedModule->meteoModelsCollection()->modelCount() > 0 )
    scheme->setMeteoModel( mWatershedModule->meteoModelsCollection()->meteorologicModel( 0 ) );
  mHydraulicSchemeCollection->addScheme( scheme );
  return scheme;
}

void ReosHydraulicNetwork::addExistingScheme( ReosHydraulicScheme *scheme )
{
  mHydraulicSchemeCollection->addScheme( scheme );
}

void ReosHydraulicNetwork::removeScheme( int schemeIndex )
{
  ReosHydraulicScheme *sch = scheme( schemeIndex );
  for ( ReosHydraulicNetworkElement *elem : std::as_const( mElements ) )
    if ( elem )
      mUselessFile.append( elem->cleanScheme( sch ) );

  mHydraulicSchemeCollection->removeScheme( schemeIndex );
}

ReosDuration ReosHydraulicNetwork::currentTimeStep() const
{
  ReosDuration ret( qint64( 0 ) );

  ReosHydraulicScheme *scheme = currentScheme();
  if ( scheme )
  {
    for ( ReosHydraulicNetworkElement *elem : mElements )
    {
      ReosDuration elemTs = elem->currentElementTimeStep();
      if ( elemTs != ReosDuration( qint64( 0 ) ) && ( ret > elemTs || ret == ReosDuration( qint64( 0 ) ) ) )
        ret = elemTs;
    }
  }

  return ret;
}

ReosMapExtent ReosHydraulicNetwork::networkExtent() const
{
  ReosMapExtent extent;
  for ( const ReosHydraulicNetworkElement *elem : mElements )
    extent.extendWithExtent( elem->extent() );

  return mGisEngine->transformToProjectExtent( extent );
}

ReosTimeWindow ReosHydraulicNetwork::mapTimeWindow() const
{
  ReosTimeWindow tw;
  for ( ReosHydraulicNetworkElement *elem : mElements )
  {
    if ( elem->type().contains( ReosHydraulicStructure2D::staticType() ) )
      tw = tw.unite( elem->timeWindow() );
  }

  ReosHydraulicScheme *scheme = mHydraulicSchemeCollection->scheme( mCurrentSchemeIndex );
  if ( scheme && scheme->meteoModel() )
    tw = tw.unite( scheme->meteoModel()->timeWindow() );

  return tw;
}

ReosHydraulicNetworkElementCompatibilty ReosHydraulicNetwork::checkSchemeCompatibility( ReosHydraulicScheme *scheme ) const
{
  ReosHydraulicNetworkElementCompatibilty ret;
  for ( auto it = mElements.constBegin(); it != mElements.constEnd(); ++it )
    ret.combine( it.value()->checkCompatiblity( scheme ) );

  return ret;
}

QList<ReosHydraulicNetworkElement *> ReosHydraulicNetwork::hydraulicNetworkElements( const QString &type ) const
{
  const QList<ReosHydraulicNetworkElement *> allElement = mElements.values();
  if ( type.isEmpty() )
    return allElement;

  QList<ReosHydraulicNetworkElement *> ret;

  for ( ReosHydraulicNetworkElement *elem : allElement )
  {
    if ( elem->type().contains( type ) )
      ret.append( elem );
  }
  return ret;
}

void ReosHydraulicNetwork::addEncodedElement( const ReosEncodedElement &element )
{
  auto it = mElementFactories.find( element.description() );
  if ( it == mElementFactories.end() )
    return;

  it->second->decodeElement( element, context() );
}

ReosWatershedModule *ReosHydraulicNetworkContext::watershedModule() const
{
  return mWatershedModule;
}

ReosHydraulicNetwork *ReosHydraulicNetworkContext::network() const
{
  return mNetwork;
}

QString ReosHydraulicNetworkContext::crs() const
{
  if ( mNetwork->gisEngine() )
    return mNetwork->gisEngine()->crs();

  return QString();
}

QString ReosHydraulicNetworkContext::projectPath() const
{
  return mProjectPath;
}

QString ReosHydraulicNetworkContext::projectName() const
{
  return mProjectName;
}

ReosEncodeContext ReosHydraulicNetworkContext::encodeContext() const
{
  ReosEncodeContext context;
  context.setBaseDir( QDir( mProjectPath ) );
  return context;
}

QString ReosHydraulicNetworkContext::currentSchemeId() const
{
  if ( mNetwork && mNetwork->currentScheme() )
    return mNetwork->currentScheme()->id();
  return QString();
}

ReosHydraulicNetworkElementFactory::ReosHydraulicNetworkElementFactory()
{

}

ReosHydraulicNetworkElementFactory::~ReosHydraulicNetworkElementFactory() {}
