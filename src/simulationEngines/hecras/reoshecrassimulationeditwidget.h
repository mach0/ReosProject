#ifndef REOSHECRASSIMULATIONEDITWIDGET_H
#define REOSHECRASSIMULATIONEDITWIDGET_H

#include <QWidget>
#include <QDialog>
#include <QComboBox>

#include "reoshydraulic2dsimulationwidget.h"
#include "reosduration.h"

class ReosHecRasSimulation;

namespace Ui
{
  class ReosHecRasSimulationEditWidget;
  class ReosHecrasConfigurationEngineDialog;
}

class ReosHecRasIntervalComputationCombo : public QComboBox
{
  public:
    enum Configuration
    {
      Big,
      Small
    };

    ReosHecRasIntervalComputationCombo( QWidget *parent = nullptr );

    void setConfig( Configuration config );
    ReosDuration currentInterval() const;
    void setInterval( const ReosDuration &duration );
};


class ReosHecRasSimulationEditWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit ReosHecRasSimulationEditWidget( ReosHecRasSimulation *simulation, QWidget *parent = nullptr );
    ~ReosHecRasSimulationEditWidget();

  private slots:
    void onPlanChanged();
    void onIntervalChanged();

  private:
    Ui::ReosHecRasSimulationEditWidget *ui;
    ReosHecRasSimulation *mSimulation = nullptr;
};

class ReosHecrasConfigurationEngineDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit ReosHecrasConfigurationEngineDialog( QWidget *parent = nullptr );
    ~ReosHecrasConfigurationEngineDialog();

  private slots:
    void onAccepted();

  private:
    Ui::ReosHecrasConfigurationEngineDialog *ui;
};


class ReosHecRasSimulationEditWidgetFactory : public ReosHydraulicSimulationWidgetFactory
{
  public:
    QString key() const override;

    QWidget *simulationSettingsWidget( ReosHydraulicStructure2D *structure, ReosHydraulicSimulation *simulation, const ReosGuiContext &guiContext ) const override;
    QDialog *engineConfigurationDialog( QWidget *parent ) const override;
    QWidget *simulationEngineDescription( QWidget *parent ) const override { return nullptr; }
    ReosImportHydraulicStructureWidget *simulationImportWidget( QWidget *parent ) const override;
};

#endif // REOSHECRASSIMULATIONEDITWIDGET_H
