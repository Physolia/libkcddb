/*
  Copyright (C) 2002 Rik Hemsley (rikkus) <rik@kde.org>
  Copyright (C) 2002 Benjamin Meyer <ben-devel@meyerhome.net>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
  Boston, MA 02111-1307, USA.
*/

#ifndef KCDDB_DEFINES_H
#define KCDDB_DEFINES_H

#include <qstring.h>
#include <qvaluelist.h>

namespace KCDDB
{
  typedef QValueList<uint> TrackOffsetList;

  enum Error
  {
    None,
    HostNotFound,
    NoResponse,
    NoSuchCD,
    CannotSave,
    Unknown
  };

  struct TrackInfo
  {
    bool    offsetKnown;
    uint    offset;
    QString title;
  };

  typedef QValueList<TrackInfo> TrackInfoList; 

  struct CDInfo
  {
    QString       artist;
    QString       title;
    QString       genre;
    uint          year;
    uint          length;
    QString       id;
    TrackInfoList trackInfoList;
  };

  enum SubmitTransport
  {
    LocalSubmit,
    HTTPSubmit,
    SMTPSubmit
  };

  enum LookupTransport
  {
    CacheOnlyLookup,
    CDDBLookup,
    HTTPLookup,
    CDDBLookupIgnoreCached,
    HTTPLookupIgnoreCached
  };

  QString submitTransportToString(SubmitTransport);
  QString lookupTransportToString(LookupTransport);

  SubmitTransport stringToSubmitTransport(const QString &);
  LookupTransport stringToLookupTransport(const QString &);

  QString errorToString(Error);

  QString trackOffsetListToId     (const TrackOffsetList &);
  QString trackOffsetListToString (const TrackOffsetList &);

  CDInfo parseStringListToCDInfo(const QStringList &);
}

#endif // KCDDB_DEFINES_H
// vim:tabstop=2:shiftwidth=2:expandtab:cinoptions=(s,U1,m1
