/* This file is part of FlashFolder. 
 * Copyright (C) 2007 zett42 ( zett42 at users.sourceforge.net ) 
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#pragma once

struct FavoritesItem
{
	tstring command;      ///< file path or TC command (required)
	tstring title;        ///< menu item title
	tstring targetpath;   ///< unused, round-trip data from TC
};

typedef std::vector<FavoritesItem> FavoritesList;

enum DirFavoritesSrc
{
	DFS_DEFAULT,
	DFS_FLASHFOLDER,
	DFS_TOTALCMD
};

void GetDirFavorites( FavoritesList* pList, DirFavoritesSrc source = DFS_DEFAULT );
void SetDirFavorites( const FavoritesList& list, DirFavoritesSrc source = DFS_DEFAULT );

int GetFavItemByPath( const FavoritesList& favs, LPCTSTR pPath, LPCTSTR pTargetPath = NULL );