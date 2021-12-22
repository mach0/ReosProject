/***************************************************************************
  reoshydrographtransfer.h - ReosHydrographTransfer

 ---------------------
 begin                : 19.5.2021
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
#ifndef REOSHYDROGRAPHTRANSFER_H
#define REOSHYDROGRAPHTRANSFER_H

#include <QPointer>
#include <QPointF>

#include "reoshydrographsource.h"
#include "reoshydrauliclink.h"
#include "reoscalculationcontext.h"

class ReosHydrographRoutingLink;

class ReosHydrographRoutingMethod : public ReosDataObject
{
  public:
    ReosHydrographRoutingMethod( ReosHydrographRoutingLink *routingLink );
    virtual void calculateOutputHydrograph( ReosHydrograph *inputHydrograph, ReosHydrograph *outputHydrograph, const ReosCalculationContext &context ) = 0;

    QString type() const override {return staticType();}
    static QString staticType() {return QStringLiteral( "hydrographRoutineMethod" );}

    virtual ReosHydrographCalculation *calculationProcess( ReosHydrograph *inputHydrograph, const ReosCalculationContext &context ) = 0;

    virtual ReosEncodedElement encode() const = 0;
};

class ReosHydrographRoutingMethodFactory
{
  public:
    virtual ReosHydrographRoutingMethod *createRoutingMethod( ReosHydrographRoutingLink *routingLink ) const = 0;
    virtual ReosHydrographRoutingMethod *createRoutingMethod( const ReosEncodedElement &encodedElement, ReosHydrographRoutingLink *routingLink ) const = 0;
    virtual QString type() const = 0;
    virtual QString displayName() const = 0;
};


class ReosHydrographRoutingMethodFactories : public ReosModule
{
  public:
    ~ReosHydrographRoutingMethodFactories();

    static void instantiate( ReosModule *parent );

    static bool isInstantiate();
    static ReosHydrographRoutingMethodFactories *instance();

    void addFactory( ReosHydrographRoutingMethodFactory *factory );

    ReosHydrographRoutingMethod *createRoutingMethod( const QString &type, ReosHydrographRoutingLink *link );
    ReosHydrographRoutingMethod *createRoutingMethod( const ReosEncodedElement &encodedElement, ReosHydrographRoutingLink *link );

    QString displayName( const QString &type ) const;
    QStringList methodTypes() const;

  private:
    ReosHydrographRoutingMethodFactories( ReosModule *parent = nullptr );
    static ReosHydrographRoutingMethodFactories *sInstance;
    using Factory = std::unique_ptr<ReosHydrographRoutingMethodFactory>;
    std::map<QString, Factory> mFactories;
};


/**
 * Abstract class that can be derived to make hydraulic link thats transfer hydrograph between a hydrograph source to another hydrograph node
 */
class ReosHydrographRoutingLink : public ReosHydraulicLink
{
    Q_OBJECT
  public:
    //! Constructor
    ReosHydrographRoutingLink( ReosHydraulicNetwork *parent = nullptr );

    //! Constructor with input and ouptut
    ReosHydrographRoutingLink( ReosHydrographSource *hydrographSource, ReosHydrographNode *destination, ReosHydraulicNetwork *parent = nullptr );

    bool setCurrentRoutingMethod( const QString &routingType );
    ReosHydrographRoutingMethod *currentRoutingMethod() const;

    //! Sets the input hydrograph source
    void setInputHydrographSource( ReosHydrographSource *hydrographSource );

    //! Returns the input hydrograph source
    ReosHydrographSource *inputHydrographSource() const;

    ReosHydrographNode *destinationNode() const;

    //! Sets the destination node
    void setHydrographDestination( ReosHydrographNode *destination );

    //! Returns the input hydrograph
    virtual ReosHydrograph *inputHydrograph() const;

    //! Returns the output hydrograph
    virtual ReosHydrograph *outputHydrograph() const;

    QString type() const override {return staticType();}
    QString static staticType() {return ReosHydraulicLink::staticType() + QString( ':' ) + QStringLiteral( "routine" ); }
    QString defaultDisplayName() const override {return tr( "Hydrograph routine" );}

    static ReosHydrographRoutingLink *decode( const ReosEncodedElement &encodedElement, const ReosHydraulicNetworkContext &context );

    //! Updates the output hydrograph for the context calculation \a context
    void updateCalculationContext( const ReosCalculationContext &context ) override;

    void updateCalculationContextFromUpstream( const ReosCalculationContext &context, bool upstreamWillChange );
    bool updateCalculationContextFromDownstream( const ReosCalculationContext &context );

  public slots:
    void calculateRouting();

  protected:
    ReosHydrographRoutingLink( ReosHydrographSource *hydrographSource,
                               ReosHydrographNode *destination,
                               const ReosEncodedElement &encodedElement,
                               ReosHydraulicNetwork *parent = nullptr );

  private slots:
    void onSourceUpdated();

  private:

    QMap<QString, ReosHydrographRoutingMethod *> mRoutingMethods;
    QString mCurrentRoutingMethod;
    ReosHydrographCalculation *mCalculation = nullptr;

    ReosHydrograph *mOutputHydrograph = nullptr;

    void encodeData( ReosEncodedElement &element,  const ReosHydraulicNetworkContext &context ) const override;

    void init();
};

class ReosHydrographRoutingLinkFactory: public ReosHydraulicNetworkElementFactory
{
  public:
    ReosHydrographRoutingLinkFactory() = default;

    virtual ReosHydraulicNetworkElement *decodeElement( const ReosEncodedElement &encodedElement, const ReosHydraulicNetworkContext &context ) const;
};

//! Class that transfers hydrograph between node without altering the hydrograph
class ReosDirectHydrographRouting: public ReosHydrographRoutingMethod
{
    Q_OBJECT
  public:
    ReosDirectHydrographRouting( ReosHydrographRoutingLink *routingLink );

    void calculateOutputHydrograph( ReosHydrograph *inputHydrograph,
                                    ReosHydrograph *outputHydrograph,
                                    const ReosCalculationContext &context ) override;

    ReosHydrographCalculation *calculationProcess( ReosHydrograph *inputHydrograph, const ReosCalculationContext &context ) override;

    QString type() const override {return staticType();}
    QString static staticType() {return ReosHydrographRoutingMethod::staticType() + QString( ':' ) + QStringLiteral( "direct" ); }

    ReosEncodedElement encode() const override
    {
      ReosEncodedElement element( type() );
      return element;
    }

  private:

    class Calculation: public ReosHydrographCalculation
    {
      public:
        Calculation( ReosHydrograph *inputHydrograph )
        {
          mInputHydrograph = std::make_unique<ReosHydrograph>();
          mInputHydrograph->copyFrom( inputHydrograph );
        }

        void start()
        {
          mHydrograph.reset( new ReosHydrograph );
          mHydrograph->copyFrom( mInputHydrograph.get() );
          mIsSuccessful = true;
        }
      private:
        std::unique_ptr<ReosHydrograph> mInputHydrograph;
    };
};

class ReosDirectHydrographRoutingFactory : public ReosHydrographRoutingMethodFactory
{
  public:
    ReosHydrographRoutingMethod *createRoutingMethod( ReosHydrographRoutingLink *routingLink ) const override
    {return new ReosDirectHydrographRouting( routingLink );}

    ReosHydrographRoutingMethod *createRoutingMethod( const ReosEncodedElement &, ReosHydrographRoutingLink *routingLink ) const override
    {return new ReosDirectHydrographRouting( routingLink );};

    virtual QString type() const override
    {return ReosDirectHydrographRouting::staticType();}

    QString displayName() const override {return QObject::tr( "Without distortion" );}
};



#endif // REOSHYDROGRAPHTRANSFER_H
