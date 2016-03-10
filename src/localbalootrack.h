/*
 * Copyright 2016 Matthieu Gallien <matthieu_gallien@yahoo.fr>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef LOCALBALOOTRACK_H
#define LOCALBALOOTRACK_H

#include <QtCore/QList>
#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtCore/QMetaType>

class LocalBalooTrack
{
public:
    LocalBalooTrack();

    QString mArtist;

    QString mTitle;

    QString mAlbum;

    double mDuration = 0;

    QUrl mFile;

};

Q_DECLARE_METATYPE(LocalBalooTrack)

#endif // LOCALBALOOTRACK_H
