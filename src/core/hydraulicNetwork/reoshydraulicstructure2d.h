/***************************************************************************
  reoshydraulicstructure2d.h - ReosHydraulicStructure2D

 ---------------------
 begin                : 9.1.2022
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
#ifndef REOSHYDRAULICSTRUCTURE2D_H
#define REOSHYDRAULICSTRUCTURE2D_H

#include "reoshydraulicnetwork.h"
#include "reospolylinesstructure.h"
#include "reosgmshgenerator.h"
#include "reosmesh.h"

class ReosTopographyCollection;

class ReosHydraulicStructure2D : public ReosHydraulicNetworkElement
{
    Q_OBJECT
  public:
    ReosHydraulicStructure2D( const QPolygonF &domain, const QString &crs, ReosHydraulicNetwork *parent = nullptr );

    QString type() const override {return staticType();}
    static QString staticType() {return ReosHydraulicNetworkElement::staticType() + QString( ':' ) + QStringLiteral( "structure2D" );}

    //! Returns the domain polygon
    QPolygonF domain( const QString &crs = QString() ) const;

    ReosPolylinesStructure *geometryStructure() const;
    ReosMeshResolutionController *meshResolutionController() const;
    ReosMesh *mesh() const;

    static ReosHydraulicStructure2D *create( const ReosEncodedElement &encodedElement, ReosHydraulicNetwork *parent = nullptr );

    ReosMeshGenerator *meshGenerator() const;
    ReosMeshGeneratorProcess *getGenerateMeshProcess();

    //! Sets active the terrain in the mesh
    void activateMeshTerrain();

    //! Deactivate any
    void deactivateMeshScalar();

    QString terrainMeshDatasetId() const;

    void runSimulation();

    ReosTopographyCollection *topographyCollecion() const;

  public slots:
    void updateCalculationContext( const ReosCalculationContext &context ) {}

  protected:
    void encodeData( ReosEncodedElement &element, const ReosHydraulicNetworkContext &context ) const;

  private:
    ReosHydraulicStructure2D( const ReosEncodedElement &encodedElement, ReosHydraulicNetwork *parent );

    ReosMeshGenerator *mMeshGenerator = nullptr;
    std::unique_ptr<ReosPolylinesStructure> mPolylinesStructures;
    ReosMeshResolutionController *mMeshResolutionController = nullptr;
    ReosTopographyCollection  *mTopographyCollecion = nullptr;
    std::unique_ptr<ReosMesh> mMesh;
    QString mTerrainDatasetId;

    void init();
    void generateMeshInPlace();

    QString directory() const;
};

class ReosHydraulicStructure2dFactory : public ReosHydraulicNetworkElementFactory
{
  public:
    ReosHydraulicStructure2dFactory() = default;
    ReosHydraulicNetworkElement *decodeElement( const ReosEncodedElement &encodedElement, const ReosHydraulicNetworkContext &context ) const override;
};


#endif // REOSHYDRAULICSTRUCTURE2D_H
