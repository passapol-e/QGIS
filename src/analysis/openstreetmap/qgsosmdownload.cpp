/***************************************************************************
    qgsosmdownload.cpp
    ---------------------
    begin                : February 2013
    copyright            : (C) 2013 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsosmdownload.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include "qgsnetworkaccessmanager.h"
#include "qgsrectangle.h"
#include "qgssettings.h"


QString QgsOSMDownload::defaultServiceUrl()
{
  QgsSettings settings;
  return settings.value( QStringLiteral( "overpass_url" ), "http://overpass-api.de/api/interpreter" ).toString();
}


QString QgsOSMDownload::queryFromRect( const QgsRectangle &rect )
{
  return QStringLiteral( "(node(%1,%2,%3,%4);<;);out;" ).arg( rect.yMinimum() ).arg( rect.xMinimum() )
         .arg( rect.yMaximum() ).arg( rect.xMaximum() );
}


QgsOSMDownload::QgsOSMDownload()
  : mServiceUrl( defaultServiceUrl() )
{
}

QgsOSMDownload::QgsOSMDownload( const QString &query )
  : mServiceUrl( defaultServiceUrl() )
  , mQuery( query )
{
}

QgsOSMDownload::~QgsOSMDownload()
{
  if ( mReply )
  {
    mReply->abort();
    mReply->deleteLater();
    mReply = nullptr;
  }
}


bool QgsOSMDownload::start()
{
  mError.clear();

  if ( mQuery.isEmpty() )
  {
    mError = tr( "No query has been specified." );
    return false;
  }

  if ( mReply )
  {
    mError = tr( "There is already a pending request for data." );
    return false;
  }

  if ( !mFile.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
  {
    mError = tr( "Cannot open output file: %1" ).arg( mFile.fileName() );
    return false;
  }

  QgsNetworkAccessManager *nwam = QgsNetworkAccessManager::instance();

  QUrl url( mServiceUrl );
  url.addQueryItem( QStringLiteral( "data" ), mQuery );

  QNetworkRequest request( url );
  request.setRawHeader( "User-Agent", "QGIS" );

  mReply = nwam->get( request );

  connect( mReply, &QIODevice::readyRead, this, &QgsOSMDownload::onReadyRead );
  connect( mReply, static_cast < void ( QNetworkReply::* )( QNetworkReply::NetworkError ) >( &QNetworkReply::error ), this, &QgsOSMDownload::onError );
  connect( mReply, &QNetworkReply::finished, this, &QgsOSMDownload::onFinished );
  connect( mReply, &QNetworkReply::downloadProgress, this, &QgsOSMDownload::downloadProgress );

  return true;
}


bool QgsOSMDownload::abort()
{
  if ( !mReply )
    return false;

  mReply->abort();
  return true;
}


void QgsOSMDownload::onReadyRead()
{
  Q_ASSERT( mReply );

  QByteArray data = mReply->read( 1024 * 1024 );
  mFile.write( data );
}


void QgsOSMDownload::onFinished()
{
  qDebug( "finished" );
  Q_ASSERT( mReply );

  mReply->deleteLater();
  mReply = nullptr;

  mFile.close();

  emit finished();
}


void QgsOSMDownload::onError( QNetworkReply::NetworkError err )
{
  qDebug( "error: %d", err );
  Q_ASSERT( mReply );

  mError = mReply->errorString();
}


bool QgsOSMDownload::isFinished() const
{
  if ( !mReply )
    return true;

  return mReply->isFinished();
}
