/** 
 * @file llfloatermediasettings.cpp
 * @brief Tabbed dialog for media settings - class implementation
 *
 * $LicenseInfo:firstyear=2002&license=viewergpl$
 * 
 * Copyright (c) 2002-2010, Linden Research, Inc.
 * 
 * Second Life Viewer Source Code
 * The source code in this file ("Source Code") is provided by Linden Lab
 * to you under the terms of the GNU General Public License, version 2.0
 * ("GPL"), unless you have obtained a separate licensing agreement
 * ("Other License"), formally executed by you and Linden Lab.  Terms of
 * the GPL can be found in doc/GPL-license.txt in this distribution, or
 * online at http://secondlife.com/developers/opensource/gplv2
 * 
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at
 * http://secondlife.com/developers/opensource/flossexception
 * 
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 * $/LicenseInfo$
 * 
 */

#include "llviewerprecompiledheaders.h"

// #include "llfloaterreg.h"
#include "llfloatermediasettings.h"
#include "llpanelmediasettingsgeneral.h"
#include "llpanelmediasettingssecurity.h"
#include "llpanelmediasettingspermissions.h"
#include "llviewercontrol.h"
#include "lluictrlfactory.h"
#include "llbutton.h"
#include "llselectmgr.h"
#include "llsdutil.h"

////////////////////////////////////////////////////////////////////////////////
// 
LLFloaterMediaSettings::LLFloaterMediaSettings(const LLSD& key)
	: LLFloater(std::string("media_settings")),
	mTabContainer(NULL),
	mPanelMediaSettingsGeneral(NULL),
	mPanelMediaSettingsSecurity(NULL),
	mPanelMediaSettingsPermissions(NULL),
	mWaitingToClose( false ),
	mIdenticalHasMediaInfo( true ),
	mMultipleMedia(false),
	mMultipleValidMedia(false)
{
	LLUICtrlFactory::getInstance()->buildFloater(this, "floater_media_settings.xml");
}

////////////////////////////////////////////////////////////////////////////////
//
LLFloaterMediaSettings::~LLFloaterMediaSettings()
{
	if ( mPanelMediaSettingsGeneral )
	{
		delete mPanelMediaSettingsGeneral;
		mPanelMediaSettingsGeneral = NULL;
	}

	if ( mPanelMediaSettingsSecurity )
	{
		delete mPanelMediaSettingsSecurity;
		mPanelMediaSettingsSecurity = NULL;
	}

	if ( mPanelMediaSettingsPermissions )
	{
		delete mPanelMediaSettingsPermissions;
		mPanelMediaSettingsPermissions = NULL;
	}

}

////////////////////////////////////////////////////////////////////////////////
//
BOOL LLFloaterMediaSettings::postBuild()
{
	mApplyBtn = getChild<LLButton>("Apply");
	mApplyBtn->setClickedCallback(onBtnApply, this);
		
	mCancelBtn = getChild<LLButton>("Cancel");
	mCancelBtn->setClickedCallback(onBtnCancel, this);

	mOKBtn = getChild<LLButton>("OK");
	mOKBtn->setClickedCallback(onBtnOK, this);
			
	mTabContainer = getChild<LLTabContainer>( "tab_container" );
	
	mPanelMediaSettingsGeneral = new LLPanelMediaSettingsGeneral();
	mTabContainer->addTabPanel(mPanelMediaSettingsGeneral,mPanelMediaSettingsGeneral->getLabel(), FALSE, onTabChanged, mTabContainer);
	mPanelMediaSettingsGeneral->setParent( this );

	// note that "permissions" tab is really "Controls" tab - refs to 'perms' and
	// 'permissions' not changed to 'controls' since we don't want to change 
	// shared files in server code and keeping everything the same seemed best.
	mPanelMediaSettingsPermissions = new LLPanelMediaSettingsPermissions();
	mTabContainer->addTabPanel(mPanelMediaSettingsPermissions,mPanelMediaSettingsPermissions->getLabel(), FALSE, onTabChanged, mTabContainer);

	mPanelMediaSettingsSecurity = new LLPanelMediaSettingsSecurity();
	mTabContainer->addTabPanel(mPanelMediaSettingsSecurity,mPanelMediaSettingsSecurity->getLabel(), FALSE, onTabChanged, mTabContainer);
	mPanelMediaSettingsSecurity->setParent( this );
		
	// restore the last tab viewed from persistance variable storage
	if (!mTabContainer->selectTab(gSavedSettings.getS32("LastMediaSettingsTab")))
	//Imprudence: *CONSIDER: rather a per-account setting, isn't it? On the other hand
	//all that tabbing seems bloat since you want to control all settings when setting up a MOAP -- AW
	{
		mTabContainer->selectFirstTab();
	};

	return TRUE;
}

//static 
void LLFloaterMediaSettings::apply()
{
	LLFloaterMediaSettings* self = LLFloaterMediaSettings::getInstance();
	if (self->haveValuesChanged())
	{
		LLSD settings;
		self->mPanelMediaSettingsGeneral->preApply();
		self->mPanelMediaSettingsGeneral->getValues( settings, false );
		self->mPanelMediaSettingsSecurity->preApply();
		self->mPanelMediaSettingsSecurity->getValues( settings, false );
		self->mPanelMediaSettingsPermissions->preApply();
		self->mPanelMediaSettingsPermissions->getValues( settings, false );
			
		LLSelectMgr::getInstance()->selectionSetMedia( LLTextureEntry::MF_HAS_MEDIA, settings );

		self->mPanelMediaSettingsGeneral->postApply();
		self->mPanelMediaSettingsSecurity->postApply();
		self->mPanelMediaSettingsPermissions->postApply();
	}
}

////////////////////////////////////////////////////////////////////////////////
void LLFloaterMediaSettings::onClose(bool app_quitting)
{
	if(mPanelMediaSettingsGeneral)
	{
		mPanelMediaSettingsGeneral->onClose(app_quitting);
	}
	LLFloater::setVisible(FALSE);
}

////////////////////////////////////////////////////////////////////////////////
//static 
void LLFloaterMediaSettings::initValues( const LLSD& media_settings, bool editable )
{
	LLFloaterMediaSettings* self = LLFloaterMediaSettings::getInstance();
	if (self->hasFocus()) return;
	
	self->clearValues(editable);
	// update all panels with values from simulator
	self->mPanelMediaSettingsGeneral->
		initValues( self->mPanelMediaSettingsGeneral, media_settings, editable );

	self->mPanelMediaSettingsSecurity->
		initValues( self->mPanelMediaSettingsSecurity, media_settings, editable );

	self->mPanelMediaSettingsPermissions->
		initValues( self->mPanelMediaSettingsPermissions, media_settings, editable );
	
	// Squirrel away initial values 
	self->mInitialValues.clear();
	self->mPanelMediaSettingsGeneral->getValues( self->mInitialValues );
	self->mPanelMediaSettingsSecurity->getValues( self->mInitialValues );
	self->mPanelMediaSettingsPermissions->getValues( self->mInitialValues );
	
	self->mApplyBtn->setEnabled(editable);
	self->mOKBtn->setEnabled(editable);
}

////////////////////////////////////////////////////////////////////////////////
// 
void LLFloaterMediaSettings::commitFields()
{
	if (hasFocus())
	{
		LLUICtrl* cur_focus = dynamic_cast<LLUICtrl*>(gFocusMgr.getKeyboardFocus());
		if (cur_focus && cur_focus->acceptsTextInput())
		{
			cur_focus->onCommit();
		};
	};
}

////////////////////////////////////////////////////////////////////////////////
//static 
void LLFloaterMediaSettings::clearValues( bool editable)
{
	LLFloaterMediaSettings* self = LLFloaterMediaSettings::getInstance();
	// clean up all panels before updating
	self->mPanelMediaSettingsGeneral	 ->clearValues(self->mPanelMediaSettingsGeneral,  editable);
	self->mPanelMediaSettingsSecurity	 ->clearValues(self->mPanelMediaSettingsSecurity,	editable);
	self->mPanelMediaSettingsPermissions->clearValues(self->mPanelMediaSettingsPermissions,  editable);	
}

////////////////////////////////////////////////////////////////////////////////
// static
void LLFloaterMediaSettings::onBtnOK( void* userdata )
{
	LLFloaterMediaSettings* self = (LLFloaterMediaSettings*)userdata;

	self->commitFields();
	self->apply();
	self->close();
}

////////////////////////////////////////////////////////////////////////////////
// static
void LLFloaterMediaSettings::onBtnApply( void* userdata )
{
	LLFloaterMediaSettings* self = (LLFloaterMediaSettings*)userdata;

	self->commitFields();
	self->apply();

	self->mInitialValues.clear();
	self->mPanelMediaSettingsGeneral->getValues( self->mInitialValues );
	self->mPanelMediaSettingsSecurity->getValues( self->mInitialValues );
	self->mPanelMediaSettingsPermissions->getValues( self->mInitialValues );

}

////////////////////////////////////////////////////////////////////////////////
// static
void LLFloaterMediaSettings::onBtnCancel( void* userdata )
{
	LLFloaterMediaSettings* self = (LLFloaterMediaSettings*)userdata;

	self->close(); 
}

////////////////////////////////////////////////////////////////////////////////
// static
void LLFloaterMediaSettings::onTabChanged(void* user_data, bool from_click)
{
	LLTabContainer* self = (LLTabContainer*)user_data;
	gSavedSettings.setS32("LastMediaSettingsTab", self->getCurrentPanelIndex());
}
////////////////////////////////////////////////////////////////////////////////
//
const std::string LLFloaterMediaSettings::getHomeUrl()
{
	if ( mPanelMediaSettingsGeneral )
		return mPanelMediaSettingsGeneral->getHomeUrl();
	else
		return std::string( "" );
}

////////////////////////////////////////////////////////////////////////////////
// virtual 
void LLFloaterMediaSettings::draw()
{
	if (NULL != mApplyBtn)
	{
		// Set the enabled state of the "Apply" button if values changed
		mApplyBtn->setEnabled( haveValuesChanged() );
	}
	
	LLFloater::draw();
}


//private
bool LLFloaterMediaSettings::haveValuesChanged() const
{
	LLFloaterMediaSettings* self = LLFloaterMediaSettings::getInstance();

	bool values_changed = false;
	// *NOTE: The code below is very inefficient.  Better to do this
	// only when data change.
	// Every frame, check to see what the values are.  If they are not
	// the same as the initial media data, enable the OK/Apply buttons
	LLSD settings;
	self->mPanelMediaSettingsGeneral->getValues( settings );
	self->mPanelMediaSettingsSecurity->getValues( settings );
	self->mPanelMediaSettingsPermissions->getValues( settings );	
	LLSD::map_const_iterator iter = settings.beginMap();
	LLSD::map_const_iterator end = settings.endMap();
	for ( ; iter != end; ++iter )
	{
		const std::string &current_key = iter->first;
		const LLSD &current_value = iter->second;
 		if ( ! llsd_equals(current_value, mInitialValues[current_key]))
		{
			values_changed = true;
			break;
		}
	}
	return values_changed;
}

