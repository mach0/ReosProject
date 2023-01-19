#include "reoshecrassimulationimportwidget.h"
#include "ui_reoshecrassimulationimport.h"

#include <QFileDialog>

#include "reoshecrassimulation.h"
#include "reossettings.h"


ReosHecRasSimulationImportWidget::ReosHecRasSimulationImportWidget( QWidget *parent )
  : ReosImportHydraulicStructureWidget( parent )
  , ui( new Ui::ReosHecRasSimulationImportWidget )
{
  ui->setupUi( this );

  connect( ui->mProjectFileButton, &QToolButton::clicked, this, &ReosHecRasSimulationImportWidget::onProjectFileButtonPressed );
}

ReosHydraulicStructure2D *ReosHecRasSimulationImportWidget::importStructure2D( const ReosHydraulicNetworkContext &context ) const
{
  ReosHecRasStructureImporterSource source( ui->mProjectFileLineEdit->text(), context );
  std::unique_ptr<ReosStructureImporter> importer( source.createImporter() );
  return ReosHydraulicStructure2D::create( importer.get(), context );
}

void ReosHecRasSimulationImportWidget::onProjectFileButtonPressed()
{
  ReosSettings settings;
  const QString dirName = settings.value( QStringLiteral( "ImportFile/directory" ) ).toString();
  const QString fileName = QFileDialog::getOpenFileName(
                             this,
                             tr( "Choose HEC-RAS project file" ),
                             dirName,
                             tr( "HEC-RAS project file *.prj" ) );

  if ( !fileName.isEmpty() )
  {
    ui->mProjectFileLineEdit->setText( fileName );
    QFileInfo fileInfo( fileName );
    settings.setValue( QStringLiteral( "ImportFile/directory" ), fileInfo.dir().path() );
  }

}
