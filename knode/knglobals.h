/***************************************************************************
                     knglobals.h - description
 copyright            : (C) 1999 by Christian Thurner
 email                : cthurner@freepage.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KNGLOBALS_H
#define KNGLOBALS_H

class KNNetAccess;
class KNodeApp;
class KNProgress;
class KNAccountManager;
class KNGroupManager;
class KNFetchArticleManager;
class KNFolderManager;
class KNSavedArticleManager;
class KNFilterManager;
class KNAppManager;


// idea: Previously the manager classes were available
//       via KNodeApp. Now they can be accessed directly,
//       this removes many header dependencies.
//       (knode.h isn't include everywhere)
class KNGlobals {

  public:

    QWidget               *topWidget;    // topWidget == top, used for message boxes,
    KNodeApp              *top;          // no need to include knode.h everywhere
    KNNetAccess           *netAccess;
    KNProgress            *progressBar;
    KNAccountManager      *accManager;
    KNGroupManager        *gManager;
    KNFetchArticleManager *fArtManager;
    KNFolderManager       *foManager;
    KNSavedArticleManager *sArtManager;
    KNFilterManager       *fiManager;
    KNAppManager          *appManager;

};


extern KNGlobals knGlobals;

#endif








