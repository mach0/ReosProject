/***************************************************************************
  reoshydrauliclink.cpp - ReosHydraulicLink

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
#include "reoshydrauliclink.h"

ReosHydraulicLink::ReosHydraulicLink( ReosHydraulicNetwork *parent )
  : ReosHydraulicNetworkElement( parent )
{}

ReosHydraulicLink::ReosHydraulicLink( const ReosEncodedElement &encodedElement, ReosHydraulicNetwork *parent )
  : ReosHydraulicNetworkElement( encodedElement, parent )
{}

ReosHydraulicNode *ReosHydraulicLink::firstNode() const
{
  if ( mNode_1.isNull() )
    return nullptr;
  else
    return mNode_1;
}

ReosHydraulicNode *ReosHydraulicLink::secondNode() const
{
  if ( mNode_2.isNull() )
    return nullptr;
  else
    return mNode_2;
}

void ReosHydraulicLink::destroy()
{
  mNode_1->detachFromSide1( this );
  mNode_2->detachFromSide2( this );
  ReosHydraulicNetworkElement::destroy();
}

void ReosHydraulicLink::attachOnSide1( ReosHydraulicNode *node )
{
  if ( mNode_1 != node )
  {
    if ( !mNode_1.isNull() )
      mNode_1->detachFromSide1( this );

    mNode_1 = node;

    node->attachBySide1( this );
    setObsolete();
  }
}

void ReosHydraulicLink::attachOnSide2( ReosHydraulicNode *node )
{
  if ( mNode_2 != node )
  {
    if ( !mNode_2.isNull() )
      mNode_2->detachFromSide2( this );

    mNode_2 = node;

    node->attachBySide2( this );

    setObsolete();
  }

}