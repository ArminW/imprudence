/** 
 * @file llfloatermap.cpp
 * @brief The "mini-map" or radar in the upper right part of the screen.
 *
 * $LicenseInfo:firstyear=2001&license=viewergpl$
 * 
 * Copyright (c) 2001-2009, Linden Research, Inc.
 * 
 * Second Life Viewer Source Code
 * The source code in this file ("Source Code") is provided by Linden Lab
 * to you under the terms of the GNU General Public License, version 2.0
 * ("GPL"), unless you have obtained a separate licensing agreement
 * ("Other License"), formally executed by you and Linden Lab.  Terms of
 * the GPL can be found in doc/GPL-license.txt in this distribution, or
 * online at http://secondlifegrid.net/programs/open_source/licensing/gplv2
 * 
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at http://secondlifegrid.net/programs/open_source/licensing/flossexception
 * 
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 * $/LicenseInfo$
 */

#include "llviewerprecompiledheaders.h"

#include "llfloatermap.h"

#include "llagent.h"
#include "llcolorscheme.h"
#include "llviewercontrol.h"
#include "lldraghandle.h"
#include "llnetmap.h"
#include "llregionhandle.h"
#include "llresizebar.h"
#include "lluictrlfactory.h"
#include "llurldispatcher.h"
#include "llviewerregion.h"

LLFloaterMap::LLFloaterMap(const LLSD& key)
	:
	LLFloater(std::string("minimap")),
	mPanelMap(NULL)
{
	LLCallbackMap::map_t factory_map;
	factory_map["mini_mapview"] = LLCallbackMap(createPanelMiniMap, this);
	LLUICtrlFactory::getInstance()->buildFloater(this, "floater_mini_map.xml", &factory_map, FALSE);


	mSlurlEditor = getChild<LLLineEditor>("mini_map_slurl");
	if (mSlurlEditor)
	{
		mSlurlEditor -> setCallbackUserData(this);
		mSlurlEditor -> setCommitOnFocusLost( FALSE );
	}

	childSetAction("mini_map_go", onClickMiniMapGo, this);
	
}


// static
void* LLFloaterMap::createPanelMiniMap(void* data)
{
	LLFloaterMap* self = (LLFloaterMap*)data;
	self->mPanelMap = new LLNetMap("Mapview");
	return self->mPanelMap;
}

BOOL LLFloaterMap::postBuild()
{
	// Send the drag handle to the back, but make sure close stays on top
	sendChildToBack(getDragHandle());
	sendChildToFront(getChild<LLButton>("llfloater_close_btn"));
	setIsChrome(TRUE);
	return TRUE;
}


// virtual
BOOL LLFloaterMap::handleKeyHere( KEY key, MASK mask )
{
	BOOL handled = FALSE;

	if( KEY_RETURN == key )
	{
		if (mask == MASK_NONE)
		{
			if (mSlurlEditor)
			{
				std::string location_raw = mSlurlEditor->getText() ;
				miniMapGo(location_raw);
				
			}
			handled = TRUE;
		}
	}

	return handled;
}

LLFloaterMap::~LLFloaterMap()
{
}


// virtual 
void LLFloaterMap::onOpen()
{
	gFloaterView->adjustToFitScreen(this, FALSE);

	gSavedSettings.setBOOL("ShowMiniMap", TRUE);
}


// virtual
void LLFloaterMap::onClose(bool app_quitting)
{
	LLFloater::setVisible(FALSE);

	if (!app_quitting)
	{
		gSavedSettings.setBOOL("ShowMiniMap", FALSE);
	}
}

BOOL LLFloaterMap::canClose()
{
	return !LLApp::isExiting();
}
//static 
void LLFloaterMap::onClickMiniMapGo( void* userdata )
{
	LLFloaterMap* self = (LLFloaterMap*) userdata;
	std::string location_raw =  self->mSlurlEditor->getText();
	miniMapGo(location_raw);
}
//static 
void LLFloaterMap::miniMapGo(std::string location_raw)
{	
	LLStringUtil::trim(location_raw);
	if( !location_raw.empty() )
	{ 
 		LLViewerRegion* regionp = gAgent.getRegion();
		std::string location = location_raw;

		std::string::size_type idx = location_raw.find("://");
		if ( idx==std::string::npos)
		{
			idx = location_raw.find_first_of("0123456789");
			if (idx || idx==std::string::npos)
			{
				location = "sl://"+location_raw;
			}
			else
			{
				location = "sl://"+regionp->getName()+"/"+location_raw;
			}

		}

		if(regionp != NULL && LLURLDispatcher::isSLURL(location))
		{
			gAgent.teleportViaSLURL(location);
			gAgent.setControlFlags(AGENT_CONTROL_STAND_UP);//stand up
		}
		
	}
 

}

// virtual
void LLFloaterMap::draw()
{
	// Note: we can't just gAgent.check cameraMouselook() because the transition states are wrong.
	if( gAgent.cameraMouselook())
	{
		setMouseOpaque(FALSE);
		getDragHandle()->setMouseOpaque(FALSE);

		drawChild(mPanelMap);
	}
	else
	{
		setMouseOpaque(TRUE);
		getDragHandle()->setMouseOpaque(TRUE);

		LLFloater::draw();
	}
}

