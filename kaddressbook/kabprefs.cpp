/*                                                                      
    This file is part of KAddressBook.                                  
    Copyright (c) 2002 Mike Pilone <mpilone@slac.com>                   
                                                                        
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or   
    (at your option) any later version.                                 
                                                                        
    This program is distributed in the hope that it will be useful,     
    but WITHOUT ANY WARRANTY; without even the implied warranty of      
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the        
    GNU General Public License for more details.                        
                                                                        
    You should have received a copy of the GNU General Public License   
    along with this program; if not, write to the Free Software         
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.           
                                                                        
    As a special exception, permission is given to link this program    
    with any edition of Qt, and distribute the resulting executable,    
    without including the source code for Qt in the source distribution.
*/                                                                      

#include <kconfig.h>
#include <klocale.h>
#include <kstaticdeleter.h>

#include "kabprefs.h"

KABPrefs *KABPrefs::sInstance = 0;
static KStaticDeleter<KABPrefs> staticDeleter;

KABPrefs::KABPrefs()
  : KPimPrefs("kaddressbookrc")
{
  KConfigSkeleton::setCurrentGroup( "Views" );
  addItemBool( "HonorSingleClick", mHonorSingleClick, false );

  KConfigSkeleton::setCurrentGroup( "General" );
  addItemBool( "AutomaticNameParsing", mAutomaticNameParsing, true );
  addItemInt( "CurrentIncSearchField", mCurrentIncSearchField, 0 );
  addItemString( "PhoneHookApplication", mPhoneHookApplication, "" );
  addItemString( "FaxHookApplication", mFaxHookApplication,
                 "kdeprintfax --phone %N" );

  QStringList defaultMaps;
  defaultMaps << "http://link2.map24.com/?lid=9cc343ae&maptype=CGI&lang=%1&street0=%s&zip0=%z&city0=%l&country0=%c";
  defaultMaps << "http://www.mapquest.com/main.adp?searchtab=address&searchtype=address&country=%c&address=%s&state=%r&zipcode=%z&city=%l&search=1";
  addItemString( "LocationMapURL", mLocationMapURL, defaultMaps[ 0 ] );
  addItemStringList( "LocationMapURLs", mLocationMapURLs, defaultMaps );

  KConfigSkeleton::setCurrentGroup( "MainWindow" );
  addItemBool( "JumpButtonBarVisible", mJumpButtonBarVisible, false );
  addItemBool( "DetailsPageVisible", mDetailsPageVisible, true );
  addItemIntList( "ExtensionsSplitter", mExtensionsSplitter );
  addItemIntList( "DetailsSplitter", mDetailsSplitter );

  KConfigSkeleton::setCurrentGroup( "Extensions_General" );
  QStringList defaultExtensions;
  defaultExtensions << "distribution_list_editor";
  addItemInt( "CurrentExtension", mCurrentExtension, 0 );
  addItemStringList( "ActiveExtensions", mActiveExtensions, defaultExtensions );

  KConfigSkeleton::setCurrentGroup( "Views" );
  QString defaultView = i18n( "Default Table View" );
  addItemString( "CurrentView", mCurrentView, defaultView );
  addItemStringList( "ViewNames", mViewNames, defaultView );

  KConfigSkeleton::setCurrentGroup( "Filters" );
  addItemInt( "CurrentFilter", mCurrentFilter, 0 );

  KConfigSkeleton::setCurrentGroup( "AddresseeEditor" );
  addItemInt( "EditorType", mEditorType, 0 );
  addItemStringList( "GlobalCustomFields", mGlobalCustomFields );
  addItemStringList( "AdvancedCustomFields", mAdvancedCustomFields );
}

KABPrefs::~KABPrefs()
{
}

KABPrefs *KABPrefs::instance()
{
  if ( !sInstance ) {
    staticDeleter.setObject( sInstance, new KABPrefs() );
    sInstance->readConfig();
  }

  return sInstance;
}

void KABPrefs::setCategoryDefaults()
{
  mCustomCategories.clear();
  
  mCustomCategories << i18n( "Business" ) << i18n( "Family" ) << i18n( "School" )
                    << i18n( "Customer" ) << i18n( "Friend" );
}
