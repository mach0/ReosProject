/***************************************************************************
  reostelemac2dsimulation.h - ReosTelemac2DSimulation

 ---------------------
 begin                : 31.3.2022
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
#ifndef REOSTELEMAC2DSIMULATION_H
#define REOSTELEMAC2DSIMULATION_H

#include "reoshydraulicsimulation.h"
#include "reoscore.h"

class ReosTelemac2DSimulationProcess : public ReosSimulationProcess
{
    Q_OBJECT
  public:
    ReosTelemac2DSimulationProcess( QString simulationfilePath );

    void start() override;
    void stop( bool b ) override;

  private:
    QString mSimulationFilePath;
    QProcess *mProcess = nullptr;

    QFile *tFile = nullptr;
    QTextStream *mStre = nullptr;
};


class ReosTelemac2DSimulation : public ReosHydraulicSimulation
{
    Q_OBJECT
  public:
    enum class Equation
    {
      FiniteVolume,
      FiniteElement
    };

    ReosTelemac2DSimulation( QObject *parent = nullptr );
    ReosTelemac2DSimulation( const ReosEncodedElement &element, QObject *parent = nullptr );
    static QString staticKey() {return QStringLiteral( "telemac2D" );}

    QString key() const override {return ReosTelemac2DSimulation::staticKey();}
    QString directoryName() const override {return  QStringLiteral( "TELEMAC_simulation" );}
    void prepareInput( ReosHydraulicStructure2D *hydraulicStructure, const ReosCalculationContext &context ) override;
    virtual ReosSimulationProcess *getProcess( ReosHydraulicStructure2D *hydraulicStructure ) const override;
    ReosEncodedElement encode() const override;

    ReosParameterDuration *timeStep() const;
    ReosParameterInteger *outputPeriod() const;
    ReosSimulationInitialConditions *initialCondition() const;

    Equation equation() const;
    void setEquation( const Equation &equation );

  private:
    ReosParameterDuration *mTimeStep = nullptr;
    ReosParameterInteger *mOutputPeriod = nullptr;
    Equation mEquation = Equation::FiniteElement;
    ReosSimulationInitialConditions *mInitialCondition = nullptr;

    QList<ReosHydraulicStructureBoundaryCondition *> createBoundaryFiles( ReosHydraulicStructure2D *hydraulicStructure, QVector<int> &verticesPosInBoundary );
    void createSelafinMeshFrame( ReosHydraulicStructure2D *hydraulicStructure, const QVector<int> &verticesPosInBoundary );
    void createSelafinInputGeometry( ReosHydraulicStructure2D *hydraulicStructure, const QVector<int> &verticesPosInBoundary );
    void createBoundaryConditionFiles( ReosHydraulicStructure2D *hydraulicStructure, QList<ReosHydraulicStructureBoundaryCondition *> boundaryConditions, const ReosCalculationContext &context );
    void createSteeringFile( ReosHydraulicStructure2D *hydraulicStructure,  QList<ReosHydraulicStructureBoundaryCondition *> boundaryConditions, const ReosCalculationContext &context );
};


class ReosTelemac2DSimulationEngineFactory : public ReosSimulationEngineFactory
{
  public:

    virtual ReosHydraulicSimulation *createSimulation( QObject *parent ) const;
    virtual ReosHydraulicSimulation *createSimulation( const ReosEncodedElement &element, QObject *parent ) const;
    virtual QString key() const {return ReosTelemac2DSimulation::staticKey();}
    QString displayName() const {return QObject::tr( "TELEMAC 2D Simulation" );}
};

#endif // REOSTELEMAC2DSIMULATION_H
