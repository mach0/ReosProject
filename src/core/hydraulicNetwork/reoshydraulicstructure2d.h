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

#include "reoscore.h"

#include "reoshydraulicnetwork.h"
#include "reospolylinesstructure.h"
#include "reoshydraulicsimulation.h"
#include "reosgmshgenerator.h"
#include "reosmesh.h"
#include "reos3dmapsettings.h"
#include "reoshydraulicsimulationresults.h"
#include "reoshydraulicstructureprofile.h"

class ReosTopographyCollection;
class ReosRoughnessStructure;
class ReosHydraulicStructureBoundaryCondition;
class ReosStructureImporter;
class ReosTimeWindowSettings;
class QDir;

class REOSCORE_EXPORT ReosHydraulicStructure2D : public ReosHydraulicNetworkElement
{
    Q_OBJECT
  public:

    enum Structure2DCapability
    {
      GeometryEditable = 1 << 0, //!< If the structure have geometry editable (structure or mesh)
      MultiSimulation = 1 << 1, //!< If the structure can have multiple simulations
      DefinedExternally = 1 << 2, //!< If the structure is defined externally
      GriddedPrecipitation = 1 << 3 //!< If the structure can accept gridded precipitation on its domain
    };

    Q_ENUM( Structure2DCapability )
    Q_DECLARE_FLAGS( Structure2DCapabilities, Structure2DCapability )
    Q_FLAG( Structure2DCapabilities )

    //! Contructor from a \a domain, ccordinate system \a crs and a \a context
    ReosHydraulicStructure2D( const QPolygonF &domain, const QString &crs, const ReosHydraulicNetworkContext &context );
    ~ReosHydraulicStructure2D();

    //! Creates a structure from \a encodedElement and a \a context
    static ReosHydraulicStructure2D *create( const ReosEncodedElement &encodedElement, const ReosHydraulicNetworkContext  &context ) SIP_SKIP;

    //! Creates a structure from \a structureImporter
    static ReosHydraulicStructure2D *create( ReosStructureImporter *structureImporter, const ReosHydraulicNetworkContext &context ) SIP_SKIP;

    static QString staticType() {return ReosHydraulicNetworkElement::staticType() + QString( ':' ) + QStringLiteral( "structure2D" );}

    QString type() const override {return staticType();}
    void saveConfiguration( ReosHydraulicScheme *scheme ) const override SIP_SKIP;
    void restoreConfiguration( ReosHydraulicScheme *scheme ) override SIP_SKIP;
    ReosMapExtent extent() const override;
    QString defaultDisplayName() const override {return tr( "Hydraulic structure 2D" );}
    ReosDuration currentElementTimeStep() const override;
    ReosDuration mapTimeStep() const override;
    ReosTimeWindow timeWindow() const override;
    QIcon icon() const override;
    ReosHydraulicNetworkElementCompatibilty checkCompatiblity( ReosHydraulicScheme *scheme ) const override SIP_SKIP;
    QFileInfoList cleanScheme( ReosHydraulicScheme *scheme ) override SIP_SKIP;

    void updateCalculationContextFromUpstream( const ReosCalculationContext &context, ReosHydraulicStructureBoundaryCondition *boundaryCondition, bool upstreamWillChange )  SIP_SKIP {}
    bool updateCalculationContextFromDownstream( const ReosCalculationContext &context ) SIP_SKIP { return false; }

    //! Returns the directory where data and simulation will be stored on the disk
    QDir structureDirectory() const;

    //! Returns whether the structure supports the \a capability
    bool hasCapability( Structure2DCapability capability ) const;

    //! Returns a pointer to the time windows settings
    ReosTimeWindowSettings *timeWindowSettings() const;

#ifndef SIP_RUN

    //! Returns a pointer to the structure importer if exists, else return nullptr
    ReosStructureImporterSource *structureImporterSource() const;

    //! Returns the domain polygon
    QPolygonF domain( const QString &crs = QString() ) const;

    //! Returns the geometrical strcuture
    ReosPolylinesStructure *geometryStructure() const;

    //**************************** mesh related methods

    //! Returns the mesh resolution controler
    ReosMeshResolutionController *meshResolutionController() const;

    //! Returns the mesh
    ReosMesh *mesh() const;

    //! Returns the mesh generator
    ReosMeshGenerator *meshGenerator() const;

    //! Generate the mesh in place and block until the mesh is generated
    void generateMesh();

    //! Returns a process that generate the mesh, caller take ownership
    ReosMeshGeneratorProcess *getGenerateMeshProcess();

    //! Returns the data of the structure necessary for simulation
    ReosSimulationData simulationData( const QString &schemeId, ReosModule::Message &message ) const;

    //! Returns the topography collection
    ReosTopographyCollection *topographyCollecion() const;

    //! Returns the rougness structure
    ReosRoughnessStructure *roughnessStructure() const;

    //! Sets active the terrain in the mesh
    void activateMeshTerrain();

    //! Returns the id of the terrain dataset
    QString terrainMeshDatasetId() const;

    //! Returns the elevation of the terran at \a position in map coordinates
    double terrainElevationAt( const QPointF &position );

    //! Deactivates any activated scalar dataset
    void deactivateMeshScalar();

    //! Deactivates any activated vector dataset
    void deactivateMeshVector();

    //! Returns the 3D map settings
    Reos3DMapSettings map3dSettings() const;

    //! Sets the 3D map settings
    void setMap3dSettings( const Reos3DMapSettings &value );

    //! Returns the 3D terrain settings
    Reos3DTerrainSettings terrain3DSettings() const;

    //! Sets the 3D terrain settings
    void setTerrain3DSettings( const Reos3DTerrainSettings &settings );

    //**************************** boundary condition related methods

    //! Returns all the boundary condition of this stucture
    QList<ReosHydraulicStructureBoundaryCondition *> boundaryConditions() const;

    //! Returns alls the boundary condition id
    QStringList boundaryConditionId() const;

    //! Returns the boundary condition with id \a boundaryId, nullptr if not exists
    ReosHydraulicStructureBoundaryCondition *boundaryConditionNetWorkElement( const QString &boundaryId ) const;

    //**************************** Simulation condition related methods

    //! Adds a new simulation with \a key corresponding to a engine and sets it the current one. Returns true if the simulation is effectivly added
    bool addSimulation( const QString &key );

    //! Remove the simulation with \a index
    void removeSimulation( int index );

    //! Returns a pointer to the simulation associated with \a scheme
    ReosHydraulicSimulation *simulation( ReosHydraulicScheme *scheme ) const;

#endif // No SIP_RUN
    //! Returns a pointer to the current simulation
    ReosHydraulicSimulation *currentSimulation() const;
#ifndef SIP_RUN

    //! Returns the count of simulation
    int simulationCount() const {return mSimulations.count();}

    //! Returns the index of the current simulation
    int currentSimulationIndex() const;

    //! Sets the current simulation with its \a index
    void setCurrentSimulation( int index );

    //! Returns the name of all the simulations
    QStringList simulationNames() const;

    //**************************** Calculation process related methods

    //! Returns a process that prepare the current simulation, caller take ownership
    ReosSimulationPreparationProcess *getPreparationProcessSimulation( const ReosSimulationData &simData, const ReosCalculationContext &context, ReosModule::Message &message );

    //! Returns a process that prepare the current simulation files in a specific \a diectory, caller take ownership
    ReosSimulationPreparationProcess *getPreparationProcessSimulation( const ReosSimulationData &simData, const ReosCalculationContext &context, ReosModule::Message &message, const QDir &directory );

    /**
     * Creates a new simulation process and returns a pointer to the process, the process is not started.
     * Caller does NOT take ownership ship and the object is destroyed once finished
     */
    ReosSimulationProcess *createSimulationProcess( const ReosCalculationContext &context, QString &error );

    //! Returns  a pointer to the current simulation process
    ReosSimulationProcess *simulationProcess( const ReosCalculationContext &context ) const;

    //! Returns whether a simulation is running
    bool hasSimulationRunning() const;

#endif // No SIP_RUN
    //! Start running a simulation and wait the simulation finishes
    bool runSimulation( const ReosCalculationContext &context );

#ifndef SIP_RUN

    //**************************** Results related methods

    //! Updates results corresponding to the scheme with id \a schemeId
    void updateResults( const QString &schemeId );

    //! Returns whether the structure contain any results
    bool hasResults() const;

    //! Returns whether a result corresponding to \a context is existing (loaded)
    bool hasResults( const QString &schemeId ) const;

    //! Returns whether a result corresponding to \a scheme is existing (loaded)
    bool hasResults( const ReosHydraulicScheme *scheme ) const;

    //! Returns whether a results corresponding to \a context is existing
    QDateTime resultsRunDateTime( const QString &schemeId ) const;

    //! Returns the time step count of the results corresponding to \a context
    int resultsTimeStepCount( const QString &schemeId ) const;

#endif // No SIP_RUN
    //! Returns the value of the results with type \a datasetType for the specified \a context, \a position and \a time
    double resultsValueAt( const QDateTime &time,
                           const ReosSpatialPosition &position,
                           ReosHydraulicSimulationResults::DatasetType datasetType,
                           const QString &schemeId );

    //! Returns a translated string corresponding to the unit of the results associated with \a context and to the type  \a datasetType
    QString resultsUnits( ReosHydraulicSimulationResults::DatasetType datasetType, const QString &schemeId );

    /**
     * Creates a raster file \a fileName with results of type \a datasetType, for the scheme \a schemeId, at \a time,
     *  with coordinates reference système (WKT) \a destination CRS, and with \a resolution in map unit of \a destinationCrs.
     *  Return True if successful.
     */
    bool rasterizeResult( const QDateTime &time,
                          ReosHydraulicSimulationResults::DatasetType datasetType,
                          const QString &schemeId,
                          const QString &fileName,
                          const QString &destinationCrs,
                          double resolution );
#ifndef SIP_RUN

    //! Removes and erase all results related to the structure, also from sources (disk).
    void removeAllResults();

    //! Remove the result associated with \a context
    void removeResults( const ReosCalculationContext &context );

    //! Clears reference to result in this structure (do not remove from source) for the scheme with id \a schemeId
    void clearResults( const QString &schemeId );

    //! Returns the results associated with \a scheme
    ReosHydraulicSimulationResults *results( ReosHydraulicScheme *scheme );

    //! Activates the result dataset groups with \a id. If id is void, the current group is reactivated
    void activateResultDatasetGroup( const QString &id = QString() );

    //! Activates the result vector dataset groups with \a id. If id is void, the current group is reactivated
    void activateResultVectorDatasetGroup( const QString &id );

    //! Returns the all the dataset ids
    QStringList meshDatasetIds() const;

    //! Returns all the vector dataset ids
    QStringList meshVectorDatasetIds() const;

    //! Returns the name of the dataset with \a id
    QString meshDatasetName( const QString &id ) const;

    //! Returns the id of the current dataset
    QString currentActivatedMeshDataset() const;

    //! Returns the id of the current vector dataset
    QString currentActivatedVectorMeshDataset() const;

    //! Returns the type of the current dataset
    ReosHydraulicSimulationResults::DatasetType currentActivatedDatasetResultType() const;

    //! Return the name of the current dataset
    QString currentDatasetName() const;

    /**
     * Exports current results in a UGRID mesh file, using only results contained in the time window.
     * If the time window is not provided or if it is invalid, all the results are exported.
     */
    void exportResultAsMesh( const QString &fileName, const ReosTimeWindow &timeWindow = ReosTimeWindow() ) const;

    ReosModule::Message exportResultAsMeshInGisProject( ReosHydraulicScheme *scheme, const QString &fileName, bool keepLayers );

    //**************************** Profiles related methods

    //! Returns a pointer to the profile collection
    ReosHydraulicStructureProfilesCollection *profilesCollection() const;

    //! Creates a profile with \a name, a geometry in the plan \a linesInPlan that has \a lineCrs as coordinate system
    int createProfile( const QString &name, const QPolygonF &linesInPlan, const QString &linesCrs );

    //! Removes the profile with \a index
    void removeProfile( int index );

    //! Renames the profile with \a index with \a name
    void renameProfile( int index, const QString &name );

    //! Return the count of profile
    int profilesCount() const;

    //! Returns the profile with \a profileIndex
    ReosHydraulicStructureProfile *profile( int profileIndex ) const;

    //! Returns the index of the \a profile
    int profileIndex( ReosHydraulicStructureProfile *profile );

  public slots:
    void updateCalculationContext( const ReosCalculationContext &context ) override;

    void onExtrernalBoundaryConditionRemoved( const QString &bcId );
    void onExtrernalBoundaryConditionAdded( const QString &bcId );

  signals:
    void meshGenerated();

    //! Emmited when boundaries change (add/remove)
    void boundariesChanged();

    //! Emitted when values of a boundary is updated
    void boundaryUpdated( ReosHydraulicStructureBoundaryCondition *bc );
    void currentSimulationChanged();
    void simulationFinished( bool success );
    void simulationResultChanged();

  protected:
    void encodeData( ReosEncodedElement &element, const ReosHydraulicNetworkContext &context ) const override;

#endif //No SIP_RUN
  private slots:
    void onBoundaryConditionAdded( const QString &bid );
    void onBoundaryConditionRemoved( const QString &bid );
    void onGeometryStructureChange();
    void onFlowsFromSolverReceived( const QDateTime &time, const QStringList &boundId, const QList<double> &values );
    void onSimulationFinished( ReosHydraulicSimulation *simulation,  const QString &schemeId, ReosSimulationProcess *process, bool success );

  private:
    ReosHydraulicStructure2D( const ReosEncodedElement &encodedElement, const ReosHydraulicNetworkContext &context );
    ReosHydraulicStructure2D( ReosStructureImporter *importer, const ReosHydraulicNetworkContext &context );

    Structure2DCapabilities mCapabilities;

    // Base geometry attributes
    std::unique_ptr<ReosPolylinesStructure> mPolylinesStructures;
    std::unique_ptr<ReosMesh> mMesh;
    QSet<QString> mBoundaryConditions;

    // Geometry editor helper existing when geometry is editable
    ReosMeshGenerator *mMeshGenerator = nullptr;
    ReosMeshResolutionController *mMeshResolutionController = nullptr;
    ReosTopographyCollection    *mTopographyCollection = nullptr;
    std::unique_ptr<ReosRoughnessStructure > mRoughnessStructure;

    QVector<QVector<int>> mBoundaryVertices;
    QVector<QVector<QVector<int>>> mHolesVertices;

    bool mMeshNeedToBeGenerated = true;

    std::unique_ptr<ReosStructureImporterSource> mStructureImporterSource;

    QList<ReosHydraulicSimulation *> mSimulations;

    ReosHydraulicStructureProfilesCollection *mProfilesCollection = nullptr;

    //** configuration related members
    int mCurrentSimulationIndex = -1;
    ReosTimeWindowSettings *mTimeWindowSettings = nullptr;
    //**

    std::map<QString, std::unique_ptr<ReosSimulationProcess>> mSimulationProcesses;
    QMap<QString, ReosHydraulicSimulationResults *> mSimulationResults;
    QPointer<ReosHydraulicSimulationResults> mCurrentResult;

    Reos3DMapSettings m3dMapSettings;
    Reos3DTerrainSettings m3dTerrainSettings;

    void initConnection();
    void generateMeshInPlace();
    QString directory() const;
    void onMeshGenerated( const ReosMeshFrameData &meshData );

    void loadResult( ReosHydraulicSimulation *simulation, const QString &schemeId );
    void setResultsOnStructure( ReosHydraulicSimulationResults *simResults );

    ReosSimulationProcess *processFromScheme( const QString &schemeId ) const;

    int simulationIndexFromId( const QString &simId ) const;

    friend class ReoHydraulicStructure2DTest;
};

#ifndef SIP_RUN
class ReosHydraulicStructure2dFactory : public ReosHydraulicNetworkElementFactory
{
  public:
    ReosHydraulicStructure2dFactory() = default;
    ReosHydraulicNetworkElement *decodeElement( const ReosEncodedElement &encodedElement, const ReosHydraulicNetworkContext &context ) const override;
};


class REOSCORE_EXPORT ReosRoughnessStructure : public ReosDataObject
{
    Q_OBJECT
  public:
    ReosRoughnessStructure( const QString &mCrs );
    ReosRoughnessStructure( const ReosEncodedElement &encodedElement );

    ReosEncodedElement encode() const;
    ReosParameterDouble *defaultRoughness() const;
    ReosPolygonStructure *structure() const;

  private:
    std::unique_ptr<ReosPolygonStructure> mStructure;
    ReosParameterDouble *mDefaultRoughness = nullptr;
};

#endif //No SIP_RUN


#endif // REOSHYDRAULICSTRUCTURE2D_H
