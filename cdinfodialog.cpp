/*
  Copyright (c) 2005 Shaheedur R. Haque <srhaque@iee.org>
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

#include "cdinfodialog.h"

#include "cdinfoencodingwidget.h"

#include <qtextcodec.h>
#include <kdebug.h>
#include <QDateTime>
#include <kglobal.h>
#include <kcharsets.h>

using KCDDB::TrackInfo;

namespace KCDDB
{
  const char *CDInfoDialog::SEPARATOR = " / ";

  CDInfoDialog::CDInfoDialog(QWidget* parent)
    : KDialog(parent), Ui::CDInfoDialogBase()
  {
      QWidget* w = new QWidget(this);
      setupUi(w);
      setMainWidget(w);

      m_info.set("source", "user");

      m_categories = KCDDB::Categories();
      m_category->addItems(m_categories.i18nList());
      m_genres = KCDDB::Genres();
      m_genre->addItems(m_genres.i18nList());

      m_trackList->addColumn(i18n("Track"));
      m_trackList->addColumn(i18n("Length"));
      m_trackList->addColumn(i18n("Title"));
      m_trackList->addColumn(i18n("Comment"));
      m_trackList->addColumn(i18n("Artist"));

      // We want control over the visibility of this column. See artistChanged().
      m_trackList->setColumnWidthMode(TRACK_ARTIST, Q3ListView::Manual);

      // Make the user-definable values in-place editable.
      m_trackList->setRenameable(TRACK_NUMBER, false);
      m_trackList->setRenameable(TRACK_TIME, false);
      m_trackList->setRenameable(TRACK_TITLE, true);
      m_trackList->setRenameable(TRACK_COMMENT, true);
      m_trackList->setRenameable(TRACK_ARTIST, true);

      // ensure we get our translations
      KGlobal::locale()->insertCatalog("libkcddb");
      connect( m_trackList, SIGNAL( selectionChanged(Q3ListViewItem*) ), this, SLOT( slotTrackSelected(Q3ListViewItem*) ) );
      connect( m_trackList, SIGNAL( doubleClicked(Q3ListViewItem*,const QPoint&,int) ), this, SLOT( slotTrackDoubleClicked(Q3ListViewItem*,const QPoint&,int) ) );
      connect( m_artist, SIGNAL( textChanged(const QString&) ), this, SLOT( artistChanged(const QString&) ) );
      connect( m_genre, SIGNAL( textChanged(const QString&) ), this, SLOT( genreChanged(const QString&) ) );
      connect( m_multiple, SIGNAL( toggled(bool) ), this, SLOT( slotMultipleArtists(bool) ) );

      connect(m_changeEncoding,SIGNAL(clicked()),SLOT(slotChangeEncoding()));
  }

  void CDInfoDialog::slotTrackSelected( Q3ListViewItem *item )
  {
      emit play(item->text(0).toUInt()-1);
  }

  void CDInfoDialog::slotNextTrack()
  {
      if (m_trackList->currentItem())
      {
          Q3ListViewItem *item = m_trackList->currentItem()->nextSibling();
          m_trackList->setSelected(item, true);
          m_trackList->ensureItemVisible(item);
      }
  }

  void CDInfoDialog::slotTrackDoubleClicked( Q3ListViewItem *item, const QPoint &, int column)
  {
      m_trackList->rename(item, column);
  }

  void CDInfoDialog::setInfo( const KCDDB::CDInfo &info, const KCDDB::TrackOffsetList &trackStartFrames )
  {
      m_info = info;

      m_artist->setText(info.get(Artist).toString().trimmed());
      m_title->setText(info.get(Title).toString().trimmed());
      m_category->setItemText(m_category->currentIndex(), m_categories.cddb2i18n(info.get(Category).toString()));

      // Make sure the revision is set before the genre to allow the genreChanged() handler to fire.
      m_revision->setText(QString::number(info.get("revision").toInt()));
      m_genre->setItemText(m_genre->currentIndex(), m_genres.cddb2i18n(info.get(Genre).toString()));
      m_year->setValue(info.get(Year).toInt());
      m_comment->setText(info.get(Comment).toString().trimmed());
      m_id->setText(info.get("discid").toString().trimmed());

      // Now do the individual tracks.
      unsigned tracks = info.numberOfTracks();
      if (tracks > 0)
      {
         m_length->setText(framesTime(trackStartFrames[tracks] - trackStartFrames[0]));
      }

      m_trackList->clear();
      for (unsigned i = 0; i < tracks; i++)
      {
          Q3ListViewItem *item = new Q3ListViewItem(m_trackList, 0);

          TrackInfo ti(info.track(i));

          item->setText(TRACK_NUMBER, QString().sprintf("%02d", i + 1));
          item->setText(TRACK_TIME, framesTime(trackStartFrames[i + 1] - trackStartFrames[i]));
          item->setText(TRACK_ARTIST, ti.get(Artist).toString());
          item->setText(TRACK_TITLE, ti.get(Title).toString());
          item->setText(TRACK_COMMENT, ti.get(Comment).toString());
      }
      // FIXME KDE4: handle playorder here too, once KCDDBInfo::CDInfo is updated.

      if (info.get(Artist).toString() == "Various" || m_multiple->isChecked()){
          m_trackList->adjustColumn(TRACK_ARTIST);
    }
  }

  QString CDInfoDialog::framesTime(unsigned frames)
  {
      QTime time;
      double ms;

      ms = frames * 1000 / 75.0;
      time = time.addMSecs((int)ms);

      // Use ".zzz" for milliseconds...
      QString temp2;
      if (time.hour() > 0)
          temp2 = time.toString("hh:mm:ss");
      else
          temp2 = time.toString("mm:ss");
      return temp2;
  } // framesTime

  KCDDB::CDInfo CDInfoDialog::info() const
  {
      KCDDB::CDInfo info = m_info;

      info.set(Artist, m_artist->text().trimmed());
      info.set(Title, m_title->text().trimmed());
      info.set(Category, m_categories.i18n2cddb(m_category->currentText()));
      info.set(Genre, m_genres.i18n2cddb(m_genre->currentText()));
      info.set(Year, m_year->value());
      info.set(Comment, m_comment->text().trimmed());
      info.set("revision", m_revision->text().trimmed().toUInt());
      info.set("discid", m_id->text().trimmed());
      int i=0;
      for (Q3ListViewItem *item = m_trackList->firstChild(); item; item=item->nextSibling())
      {
          TrackInfo& track = info.track(i);
          track.set(Artist,item->text(TRACK_ARTIST).trimmed());
          track.set(Title,item->text(TRACK_TITLE).trimmed());
          track.set(Comment,item->text(TRACK_COMMENT).trimmed());
          i++;
          // FIXME KDE4: handle track lengths here too, once KCDDBInfo::CDInfo is updated.
      }
      // FIXME KDE4: handle playorder here too, once KCDDBInfo::CDInfo is updated.
      return info;
  }


  void CDInfoDialog::artistChanged( const QString &newArtist )
  {
      // Enable special handling of compilations.
      if (newArtist.trimmed().compare("Various")) {
          m_multiple->setChecked(false);
      } else {
          m_multiple->setChecked(true);
      }
  }

  void CDInfoDialog::genreChanged( const QString &newGenre )
  {
      // Disable changes to category if the version number indicates that a record
      // is already in the database, or if the genre is poorly set. The latter
      // condition also provides a "back-door" override.
      m_category->setEnabled((m_revision->text().trimmed().toUInt() < 1) ||
                              (newGenre.compare("Unknown") == 0));
  }


  void CDInfoDialog::slotMultipleArtists( bool hasMultipleArtist)
  {
      if(hasMultipleArtist){
          for (Q3ListViewItem *item = m_trackList->firstChild(); item; item=item->nextSibling())
          {
              QString title = item->text(TRACK_TITLE);
              int separator = title.indexOf(SEPARATOR);
              if (separator != -1)
              {
                  // Artists probably entered already
                  item->setText(TRACK_ARTIST, title.left(separator));
                  item->setText(TRACK_TITLE, title.mid(separator + 3));
              }
          }
          m_trackList->adjustColumn(TRACK_ARTIST);
          m_trackList->adjustColumn(TRACK_TITLE);
      }
      else{
          for (Q3ListViewItem *item = m_trackList->firstChild(); item; item=item->nextSibling())
          {
              QString artist = item->text(TRACK_ARTIST);
              if (!artist.isEmpty())
              {
                  item->setText(TRACK_ARTIST, QString::null);
                  item->setText(TRACK_TITLE, artist + SEPARATOR + item->text(TRACK_TITLE));
              }
          }
          m_trackList->hideColumn(TRACK_ARTIST);
          m_trackList->adjustColumn(TRACK_TITLE);
      }
  }


  void CDInfoDialog::slotChangeEncoding()
  {
      kDebug() << k_funcinfo << endl;

      KDialog* dialog = new KDialog(this);
      dialog->setCaption(i18n("Change Encoding"));
      dialog->setButtons( KDialog::Ok | KDialog::Cancel);
      dialog->setModal( true );


      QStringList songTitles;
      for (Q3ListViewItem *item = m_trackList->firstChild(); item; item=item->nextSibling())
      {
          QString title = item->text(TRACK_ARTIST).trimmed();
          if (!title.isEmpty())
              title.append(SEPARATOR);
          title.append(item->text(TRACK_TITLE).trimmed());
          songTitles << title;
      }

      KCDDB::CDInfoEncodingWidget* encWidget = new KCDDB::CDInfoEncodingWidget(
          dialog, m_artist->text(),m_title->text(), songTitles);

      dialog->setMainWidget(encWidget);

      if (dialog->exec())
      {
        KCharsets* charsets = KGlobal::charsets();
        QTextCodec* codec = charsets->codecForName(charsets->encodingForName(encWidget->selectedEncoding()));

        m_artist->setText(codec->toUnicode(m_artist->text().toLatin1()));
        m_title->setText(codec->toUnicode(m_title->text().toLatin1()));
        m_genre->setItemText(m_genre->currentIndex(), codec->toUnicode(m_genre->currentText().toLatin1()));
        m_comment->setText(codec->toUnicode(m_comment->text().toLatin1()));

        for (Q3ListViewItem *item = m_trackList->firstChild(); item; item=item->nextSibling())
        {
            item->setText(TRACK_ARTIST,codec->toUnicode(item->text(TRACK_ARTIST).toLatin1()));
            item->setText(TRACK_TITLE,codec->toUnicode(item->text(TRACK_TITLE).toLatin1()));
            item->setText(TRACK_COMMENT,codec->toUnicode(item->text(TRACK_COMMENT).toLatin1()));
        }
      }
  }
}

#include "cdinfodialog.moc"
