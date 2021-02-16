#include "reoswatershedmodule.h"

#include "reoswatersheddelineating.h"
#include "reosconcentrationtimecalculation.h"

ReosWatershedModule::ReosWatershedModule( ReosModule *parent, ReosGisEngine *gisEngine ):
  ReosModule( parent ),
  mWatershedTree( new ReosWatershedTree( gisEngine, this ) ),
  mDelineatingModule( new ReosWatershedDelineating( this, mWatershedTree, gisEngine ) )
{

}

ReosWatershedModule::~ReosWatershedModule()
{
  if ( ReosConcentrationTimeFormulasRegistery::isInstantiate() )
    delete ReosConcentrationTimeFormulasRegistery::instance();
}

ReosWatershedTree *ReosWatershedModule::watershedTree() const
{
  return mWatershedTree;
}


ReosWatershedDelineating *ReosWatershedModule::delineatingModule() const
{
  return mDelineatingModule;
}

void ReosWatershedModule::decode( const ReosEncodedElement &element )
{
  if ( element.description() != QStringLiteral( "watershed-module" ) )
    return;

  mWatershedTree->decode( element.getEncodedData( QStringLiteral( "watershed-tree" ) ) );
  mDelineatingModule->decode( element.getEncodedData( QStringLiteral( "delineating-module" ) ) );

  emit hasBeenReset();
}

ReosEncodedElement ReosWatershedModule::encode() const
{
  ReosEncodedElement ret( QStringLiteral( "watershed-module" ) );
  ret.addEncodedData( QStringLiteral( "watershed-tree" ), mWatershedTree->encode() );
  ret.addEncodedData( QStringLiteral( "delineating-module" ), mDelineatingModule->encode() );

  return ret;
}
