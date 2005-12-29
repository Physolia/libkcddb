/*
  Copyright (C) 2005 Richard Lärkäng <nouseforaname@home.se>

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
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/


#include "musicbrainzlookup.h"
extern "C" {
#include "sha1.h"
//#include "base64.h"
}
#include <kdebug.h>
#include <kcodecs.h>
#include <cstdio>
#include <cstring>
#include <musicbrainz/musicbrainz.h>

namespace KCDDB
{
  MusicBrainzLookup::MusicBrainzLookup()
  {

  }

  MusicBrainzLookup::~MusicBrainzLookup()
  {

  }
  
  CDDB::Result MusicBrainzLookup::lookup( const QString &, uint, const TrackOffsetList & trackOffsetList )
  {
    QString discId = calculateDiscId(trackOffsetList);
    
    kdDebug() << "Should lookup " << discId << endl;

    ::MusicBrainz mb;

    mb.UseUTF8(true);
    mb.SetDepth(4);

    vector<string> args;
    args.insert(args.begin(), discId.toLatin1().data());

    if (!mb.Query(MBQ_GetCDInfoFromCDIndexId, &args))
    {
      string error;
      
      mb.GetQueryError(error);
      kdDebug() << "Query failed: " << error.c_str() << endl;
      
      return UnknownError;
    }

    if (mb.DataInt(MBE_GetNumAlbums) < 1)
    {
      kdDebug() << "No CD Found" << endl;

      return UnknownError;
    }

    mb.Select(MBS_SelectAlbum, 1);

    CDInfo info;

    info.set("title", QString::fromUtf8(mb.Data(MBE_AlbumGetAlbumName).c_str()));
    // FIXME
    info.set("artist", QString::fromUtf8(mb.Data(MBE_AlbumGetArtistName, 1).c_str()));

    int numTracks = trackOffsetList.count()-1;

    for (int i=1; i <= numTracks; i++)
    {
      TrackInfo track;
      track.set("artist", QString::fromUtf8(mb.Data(MBE_AlbumGetArtistName, i).c_str()));
      track.set("title", QString::fromUtf8(mb.Data(MBE_AlbumGetTrackName, i).c_str()));

      info.trackInfoList << track;
    }

    kdDebug() << "Query succeeded :-)" << endl;

    cdInfoList_ << info;

    return Success;
  }
  
  QString MusicBrainzLookup::calculateDiscId(const TrackOffsetList & trackOffsetList )
  {
    // Code based on libmusicbrainz/lib/diskid.cpp
    
    int numTracks = trackOffsetList.count()-1;

    SHA_INFO       sha;
    unsigned char  digest[20];
    char           temp[9];

    sha_init(&sha);

    // FIXME How do I check that?
    int firstTrack = 1;
    int lastTrack = numTracks;

    sprintf(temp, "%02X", firstTrack);
    sha_update(&sha, (unsigned char *)temp, strlen(temp));

    sprintf(temp, "%02X", lastTrack);
    sha_update(&sha, (unsigned char *)temp, strlen(temp));

    for(int i = 0; i < 100; i++)
    {
      long offset;
      if (i == 0)
        offset = trackOffsetList[numTracks];
      else if (i <= numTracks)
        offset = trackOffsetList[i-1];
      else
        offset = 0;

      sprintf(temp, "%08lX", offset);
      sha_update(&sha, (unsigned char *)temp, strlen(temp));
    }
    
    sha_final(digest, &sha);

    QByteArray base64 = KCodecs::base64Encode(QByteArray((const char *)digest, 20));

    // '/' '+' and '=' replaced for MusicBrainz
    QString res = QString::fromLatin1(base64).replace('/',"_").replace('+',".").replace('=',"-");
    
    //free(base64);

    return res;
  }
}

// vim:tabstop=2:shiftwidth=2:expandtab:cinoptions=(s,U1,m1

#include "musicbrainzlookup.moc"
