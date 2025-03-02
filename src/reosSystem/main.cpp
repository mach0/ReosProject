/***************************************************************************
  main.cpp

 ---------------------
 begin                : 24.3.2023
 copyright            : (C) 2023 by Vincent Cloarec
 email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "reosapplication.h"
#include "lekanmainwindow.h"
#include "reossettings.h"

#include <QSystemTrayIcon>
#include <QAction>
#include <QMenu>

int main( int argc, char *argv[] )
{
#ifdef _MSC_VER
  qputenv( "PATH", "C:\\WINDOWS\\system32;C:\\WINDOWS;C:\\WINDOWS\\system32\\WBem" );
#endif

  std::unique_ptr<ReosApplication> a( ReosApplication::initializationReos( argc, argv, QStringLiteral( "Reos System" ) ) );

  std::unique_ptr<LekanMainWindow> lekan;
  QSystemTrayIcon *trayIcon = new QSystemTrayIcon( a.get() );

  QAction *actionStartLekan = new QAction( QObject::tr( "Open Lekan" ), trayIcon );
  QObject::connect( actionStartLekan, &QAction::triggered, a.get(), [&]
  {
    if ( !lekan )
      lekan.reset( new LekanMainWindow( a->coreModule() ) );

    if ( lekan->isVisible() )
      return;

    ReosSettings settings;
    if ( settings.contains( QStringLiteral( "Windows/MainWindow/geometry" ) ) )
    {
      lekan->restoreGeometry( settings.value( QStringLiteral( "Windows/MainWindow/geometry" ) ).toByteArray() );
      lekan->showMaximized();
    }
    else
    {
      lekan->showMaximized();
    }
  } );

  QMenu contextMenu;
  contextMenu.addAction( actionStartLekan );

  trayIcon->setContextMenu( &contextMenu );
  trayIcon->show();

  int ret = a->exec();

  return ret;

}
