/** 
 * @file llfloatertools.cpp
 * @brief The edit tools, including move, position, land, etc.
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
 * online at http://secondlifegrid.net/programs/open_source/licensing/gplv2
 * 
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at
 * http://secondlifegrid.net/programs/open_source/licensing/flossexception
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

#include "llfloatertools.h"

#include "llfontgl.h"
#include "llcoord.h"
#include "llgl.h"

#include "llagent.h"
#include "llbutton.h"
#include "llcheckboxctrl.h"
#include "llcombobox.h"
#include "lldraghandle.h"
#include "llfloaterbuildoptions.h"
#include "llfloatermediasettings.h"
#include "llfloateropenobject.h"
#include "llfocusmgr.h"
#include "llmediactrl.h"
#include "llmediaentry.h"
#include "llmenugl.h"
#include "llnotify.h"
#include "llpanelcontents.h"
#include "llpanelface.h"
#include "llpanelland.h"
#include "llpanelinventory.h"
#include "llpanelobject.h"
#include "llpanelvolume.h"
#include "llpanelpermissions.h"
#include "llresmgr.h"
#include "llselectmgr.h"
#include "llslider.h"
#include "llspinctrl.h"
#include "llstatusbar.h"
#include "lltabcontainer.h"
#include "lltextbox.h"
#include "lltoolbrush.h"
#include "lltoolcomp.h"
#include "lltooldraganddrop.h"
#include "lltoolface.h"
#include "lltoolfocus.h"
#include "lltoolgrab.h"
#include "lltoolgrab.h"
#include "lltoolindividual.h"
#include "lltoolmgr.h"
#include "lltoolpie.h"
#include "lltoolpipette.h"
#include "lltoolplacer.h"
#include "lltoolselectland.h"
#include "lltrans.h"
#include "llui.h"
#include "llviewermenu.h"
#include "llviewerparcelmgr.h"
#include "llviewerregion.h"
#include "llviewerwindow.h"
#include "llviewercontrol.h"
#include "llviewerjoystick.h"
#include "llvograss.h"
#include "llvotree.h"
#include "llvovolume.h"
#include "lluictrlfactory.h"

#include "hippoLimits.h"

// Globals
LLFloaterTools *gFloaterTools = NULL;


const std::string PANEL_NAMES[LLFloaterTools::PANEL_COUNT] =
{
	std::string("General"), 	// PANEL_GENERAL,
	std::string("Object"), 	// PANEL_OBJECT,
	std::string("Features"),	// PANEL_FEATURES,
	std::string("Texture"),	// PANEL_FACE,
	std::string("Content"),	// PANEL_CONTENTS,
};

// Local prototypes
void commit_select_tool(LLUICtrl *ctrl, void *data);
void commit_select_component(LLUICtrl *ctrl, void *data);
void click_show_more(void*);
void click_popup_info(void*);
void click_popup_done(void*);
void click_popup_minimize(void*);
void click_popup_grab_drag(LLUICtrl *, void*);
void click_popup_grab_lift(LLUICtrl *, void*);
void click_popup_grab_spin(LLUICtrl *, void*);
void click_popup_rotate_left(void*);
void click_popup_rotate_reset(void*);
void click_popup_rotate_right(void*);
void click_popup_dozer_mode(LLUICtrl *, void *user);
void commit_slider_dozer_size(LLUICtrl *, void*);
void commit_slider_dozer_force(LLUICtrl *, void*);
void click_apply_to_selection(void*);
void commit_radio_zoom(LLUICtrl *, void*);
void commit_radio_orbit(LLUICtrl *, void*);
void commit_radio_pan(LLUICtrl *, void*);
void commit_grid_mode(LLUICtrl *, void*);
void commit_slider_zoom(LLUICtrl *, void*);


//static
void*	LLFloaterTools::createPanelPermissions(void* data)
{
	LLFloaterTools* floater = (LLFloaterTools*)data;
	floater->mPanelPermissions = new LLPanelPermissions("General");
	return floater->mPanelPermissions;
}
//static
void*	LLFloaterTools::createPanelObject(void* data)
{
	LLFloaterTools* floater = (LLFloaterTools*)data;
	floater->mPanelObject = new LLPanelObject("Object");
	return floater->mPanelObject;
}

//static
void*	LLFloaterTools::createPanelVolume(void* data)
{
	LLFloaterTools* floater = (LLFloaterTools*)data;
	floater->mPanelVolume = new LLPanelVolume("Features");
	return floater->mPanelVolume;
}

//static
void*	LLFloaterTools::createPanelFace(void* data)
{
	LLFloaterTools* floater = (LLFloaterTools*)data;
	floater->mPanelFace = new LLPanelFace("Texture");
	return floater->mPanelFace;
}

//static
void*	LLFloaterTools::createPanelContents(void* data)
{
	LLFloaterTools* floater = (LLFloaterTools*)data;
	floater->mPanelContents = new LLPanelContents("Contents");
	return floater->mPanelContents;
}

//static
void*	LLFloaterTools::createPanelContentsInventory(void* data)
{
	LLFloaterTools* floater = (LLFloaterTools*)data;
	floater->mPanelContents->mPanelInventory = new LLPanelInventory(std::string("ContentsInventory"), LLRect());
	return floater->mPanelContents->mPanelInventory;
}

//static
void*	LLFloaterTools::createPanelLandInfo(void* data)
{
	LLFloaterTools* floater = (LLFloaterTools*)data;
	floater->mPanelLandInfo = new LLPanelLandInfo(std::string("land info panel"));
	return floater->mPanelLandInfo;
}

void LLFloaterTools::updateToolsSizeLimits()
{
	if (gSavedSettings.getBOOL("DisableMaxBuildConstraints"))
	{
		getChild<LLSpinCtrl>("Scale X")->setMaxValue(F32_MAX);
		getChild<LLSpinCtrl>("Scale Y")->setMaxValue(F32_MAX);
		getChild<LLSpinCtrl>("Scale Z")->setMaxValue(F32_MAX);
	}
	else
	{
		getChild<LLSpinCtrl>("Scale X")->setMaxValue(gHippoLimits->getMaxPrimScale());
		getChild<LLSpinCtrl>("Scale Y")->setMaxValue(gHippoLimits->getMaxPrimScale());
		getChild<LLSpinCtrl>("Scale Z")->setMaxValue(gHippoLimits->getMaxPrimScale());

		getChild<LLSpinCtrl>("Scale X")->setMinValue(gHippoLimits->getMinPrimScale());
		getChild<LLSpinCtrl>("Scale Y")->setMinValue(gHippoLimits->getMinPrimScale());
		getChild<LLSpinCtrl>("Scale Z")->setMinValue(gHippoLimits->getMinPrimScale());
	}
}

void LLFloaterTools::updateToolsPrecision()
{
	U32 decimals = gSavedSettings.getU32("DecimalsForTools");
	if (decimals != mPrecision)
	{
		if (decimals > 5)
		{
			decimals = 5;
		}
		getChild<LLSpinCtrl>("Pos X")->setPrecision(decimals);
		getChild<LLSpinCtrl>("Pos Y")->setPrecision(decimals);
		getChild<LLSpinCtrl>("Pos Z")->setPrecision(decimals);
		getChild<LLSpinCtrl>("Scale X")->setPrecision(decimals);
		getChild<LLSpinCtrl>("Scale Y")->setPrecision(decimals);
		getChild<LLSpinCtrl>("Scale Z")->setPrecision(decimals);
		getChild<LLSpinCtrl>("Rot X")->setPrecision(decimals);
		getChild<LLSpinCtrl>("Rot Y")->setPrecision(decimals);
		getChild<LLSpinCtrl>("Rot Z")->setPrecision(decimals);
		mPrecision = decimals;
	}
}

BOOL	LLFloaterTools::postBuild()
{
	
	// Hide until tool selected
	setVisible(FALSE);

	// Since we constantly show and hide this during drags, don't
	// make sounds on visibility changes.
	setSoundFlags(LLView::SILENT);

	getDragHandle()->setEnabled( !gSavedSettings.getBOOL("ToolboxAutoMove") );

	LLRect rect;
	mBtnFocus = getChild<LLButton>("button focus");//btn;
	childSetAction("button focus",LLFloaterTools::setEditTool, (void*)LLToolCamera::getInstance());
	mBtnMove = getChild<LLButton>("button move");
	childSetAction("button move",LLFloaterTools::setEditTool, (void*)LLToolGrab::getInstance());
	mBtnEdit = getChild<LLButton>("button edit");
	childSetAction("button edit",LLFloaterTools::setEditTool, (void*)LLToolCompTranslate::getInstance());
	mBtnCreate = getChild<LLButton>("button create");
	childSetAction("button create",LLFloaterTools::setEditTool, (void*)LLToolCompCreate::getInstance());
	mBtnLand = getChild<LLButton>("button land" );
	childSetAction("button land",LLFloaterTools::setEditTool, (void*)LLToolSelectLand::getInstance());
	mTextStatus = getChild<LLTextBox>("text status");

	childSetCommitCallback("slider zoom",commit_slider_zoom,this);

	mRadioZoom = getChild<LLCheckBoxCtrl>("radio zoom");
	childSetCommitCallback("radio zoom",commit_radio_zoom,this);
	mRadioOrbit = getChild<LLCheckBoxCtrl>("radio orbit");
	childSetCommitCallback("radio orbit",commit_radio_orbit,this);
	mRadioPan = getChild<LLCheckBoxCtrl>("radio pan");
	childSetCommitCallback("radio pan",commit_radio_pan,this);

	mRadioMove = getChild<LLCheckBoxCtrl>("radio move");
	childSetCommitCallback("radio move",click_popup_grab_drag,this);
	mRadioLift = getChild<LLCheckBoxCtrl>("radio lift");
	childSetCommitCallback("radio lift",click_popup_grab_lift,this);
	mRadioSpin = getChild<LLCheckBoxCtrl>("radio spin");
	childSetCommitCallback("radio spin",click_popup_grab_spin,NULL);
	mRadioPosition = getChild<LLCheckBoxCtrl>("radio position");
	childSetCommitCallback("radio position",commit_select_tool,LLToolCompTranslate::getInstance());
	mRadioRotate = getChild<LLCheckBoxCtrl>("radio rotate");
	childSetCommitCallback("radio rotate",commit_select_tool,LLToolCompRotate::getInstance());
	mRadioStretch = getChild<LLCheckBoxCtrl>("radio stretch");
	childSetCommitCallback("radio stretch",commit_select_tool,LLToolCompScale::getInstance());
	mRadioSelectFace = getChild<LLCheckBoxCtrl>("radio select face");
	childSetCommitCallback("radio select face",commit_select_tool,LLToolFace::getInstance());
	mCheckSelectIndividual = getChild<LLCheckBoxCtrl>("checkbox edit linked parts");
	childSetValue("checkbox edit linked parts",(BOOL)gSavedSettings.getBOOL("EditLinkedParts"));
	childSetCommitCallback("checkbox edit linked parts",commit_select_component,this);
	mCheckSnapToGrid = getChild<LLCheckBoxCtrl>("checkbox snap to grid");
	childSetValue("checkbox snap to grid",(BOOL)gSavedSettings.getBOOL("SnapEnabled"));
	mBtnGridOptions = getChild<LLButton>("Options...");
	childSetAction("Options...",onClickGridOptions, this);
	mCheckStretchUniform = getChild<LLCheckBoxCtrl>("checkbox uniform");
	childSetValue("checkbox uniform",(BOOL)gSavedSettings.getBOOL("ScaleUniform"));
	mCheckStretchTexture = getChild<LLCheckBoxCtrl>("checkbox stretch textures");
	childSetValue("checkbox stretch textures",(BOOL)gSavedSettings.getBOOL("ScaleStretchTextures"));
	mTextGridMode = getChild<LLTextBox>("text ruler mode");
	mComboGridMode = getChild<LLComboBox>("combobox grid mode");
	childSetCommitCallback("combobox grid mode",commit_grid_mode, this);
	mBtnLink = getChild<LLButton>("link_btn");
	childSetAction("link_btn",onClickLink, this);
	mBtnUnlink = getChild<LLButton>("unlink_btn");
	childSetAction("unlink_btn",onClickUnlink, this);

	mTitleMedia			= getChild<LLMediaCtrl>("title_media");
	mBtnAddMedia = getChild<LLButton>("add_media");
	childSetAction("add_media",onClickBtnAddMedia,this);
	mBtnDeleteMedia = getChild<LLButton>("delete_media");
	childSetAction("delete_media",onClickBtnDeleteMedia,this);
	mBtnEditMedia = getChild<LLButton>("edit_media");
	childSetAction("edit_media",onClickBtnEditMedia,this);

	LLFloaterMediaSettings::hideInstance("media_settings");//imp fixme:ui-design why have an extra floater here?
							//it would perfectly fit into here as a media-tab -- AW

	// Set the default size limits for spinners -- MC
	updateToolsSizeLimits();

	// Set the default decimal precision for spinners -- MC
	updateToolsPrecision();

	//
	// Create Buttons
	//

	static	const std::string	toolNames[]={
			"ToolCube",
			"ToolPrism",
			"ToolPyramid",
			"ToolTetrahedron",
			"ToolCylinder",
			"ToolHemiCylinder",
			"ToolCone",
			"ToolHemiCone",
			"ToolSphere",
			"ToolHemiSphere",
			"ToolTorus",
			"ToolTube",
			"ToolRing",
			"ToolTree",
			"ToolGrass"};
	void*	toolData[]={
			&LLToolPlacerPanel::sCube,
			&LLToolPlacerPanel::sPrism,
			&LLToolPlacerPanel::sPyramid,
			&LLToolPlacerPanel::sTetrahedron,
			&LLToolPlacerPanel::sCylinder,
			&LLToolPlacerPanel::sCylinderHemi,
			&LLToolPlacerPanel::sCone,
			&LLToolPlacerPanel::sConeHemi,
			&LLToolPlacerPanel::sSphere,
			&LLToolPlacerPanel::sSphereHemi,
			&LLToolPlacerPanel::sTorus,
			&LLToolPlacerPanel::sSquareTorus,
			&LLToolPlacerPanel::sTriangleTorus,
			&LLToolPlacerPanel::sTree,
			&LLToolPlacerPanel::sGrass};
	for(size_t t=0; t<LL_ARRAY_SIZE(toolNames); ++t)
	{
		LLButton *found = getChild<LLButton>(toolNames[t]);
		if(found)
		{
			found->setClickedCallback(setObjectType,toolData[t]);
			mButtons.push_back( found );
		}
		else
		{
			llwarns << "Tool button not found! DOA Pending." << llendl;
		}
	}
	mComboTreesGrass = getChild<LLComboBox>("trees_grass");
	childSetCommitCallback("trees_grass", onSelectTreesGrass, (void*)0);
	mCheckCopySelection = getChild<LLCheckBoxCtrl>("checkbox copy selection");
	childSetValue("checkbox copy selection",(BOOL)gSavedSettings.getBOOL("CreateToolCopySelection"));
	mCheckSticky = getChild<LLCheckBoxCtrl>("checkbox sticky");
	childSetValue("checkbox sticky",(BOOL)gSavedSettings.getBOOL("CreateToolKeepSelected"));
	mCheckCopyCenters = getChild<LLCheckBoxCtrl>("checkbox copy centers");
	childSetValue("checkbox copy centers",(BOOL)gSavedSettings.getBOOL("CreateToolCopyCenters"));
	mCheckCopyRotates = getChild<LLCheckBoxCtrl>("checkbox copy rotates");
	childSetValue("checkbox copy rotates",(BOOL)gSavedSettings.getBOOL("CreateToolCopyRotates"));
	mRadioSelectLand = getChild<LLCheckBoxCtrl>("radio select land");
	childSetCommitCallback("radio select land",commit_select_tool, LLToolSelectLand::getInstance());
	mRadioDozerFlatten = getChild<LLCheckBoxCtrl>("radio flatten");
	childSetCommitCallback("radio flatten",click_popup_dozer_mode,  (void*)0);
	mRadioDozerRaise = getChild<LLCheckBoxCtrl>("radio raise");
	childSetCommitCallback("radio raise",click_popup_dozer_mode,  (void*)1);
	mRadioDozerLower = getChild<LLCheckBoxCtrl>("radio lower");
	childSetCommitCallback("radio lower",click_popup_dozer_mode,  (void*)2);
	mRadioDozerSmooth = getChild<LLCheckBoxCtrl>("radio smooth");
	childSetCommitCallback("radio smooth",click_popup_dozer_mode,  (void*)3);
	mRadioDozerNoise = getChild<LLCheckBoxCtrl>("radio noise");
	childSetCommitCallback("radio noise",click_popup_dozer_mode,  (void*)4);
	mRadioDozerRevert = getChild<LLCheckBoxCtrl>("radio revert");
	childSetCommitCallback("radio revert",click_popup_dozer_mode,  (void*)5);
	mBtnApplyToSelection = getChild<LLButton>("button apply to selection");
	childSetAction("button apply to selection",click_apply_to_selection,  (void*)0);

	mSliderDozerSize = getChild<LLSlider>("slider brush size");
	childSetCommitCallback("slider brush size", commit_slider_dozer_size,  (void*)0);
	childSetValue( "slider brush size", gSavedSettings.getF32("LandBrushSize"));
	
	mSliderDozerForce = getChild<LLSlider>("slider force");
	childSetCommitCallback("slider force",commit_slider_dozer_force,  (void*)0);
	// the setting stores the actual force multiplier, but the slider is logarithmic, so we convert here
	childSetValue( "slider force", log10(gSavedSettings.getF32("LandBrushForce")));

	mTab = getChild<LLTabContainer>("Object Info Tabs");
	if(mTab)
	{
		mTab->setFollows(FOLLOWS_TOP | FOLLOWS_LEFT);
		mTab->setBorderVisible(FALSE);
		mTab->selectFirstTab();
	}

	mStatusText["rotate"] = getString("status_rotate");
	mStatusText["scale"] = getString("status_scale");
	mStatusText["move"] = getString("status_move");
	mStatusText["modifyland"] = getString("status_modifyland");
	mStatusText["camera"] = getString("status_camera");
	mStatusText["grab"] = getString("status_grab");
	mStatusText["place"] = getString("status_place");
	mStatusText["selectland"] = getString("status_selectland");
	
	return TRUE;
}

// Create the popupview with a dummy center.  It will be moved into place
// during LLViewerWindow's per-frame hover processing.
LLFloaterTools::LLFloaterTools()
:	LLFloater(std::string("toolbox floater")),
	mBtnFocus(NULL),
	mBtnMove(NULL),
	mBtnEdit(NULL),
	mBtnCreate(NULL),
	mBtnLand(NULL),
	mTextStatus(NULL),

	mRadioOrbit(NULL),
	mRadioZoom(NULL),
	mRadioPan(NULL),

	mRadioMove(NULL),
	mRadioLift(NULL),
	mRadioSpin(NULL),

	mRadioPosition(NULL),
	mRadioRotate(NULL),
	mRadioStretch(NULL),
	mRadioSelectFace(NULL),
	mCheckSelectIndividual(NULL),

	mCheckSnapToGrid(NULL),
	mBtnGridOptions(NULL),
	mTextGridMode(NULL),
	mComboGridMode(NULL),
	mCheckStretchUniform(NULL),
	mCheckStretchTexture(NULL),

	mBtnRotateLeft(NULL),
	mBtnRotateReset(NULL),
	mBtnRotateRight(NULL),

	mBtnDelete(NULL),
	mBtnDuplicate(NULL),
	mBtnDuplicateInPlace(NULL),

	mBtnLink(NULL),
	mBtnUnlink(NULL),

	mComboTreesGrass(NULL),
	mCheckSticky(NULL),
	mCheckCopySelection(NULL),
	mCheckCopyCenters(NULL),
	mCheckCopyRotates(NULL),
	mRadioSelectLand(NULL),
	mRadioDozerFlatten(NULL),
	mRadioDozerRaise(NULL),
	mRadioDozerLower(NULL),
	mRadioDozerSmooth(NULL),
	mRadioDozerNoise(NULL),
	mRadioDozerRevert(NULL),
	mSliderDozerSize(NULL),
	mSliderDozerForce(NULL),
	mBtnApplyToSelection(NULL),

	mTab(NULL),
	mPanelPermissions(NULL),
	mPanelObject(NULL),
	mPanelVolume(NULL),
	mPanelContents(NULL),
	mPanelFace(NULL),
	mPanelLandInfo(NULL),

	mTabLand(NULL),
	mDirty(TRUE),
	mPrecision(3),

	mBtnAddMedia(NULL),
	mBtnDeleteMedia(NULL),
	mBtnEditMedia(NULL),
	mTitleMedia(NULL),
	mNeedMediaTitle(TRUE)
{
	setAutoFocus(FALSE);
	LLCallbackMap::map_t factory_map;
	factory_map["General"] = LLCallbackMap(createPanelPermissions, this);//LLPanelPermissions
	factory_map["Object"] = LLCallbackMap(createPanelObject, this);//LLPanelObject
	factory_map["Features"] = LLCallbackMap(createPanelVolume, this);//LLPanelVolume
	factory_map["Texture"] = LLCallbackMap(createPanelFace, this);//LLPanelFace
	factory_map["Contents"] = LLCallbackMap(createPanelContents, this);//LLPanelContents
	factory_map["ContentsInventory"] = LLCallbackMap(createPanelContentsInventory, this);//LLPanelContents
	factory_map["land info panel"] = LLCallbackMap(createPanelLandInfo, this);//LLPanelLandInfo

	LLUICtrlFactory::getInstance()->buildFloater(this,"floater_tools.xml",&factory_map,FALSE);
}

LLFloaterTools::~LLFloaterTools()
{
	// children automatically deleted
}

void LLFloaterTools::setStatusText(const std::string& text)
{
	std::map<std::string, std::string>::iterator iter = mStatusText.find(text);
	if (iter != mStatusText.end())
	{
		mTextStatus->setText(iter->second);
	}
	else
	{
		mTextStatus->setText(text);
	}
}

void LLFloaterTools::refresh()
{
	const S32 INFO_WIDTH = getRect().getWidth();
	const S32 INFO_HEIGHT = 384;
	LLRect object_info_rect(0, 0, INFO_WIDTH, -INFO_HEIGHT);
	BOOL all_volume = LLSelectMgr::getInstance()->selectionAllPCode( LL_PCODE_VOLUME );

	S32 idx_features = mTab->getPanelIndexByTitle(PANEL_NAMES[PANEL_FEATURES]);
	S32 idx_face = mTab->getPanelIndexByTitle(PANEL_NAMES[PANEL_FACE]);
	S32 idx_contents = mTab->getPanelIndexByTitle(PANEL_NAMES[PANEL_CONTENTS]);

	S32 selected_index = mTab->getCurrentPanelIndex();

	if (!all_volume && (selected_index == idx_features || selected_index == idx_face ||
		selected_index == idx_contents))
	{
		mTab->selectFirstTab();
	}

	mTab->enableTabButton(idx_features, all_volume);
	mTab->enableTabButton(idx_face, all_volume);
	mTab->enableTabButton(idx_contents, all_volume);

	// Refresh object and prim count labels
	LLLocale locale(LLLocale::USER_LOCALE);
	std::string obj_count_string;
	LLResMgr::getInstance()->getIntegerString(obj_count_string, LLSelectMgr::getInstance()->getSelection()->getRootObjectCount());
	childSetTextArg("obj_count",  "[COUNT]", obj_count_string);	
	std::string prim_count_string;
	LLResMgr::getInstance()->getIntegerString(prim_count_string, LLSelectMgr::getInstance()->getSelection()->getObjectCount());
	childSetTextArg("prim_count", "[COUNT]", prim_count_string);

	updateToolsPrecision();

	// Refresh child tabs
	mPanelPermissions->refresh();
	mPanelObject->refresh();
	mPanelVolume->refresh();
	mPanelFace->refresh();
	refreshMedia();
	mPanelContents->refresh();
	mPanelLandInfo->refresh();
}

void LLFloaterTools::draw()
{
	if (mDirty)
	{
		refresh();
		mDirty = FALSE;
	}

	// grab media name/title and update the UI widget
	updateMediaTitle();


	//	mCheckSelectIndividual->set(gSavedSettings.getBOOL("EditLinkedParts"));
	LLFloater::draw();
}

void LLFloaterTools::dirty()
{
	mDirty = TRUE; 
	LLFloaterOpenObject::dirty();
}

// Clean up any tool state that should not persist when the
// floater is closed.
void LLFloaterTools::resetToolState()
{
	gCameraBtnZoom = TRUE;
	gCameraBtnOrbit = FALSE;
	gCameraBtnPan = FALSE;

	gGrabBtnSpin = FALSE;
	gGrabBtnVertical = FALSE;
}

void LLFloaterTools::updatePopup(LLCoordGL center, MASK mask)
{
	LLTool *tool = LLToolMgr::getInstance()->getCurrentTool();

	// HACK to allow seeing the buttons when you have the app in a window.
	// Keep the visibility the same as it 
	if (tool == gToolNull)
	{
		return;
	}

	if ( isMinimized() )
	{	// SL looks odd if we draw the tools while the window is minimized
		return;
	}
	
	// Focus buttons
	BOOL focus_visible = (	tool == LLToolCamera::getInstance() );

	mBtnFocus	->setToggleState( focus_visible );

	mRadioZoom	->setVisible( focus_visible );
	mRadioOrbit	->setVisible( focus_visible );
	mRadioPan	->setVisible( focus_visible );
	childSetVisible("slider zoom", focus_visible);
	childSetEnabled("slider zoom", gCameraBtnZoom);

	mRadioZoom	->set(	!gCameraBtnOrbit &&
						!gCameraBtnPan &&
						!(mask == MASK_ORBIT) &&
						!(mask == (MASK_ORBIT | MASK_ALT)) &&
						!(mask == MASK_PAN) &&
						!(mask == (MASK_PAN | MASK_ALT)) );

	mRadioOrbit	->set(	gCameraBtnOrbit || 
						(mask == MASK_ORBIT) ||
						(mask == (MASK_ORBIT | MASK_ALT)) );

	mRadioPan	->set(	gCameraBtnPan ||
						(mask == MASK_PAN) ||
						(mask == (MASK_PAN | MASK_ALT)) );

	// multiply by correction factor because volume sliders go [0, 0.5]
	childSetValue( "slider zoom", gAgent.getCameraZoomFraction() * 0.5f);

	// Move buttons
	BOOL move_visible = (tool == LLToolGrab::getInstance());

	if (mBtnMove) mBtnMove	->setToggleState( move_visible );

	// HACK - highlight buttons for next click
	if (mRadioMove)
	{
		mRadioMove	->setVisible( move_visible );
		mRadioMove	->set(	!gGrabBtnSpin && 
							!gGrabBtnVertical &&
							!(mask == MASK_VERTICAL) && 
							!(mask == MASK_SPIN) );
	}

	if (mRadioLift)
	{
		mRadioLift	->setVisible( move_visible );
		mRadioLift	->set(	gGrabBtnVertical || 
							(mask == MASK_VERTICAL) );
	}

	if (mRadioSpin)
	{
		mRadioSpin	->setVisible( move_visible );
		mRadioSpin	->set(	gGrabBtnSpin || 
							(mask == MASK_SPIN) );
	}

	// Edit buttons
	BOOL edit_visible = tool == LLToolCompTranslate::getInstance() ||
						tool == LLToolCompRotate::getInstance() ||
						tool == LLToolCompScale::getInstance() ||
						tool == LLToolFace::getInstance() ||
						tool == LLToolIndividual::getInstance() ||
						tool == LLToolPipette::getInstance();

	mBtnEdit	->setToggleState( edit_visible );

	mRadioPosition	->setVisible( edit_visible );
	mRadioRotate	->setVisible( edit_visible );
	mRadioStretch	->setVisible( edit_visible );
	if (mRadioSelectFace)
	{
		mRadioSelectFace->setVisible( edit_visible );
		mRadioSelectFace->set( tool == LLToolFace::getInstance() );
	}

	if (mCheckSelectIndividual)
	{
		mCheckSelectIndividual->setVisible(edit_visible);
		//mCheckSelectIndividual->set(gSavedSettings.getBOOL("EditLinkedParts"));
	}

	mRadioPosition	->set( tool == LLToolCompTranslate::getInstance() );
	mRadioRotate	->set( tool == LLToolCompRotate::getInstance() );
	mRadioStretch	->set( tool == LLToolCompScale::getInstance() );

	if (mComboGridMode) 
	{
		mComboGridMode->setVisible( edit_visible );
		S32 index = mComboGridMode->getCurrentIndex();
		mComboGridMode->removeall();

		switch (mObjectSelection->getSelectType())
		{
		case SELECT_TYPE_HUD:
		  mComboGridMode->add(getString("grid_screen_text"));
		  mComboGridMode->add(getString("grid_local_text"));
		  //mComboGridMode->add(getString("grid_reference_text"));
		  break;
		case SELECT_TYPE_WORLD:
		  mComboGridMode->add(getString("grid_world_text"));
		  mComboGridMode->add(getString("grid_local_text"));
		  mComboGridMode->add(getString("grid_reference_text"));
		  break;
		case SELECT_TYPE_ATTACHMENT:
		  mComboGridMode->add(getString("grid_attachment_text"));
		  mComboGridMode->add(getString("grid_local_text"));
		  mComboGridMode->add(getString("grid_reference_text"));
		  break;
		}

		mComboGridMode->setCurrentByIndex(index);
	}
	if (mTextGridMode) mTextGridMode->setVisible( edit_visible );

	// Snap to grid disabled for grab tool - very confusing
	if (mCheckSnapToGrid) mCheckSnapToGrid->setVisible( edit_visible /* || tool == LLToolGrab::getInstance() */ );
	if (mBtnGridOptions) mBtnGridOptions->setVisible( edit_visible /* || tool == LLToolGrab::getInstance() */ );

	//mCheckSelectLinked	->setVisible( edit_visible );
	if (mCheckStretchUniform) mCheckStretchUniform->setVisible( edit_visible );
	if (mCheckStretchTexture) mCheckStretchTexture->setVisible( edit_visible );

	if (mBtnLink) mBtnLink->setVisible( edit_visible );
	if (mBtnUnlink) mBtnUnlink->setVisible( edit_visible );

	//TODO: Move these into llselectmgr
	// Check to see if we can link things
	bool can_link = false;
	if (!gSavedSettings.getBOOL("EditLinkedParts"))
	{
		if(LLSelectMgr::getInstance()->selectGetAllRootsValid() && LLSelectMgr::getInstance()->getSelection()->getRootObjectCount() >= 2)
		{
			struct f : public LLSelectedObjectFunctor
			{
				virtual bool apply(LLViewerObject* object)
				{
					return object->permModify();
				}
			}
			func;
			const bool firstonly = true;
			can_link = LLSelectMgr::getInstance()->getSelection()->applyToRootObjects(&func, firstonly);
		}
	}
	mBtnLink->setEnabled(can_link);

	// Check to see if we can unlink things
	bool can_unlink = false;
	if (tool != LLToolFace::getInstance())
	{
		if (LLSelectMgr::getInstance()->selectGetAllRootsValid() &&
			LLSelectMgr::getInstance()->getSelection()->getFirstEditableObject() &&
			!LLSelectMgr::getInstance()->getSelection()->isAttachment())
		{
			// LL's viewer unlinks the last linkset selected, 
			// regardless of how many linksets or prims are selected total. 
			// Preserve that behavior when enabling the unlink option.  
			if (gSavedSettings.getBOOL("EditLinkedParts"))
			{
				struct f : public LLSelectedNodeFunctor
				{
					virtual bool apply(LLSelectNode* pNode)
					{
						// Return the first selection node that is
						//    1) not a root prim
						//    2) or a root prim that has child prims
						// or in other words: any prim that is part of a linkset
						return (pNode->getObject() != pNode->getObject()->getRootEdit()) || 
								(pNode->getObject()->numChildren() != 0);
					}
				} func;

				if (LLSelectMgr::getInstance()->getSelection()->getFirstRootNode(&func, TRUE))
				{
					// the selection contains at least one prim (child or root) that is part of a linkset
					can_unlink = true;
				}
			}
			else
			{
				if (LLSelectMgr::getInstance()->getSelection()->getRootObjectCount() != 
					LLSelectMgr::getInstance()->getSelection()->getObjectCount())
				{
					can_unlink = true;
				}
			}
		}
	}
	mBtnUnlink->setEnabled(can_unlink);


	// Create buttons
	BOOL create_visible = (tool == LLToolCompCreate::getInstance());

	mBtnCreate	->setToggleState(	tool == LLToolCompCreate::getInstance() );

	updateTreeGrassCombo(create_visible);

	if (mCheckCopySelection
		&& mCheckCopySelection->get())
	{
		// don't highlight any placer button
		for (std::vector<LLButton*>::size_type i = 0; i < mButtons.size(); i++)
		{
			mButtons[i]->setToggleState(FALSE);
			mButtons[i]->setVisible( create_visible );
		}
	}
	else
	{
		// Highlight the correct placer button
		for( std::vector<LLButton*>::size_type i = 0; i < mButtons.size(); i++ )
		{
			LLPCode pcode = LLToolPlacer::getObjectType();
			void *userdata = mButtons[i]->getCallbackUserData();
			LLPCode *cur = (LLPCode*) userdata;

			BOOL state = (pcode == *cur);
			mButtons[i]->setToggleState( state );
			mButtons[i]->setVisible( create_visible );
		}
	}

	if (mCheckSticky) mCheckSticky		->setVisible( create_visible );
	if (mCheckCopySelection) mCheckCopySelection	->setVisible( create_visible );
	if (mCheckCopyCenters) mCheckCopyCenters	->setVisible( create_visible );
	if (mCheckCopyRotates) mCheckCopyRotates	->setVisible( create_visible );

	if (mCheckCopyCenters) mCheckCopyCenters->setEnabled( mCheckCopySelection->get() );
	if (mCheckCopyRotates) mCheckCopyRotates->setEnabled( mCheckCopySelection->get() );

	// Land buttons
	BOOL land_visible = (tool == LLToolBrushLand::getInstance() || tool == LLToolSelectLand::getInstance() );

	if (mBtnLand)	mBtnLand	->setToggleState( land_visible );

	//	mRadioEditLand	->set( tool == LLToolBrushLand::getInstance() );
	if (mRadioSelectLand)	mRadioSelectLand->set( tool == LLToolSelectLand::getInstance() );

	//	mRadioEditLand	->setVisible( land_visible );
	if (mRadioSelectLand)	mRadioSelectLand->setVisible( land_visible );

	S32 dozer_mode = gSavedSettings.getS32("RadioLandBrushAction");

	if (mRadioDozerFlatten)
	{
		mRadioDozerFlatten	->set( tool == LLToolBrushLand::getInstance() && dozer_mode == 0);
		mRadioDozerFlatten	->setVisible( land_visible );
	}
	if (mRadioDozerRaise)
	{
		mRadioDozerRaise	->set( tool == LLToolBrushLand::getInstance() && dozer_mode == 1);
		mRadioDozerRaise	->setVisible( land_visible );
	}
	if (mRadioDozerLower)
	{
		mRadioDozerLower	->set( tool == LLToolBrushLand::getInstance() && dozer_mode == 2);
		mRadioDozerLower	->setVisible( land_visible );
	}
	if (mRadioDozerSmooth)
	{
		mRadioDozerSmooth	->set( tool == LLToolBrushLand::getInstance() && dozer_mode == 3);
		mRadioDozerSmooth	->setVisible( land_visible );
	}
	if (mRadioDozerNoise)
	{
		mRadioDozerNoise	->set( tool == LLToolBrushLand::getInstance() && dozer_mode == 4);
		mRadioDozerNoise	->setVisible( land_visible );
	}
	if (mRadioDozerRevert)
	{
		mRadioDozerRevert	->set( tool == LLToolBrushLand::getInstance() && dozer_mode == 5);
		mRadioDozerRevert	->setVisible( land_visible );
	}
	if (mBtnApplyToSelection)
	{
		mBtnApplyToSelection->setVisible( land_visible );
		mBtnApplyToSelection->setEnabled( land_visible && !LLViewerParcelMgr::getInstance()->selectionEmpty() && tool != LLToolSelectLand::getInstance());
	}
	if (mSliderDozerSize)
	{
		mSliderDozerSize	->setVisible( land_visible );
		childSetVisible("Bulldozer:", land_visible);
		childSetVisible("Dozer Size:", land_visible);
	}
	if (mSliderDozerForce)
	{
		mSliderDozerForce	->setVisible( land_visible );
		childSetVisible("Strength:", land_visible);
	}

	childSetVisible("obj_count", !land_visible);
	childSetVisible("prim_count", !land_visible);
	mTab->setVisible(!land_visible);
	mPanelLandInfo->setVisible(land_visible);
}


// virtual
BOOL LLFloaterTools::canClose()
{
	// don't close when quitting, so camera will stay put
	return !LLApp::isExiting();
}

// virtual
void LLFloaterTools::onOpen()
{
	mParcelSelection = LLViewerParcelMgr::getInstance()->getFloatingParcelSelection();
	mObjectSelection = LLSelectMgr::getInstance()->getEditSelection();
	
	// gMenuBarView->setItemVisible(std::string("Tools"), TRUE);
	// gMenuBarView->arrange();
}

// virtual
void LLFloaterTools::onClose(bool app_quitting)
{
	setMinimized(FALSE);
	setVisible(FALSE);
	mTab->setVisible(FALSE);

	LLViewerJoystick::getInstance()->moveAvatar(false);

	// destroy media source used to grab media title
	if( mTitleMedia )
		mTitleMedia->unloadMediaSource();

	// Different from handle_reset_view in that it doesn't actually 
	//   move the camera if EditCameraMovement is not set.
	gAgent.resetView(gSavedSettings.getBOOL("EditCameraMovement"));
	
	// exit component selection mode
	LLSelectMgr::getInstance()->promoteSelectionToRoot();
	gSavedSettings.setBOOL("EditLinkedParts", FALSE);

	gViewerWindow->showCursor();

	resetToolState();

	mParcelSelection = NULL;
	mObjectSelection = NULL;

	// Switch back to basic toolset
	LLToolMgr::getInstance()->setCurrentToolset(gBasicToolset);
	// we were already in basic toolset, using build tools
	// so manually reset tool to default (pie menu tool)
	LLToolMgr::getInstance()->getCurrentToolset()->selectFirstTool();

	// gMenuBarView->setItemVisible(std::string("Tools"), FALSE);
	// gMenuBarView->arrange();
}

void LLFloaterTools::showPanel(EInfoPanel panel)
{
	llassert(panel >= 0 && panel < PANEL_COUNT);
	mTab->selectTabByName(PANEL_NAMES[panel]);
}

void click_popup_info(void*)
{
//	gBuildView->setPropertiesPanelOpen(TRUE);
}

void click_popup_done(void*)
{
	handle_reset_view();
}

void click_popup_grab_drag(LLUICtrl*, void*)
{
	gGrabBtnVertical = FALSE;
	gGrabBtnSpin = FALSE;
}

void click_popup_grab_lift(LLUICtrl*, void*)
{
	gGrabBtnVertical = TRUE;
	gGrabBtnSpin = FALSE;
}

void click_popup_grab_spin(LLUICtrl*, void*)
{
	gGrabBtnVertical = FALSE;
	gGrabBtnSpin = TRUE;
}

void commit_radio_zoom(LLUICtrl *, void*)
{
	gCameraBtnZoom = TRUE;
	gCameraBtnOrbit = FALSE;
	gCameraBtnPan = FALSE;
}

void commit_radio_orbit(LLUICtrl *, void*)
{
	gCameraBtnZoom = FALSE;
	gCameraBtnOrbit = TRUE;
	gCameraBtnPan = FALSE;
}

void commit_radio_pan(LLUICtrl *, void*)
{
	gCameraBtnZoom = FALSE;
	gCameraBtnOrbit = FALSE;
	gCameraBtnPan = TRUE;
}

void commit_slider_zoom(LLUICtrl *ctrl, void*)
{
	// renormalize value, since max "volume" level is 0.5 for some reason
	F32 zoom_level = (F32)ctrl->getValue().asReal() * 2.f; // / 0.5f;
	gAgent.setCameraZoomFraction(zoom_level);
}

void click_popup_rotate_left(void*)
{
	LLSelectMgr::getInstance()->selectionRotateAroundZ( 45.f );
	dialog_refresh_all();
}

void click_popup_rotate_reset(void*)
{
	LLSelectMgr::getInstance()->selectionResetRotation();
	dialog_refresh_all();
}

void click_popup_rotate_right(void*)
{
	LLSelectMgr::getInstance()->selectionRotateAroundZ( -45.f );
	dialog_refresh_all();
}


void click_popup_dozer_mode(LLUICtrl *, void *user)
{
	S32 mode = (S32)(intptr_t) user;
	gFloaterTools->setEditTool( LLToolBrushLand::getInstance() );
	gSavedSettings.setS32("RadioLandBrushAction", mode);
}

void commit_slider_dozer_size(LLUICtrl *ctrl, void*)
{
	F32 size = (F32)ctrl->getValue().asReal();
	gSavedSettings.setF32("LandBrushSize", size);
}

void commit_slider_dozer_force(LLUICtrl *ctrl, void*)
{
	// the slider is logarithmic, so we exponentiate to get the actual force multiplier
	F32 dozer_force = pow(10.f, (F32)ctrl->getValue().asReal());
	gSavedSettings.setF32("LandBrushForce", dozer_force);
}




void click_apply_to_selection(void* user)
{
	LLToolBrushLand::getInstance()->modifyLandInSelectionGlobal();
}

void commit_select_tool(LLUICtrl *ctrl, void *data)
{
	S32 show_owners = gSavedSettings.getBOOL("ShowParcelOwners");
	gFloaterTools->setEditTool(data);
	gSavedSettings.setBOOL("ShowParcelOwners", show_owners);
}

void commit_select_component(LLUICtrl *ctrl, void *data)
{
	LLFloaterTools* floaterp = (LLFloaterTools*)data;

	//forfeit focus
	if (gFocusMgr.childHasKeyboardFocus(floaterp))
	{
		gFocusMgr.setKeyboardFocus(NULL);
	}

	BOOL select_individuals = floaterp->mCheckSelectIndividual->get();
	gSavedSettings.setBOOL("EditLinkedParts", select_individuals);
	floaterp->dirty();

	if (select_individuals)
	{
		LLSelectMgr::getInstance()->demoteSelectionToIndividuals();
	}
	else
	{
		LLSelectMgr::getInstance()->promoteSelectionToRoot();
	}
}

void commit_grid_mode(LLUICtrl *ctrl, void *data)   
{   
	LLComboBox* combo = (LLComboBox*)ctrl;   
    
	LLSelectMgr::getInstance()->setGridMode((EGridMode)combo->getCurrentIndex());
} 

// static 
void LLFloaterTools::setObjectType( void* data )
{
	LLPCode pcode = *(LLPCode*) data;
	LLToolPlacer::setObjectType( pcode );
	gSavedSettings.setBOOL("CreateToolCopySelection", FALSE);
	gFloaterTools->updateTreeGrassCombo(true);
	gFocusMgr.setMouseCapture(NULL);
}

// static
void LLFloaterTools::onClickGridOptions(void* data)
{
	//LLFloaterTools* floaterp = (LLFloaterTools*)data;
	LLFloaterBuildOptions::show(NULL);
	// RN: this makes grid options dependent on build tools window
	//floaterp->addDependentFloater(LLFloaterBuildOptions::getInstance(), FALSE);
}

void LLFloaterTools::setEditTool(void* tool_pointer)
{
	select_tool(tool_pointer);
}

void LLFloaterTools::onFocusReceived()
{
	LLToolMgr::getInstance()->setCurrentToolset(gBasicToolset);
	LLFloater::onFocusReceived();
}

void LLFloaterTools::updateTreeGrassCombo(bool visible)
{
	LLTextBox* tree_grass_label = getChild<LLTextBox>("tree_grass_label");
	if (visible) 
	{
		LLPCode pcode = LLToolPlacer::getObjectType();
		std::map<std::string, S32>::iterator it, end;
		std::string selected;
		if (pcode == LLToolPlacerPanel::sTree) 
		{
			tree_grass_label->setVisible(visible);
			LLButton* button = getChild<LLButton>("ToolTree");
			tree_grass_label->setText(button->getToolTip());

			selected = gSavedSettings.getString("LastTree");
			it = LLVOTree::sSpeciesNames.begin();
			end = LLVOTree::sSpeciesNames.end();
		} 
		else if (pcode == LLToolPlacerPanel::sGrass) 
		{
			tree_grass_label->setVisible(visible);
			LLButton* button = getChild<LLButton>("ToolGrass");
			tree_grass_label->setText(button->getToolTip());

			selected = gSavedSettings.getString("LastGrass");
			it = LLVOGrass::sSpeciesNames.begin();
			end = LLVOGrass::sSpeciesNames.end();
		} 
		else 
		{
			mComboTreesGrass->removeall();
			mComboTreesGrass->setLabel(LLStringExplicit(""));  // LLComboBox::removeall() does not clear the label
			mComboTreesGrass->setEnabled(false);
			mComboTreesGrass->setVisible(false);
			tree_grass_label->setVisible(false);
			return;
		}

		mComboTreesGrass->removeall();
		mComboTreesGrass->add("Random");

		int select = 0, i = 0;

		while (it != end) 
		{
			const std::string &species = it->first;
			mComboTreesGrass->add(species);  ++i;
			if (species == selected) select = i;
			++it;
		}
		// if saved species not found, default to "Random"
		mComboTreesGrass->selectNthItem(select);
		mComboTreesGrass->setEnabled(true);
	}
	
	mComboTreesGrass->setVisible(visible);
	tree_grass_label->setVisible(visible);
}

// static
void LLFloaterTools::onSelectTreesGrass(LLUICtrl*, void*)
{
	const std::string &selected = gFloaterTools->mComboTreesGrass->getValue();
	LLPCode pcode = LLToolPlacer::getObjectType();
	if (pcode == LLToolPlacerPanel::sTree) 
	{
		gSavedSettings.setString("LastTree", selected);
	} 
	else if (pcode == LLToolPlacerPanel::sGrass) 
	{
		gSavedSettings.setString("LastGrass", selected);
	}  
}

// static
void LLFloaterTools::onClickLink(void* data)
{
	if(!LLSelectMgr::getInstance()->selectGetAllRootsValid())
	{
		LLNotifications::instance().add("UnableToLinkWhileDownloading");
		return;
	}

	S32 max_linked_prims = gHippoLimits->getMaxLinkedPrims();
	if (max_linked_prims > -1)
	{
		S32 object_count = LLSelectMgr::getInstance()->getSelection()->getObjectCount();
		if (object_count > max_linked_prims + 1)
		{
			LLSD args;
			args["COUNT"] = llformat("%d", object_count);
			args["MAX"] = llformat("%d", max_linked_prims +1);
			LLNotifications::instance().add("UnableToLinkObjects", args);
			return;
		}
	}

	if(LLSelectMgr::getInstance()->getSelection()->getRootObjectCount() < 2)
	{
		LLNotifications::instance().add("CannotLinkIncompleteSet");
		return;
	}
	if(!LLSelectMgr::getInstance()->selectGetRootsModify())
	{
		LLNotifications::instance().add("CannotLinkModify");
		return;
	}
	LLUUID owner_id;
	std::string owner_name;
	if(!LLSelectMgr::getInstance()->selectGetOwner(owner_id, owner_name))
	{
	  // we don't actually care if you're the owner, but novices are
	  // the most likely to be stumped by this one, so offer the
	  // easiest and most likely solution.
	  LLNotifications::instance().add("CannotLinkDifferentOwners");
	  return;
	}
	LLSelectMgr::getInstance()->sendLink();
	return;
}

// static
void LLFloaterTools::onClickUnlink(void* data)
{
	LLSelectMgr::getInstance()->sendDelink();
}


// Media stuff
void LLFloaterTools::refreshMedia()
{
	getMediaState();	
}

bool LLFloaterTools::selectedMediaEditable()
{
	U32 owner_mask_on;
	U32 owner_mask_off;
	U32 valid_owner_perms = LLSelectMgr::getInstance()->selectGetPerm( PERM_OWNER, 
																	  &owner_mask_on, &owner_mask_off );
	U32 group_mask_on;
	U32 group_mask_off;
	U32 valid_group_perms = LLSelectMgr::getInstance()->selectGetPerm( PERM_GROUP, 
																	  &group_mask_on, &group_mask_off );
	U32 everyone_mask_on;
	U32 everyone_mask_off;
	S32 valid_everyone_perms = LLSelectMgr::getInstance()->selectGetPerm( PERM_EVERYONE, 
																		 &everyone_mask_on, &everyone_mask_off );
	
	bool selected_Media_editable = false;
	
	// if perms we got back are valid
	if ( valid_owner_perms &&
		valid_group_perms && 
		valid_everyone_perms )
	{
		
		if ( ( owner_mask_on & PERM_MODIFY ) ||
			( group_mask_on & PERM_MODIFY ) || 
			( group_mask_on & PERM_MODIFY ) )
		{
			selected_Media_editable = true;
		}
		else
			// user is NOT allowed to press the RESET button
		{
			selected_Media_editable = false;
		};
	};
	
	return selected_Media_editable;
}

void LLFloaterTools::getMediaState()
{
	//impfixme
	std::string url = gAgent.getRegion()->getCapability("ObjectMedia");
	bool has_media_capability = (!url.empty());
/*imp fixme:ignoring while porting for testing on local opensim
	if(!has_media_capability)
	{
		childSetEnabled("Add_Media",  FALSE);
		LL_WARNS("LLFloaterTools: media") << "Media not enabled (no capability) in this region!" << LL_ENDL;
		clearMediaSettings();
		return;
	}*/

	LLObjectSelectionHandle selected_objects =LLSelectMgr::getInstance()->getSelection();
	LLViewerObject* first_object = selected_objects->getFirstObject();
	LLTextBox* media_info = getChild<LLTextBox>("media_info");

//impfixme: does editing media on a different parcel then the agents make sense?
/*
	std::string url = first_object->getRegion()->getCapability("ObjectMedia");
	bool has_media_capability = (!url.empty());

	if(!has_media_capability)
	{
		childSetEnabled("Add_Media",  FALSE);
		LL_WARNS("LLFloaterTools: media") << "Media not enabled (no capability) in this region!" << LL_ENDL;
		clearMediaSettings();
		return;
	}
*/

	if( !(first_object 
		  && first_object->getPCode() == LL_PCODE_VOLUME
		  &&first_object->permModify()
	      ))
	{
		childSetEnabled("Add_Media",  FALSE);
		media_info->clear();
		clearMediaSettings();
		return;
	}
	
	bool editable = (first_object->permModify() || selectedMediaEditable());

	// Check modify permissions and whether any selected objects are in
	// the process of being fetched.  If they are, then we're not editable
	if (editable)
	{
		LLObjectSelection::iterator iter = selected_objects->begin(); 
		LLObjectSelection::iterator end = selected_objects->end();
		for ( ; iter != end; ++iter)
		{
			LLSelectNode* node = *iter;
			LLVOVolume* object = dynamic_cast<LLVOVolume*>(node->getObject());
			if (NULL != object)
			{
				if (!object->permModify())
				{
					LL_INFOS("LLFloaterTools: media")
						<< "Selection not editable due to lack of modify permissions on object id "
						<< object->getID() << LL_ENDL;
					
					editable = false;
					break;
				}
				// XXX DISABLE this for now, because when the fetch finally 
				// does come in, the state of this floater doesn't properly
				// update.  Re-selecting fixes the problem, but there is 
				// contention as to whether this is a sufficient solution.
//				if (object->isMediaDataBeingFetched())
//				{
//					LL_INFOS("LLFloaterTools: media")
//						<< "Selection not editable due to media data being fetched for object id "
//						<< object->getID() << LL_ENDL;
//						
//					editable = false;
//					break;
//				}
			}
		}
	}

	// Media settings
	bool bool_has_media = false;
	struct media_functor : public LLSelectedTEGetFunctor<bool>
	{
		bool get(LLViewerObject* object, S32 face)
		{
			LLTextureEntry *te = object->getTE(face);
			if (te)
			{
				return te->hasMedia();
			}
			return false;
		}
	} func;
	
	// check if all faces have media(or, all dont have media)
	LLFloaterMediaSettings::getInstance()->mIdenticalHasMediaInfo = selected_objects->getSelectedTEValue( &func, bool_has_media );
	
	const LLMediaEntry default_media_data;
	
	struct functor_getter_media_data : public LLSelectedTEGetFunctor< LLMediaEntry>
    {
		functor_getter_media_data(const LLMediaEntry& entry): mMediaEntry(entry) {}	

        LLMediaEntry get( LLViewerObject* object, S32 face )
        {
            if ( object )
                if ( object->getTE(face) )
                    if ( object->getTE(face)->getMediaData() )
                        return *(object->getTE(face)->getMediaData());
			return mMediaEntry;
        };
		
		const LLMediaEntry& mMediaEntry;
		
    } func_media_data(default_media_data);

	LLMediaEntry media_data_get;
	LLFloaterMediaSettings::getInstance()->mMultipleMedia = !(selected_objects->getSelectedTEValue( &func_media_data, media_data_get ));
	
	std::string multi_media_info_str = LLTrans::getString("Multiple Media");
	std::string media_title = "";
	mNeedMediaTitle = false;
	// update UI depending on whether "object" (prim or face) has media
	// and whether or not you are allowed to edit it.
	
	childSetEnabled("Add_Media",  editable);
	// IF all the faces have media (or all dont have media)
	if ( LLFloaterMediaSettings::getInstance()->mIdenticalHasMediaInfo )
	{
		// TODO: get media title and set it.
 		media_info->clear();
		// if identical is set, all faces are same (whether all empty or has the same media)
		if(!(LLFloaterMediaSettings::getInstance()->mMultipleMedia) )
		{
			// Media data is valid
			if(media_data_get!=default_media_data)
			{
				// initial media title is the media URL (until we get the name)
				media_title = media_data_get.getHomeURL();

				// kick off a navigate and flag that we need to update the title
				navigateToTitleMedia( media_data_get.getHomeURL() );
				mNeedMediaTitle = true;
			}
			// else all faces might be empty. 
		}
		else // there' re Different Medias' been set on on the faces.
		{
			media_title = multi_media_info_str;
			mNeedMediaTitle = false;
		}
		
		childSetEnabled("media_tex",  bool_has_media && editable);
		childSetEnabled( "edit_media", bool_has_media && LLFloaterMediaSettings::getInstance()->mIdenticalHasMediaInfo && editable );
		childSetEnabled( "delete_media", bool_has_media && editable );
		childSetEnabled( "add_media", ( ! bool_has_media ) && editable );
			// TODO: display a list of all media on the face - use 'identical' flag
	}
	else // not all face has media but at least one does.
	{
		// seleted faces have not identical value
		LLFloaterMediaSettings::getInstance()->mMultipleValidMedia = selected_objects->isMultipleTEValue(&func_media_data, default_media_data );
	
		if(LLFloaterMediaSettings::getInstance()->mMultipleValidMedia)
		{
			media_title = multi_media_info_str;
			mNeedMediaTitle = false;
		}
		else
		{
			// Media data is valid
			if(media_data_get!=default_media_data)
			{
				// initial media title is the media URL (until we get the name)
				media_title = media_data_get.getHomeURL();

				// kick off a navigate and flag that we need to update the title
				navigateToTitleMedia( media_data_get.getHomeURL() );
				mNeedMediaTitle = true;
			}
		}
		
		childSetEnabled("media_tex",  TRUE);
 		childSetEnabled( "edit_media", LLFloaterMediaSettings::getInstance()->mIdenticalHasMediaInfo);
		childSetEnabled( "delete_media", TRUE);
		childSetEnabled( "add_media", FALSE );
	}
	media_info->setText(media_title);
	
	// load values for media settings
	updateMediaSettings();
	
 	LLFloaterMediaSettings::initValues(mMediaSettings, editable );
}


//////////////////////////////////////////////////////////////////////////////
// called when a user wants to add media to a prim or prim face
void LLFloaterTools::onClickBtnAddMedia(void* data)
{
	// check if multiple faces are selected
	if(LLSelectMgr::getInstance()->getSelection()->isMultipleTESelected())
	{
		LLNotifications::getInstance()->add("MultipleFacesSelected", LLSD(), LLSD(), multipleFacesSelectedConfirm);
	}
	else
	{
		onClickBtnEditMedia(data);
	}
}

// static
bool LLFloaterTools::multipleFacesSelectedConfirm(const LLSD& notification, const LLSD& response)
{
	S32 option = LLNotification::getSelectedOption(notification, response);
	switch( option )
	{
		case 0:  // "Yes"
			gFloaterTools->onClickBtnEditMedia(NULL);
			break;
		case 1:  // "No"
		default:
			break;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////
// called when a user wants to edit existing media settings on a prim or prim face
// TODO: test if there is media on the item and only allow editing if present
void LLFloaterTools::onClickBtnEditMedia(void* data)
{
	LLFloaterTools* self = (LLFloaterTools*)data;
	self->refreshMedia();
 	LLFloaterMediaSettings::showInstance("media_settings");	
}

//////////////////////////////////////////////////////////////////////////////
// called when a user wants to delete media from a prim or prim face
void LLFloaterTools::onClickBtnDeleteMedia(void* data)
{
	LLNotifications::getInstance()->add("DeleteMedia", LLSD(), LLSD(), deleteMediaConfirm);
}


// static
bool LLFloaterTools::deleteMediaConfirm(const LLSD& notification, const LLSD& response)
{
	S32 option = LLNotification::getSelectedOption(notification, response);
	switch( option )
	{
		case 0:  // "Yes"
			LLSelectMgr::getInstance()->selectionSetMedia( 0, LLSD() );
			if(LLFloaterMediaSettings::instanceVisible("media_settings"))
			{
				LLFloaterMediaSettings::hideInstance("media_settings");
			}
			break;
			
		case 1:  // "No"
		default:
			break;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////
//
void LLFloaterTools::clearMediaSettings()
{
	LLFloaterMediaSettings::getInstance();
	LLFloaterMediaSettings::clearValues(false);

}

//////////////////////////////////////////////////////////////////////////////
//
void LLFloaterTools::navigateToTitleMedia( const std::string url )
{
	if ( mTitleMedia )
	{
		LLPluginClassMedia* media_plugin = mTitleMedia->getMediaPlugin();
		if ( media_plugin )
		{
			// if it's a movie, we don't want to hear it
			media_plugin->setVolume( 0 );
		};
		mTitleMedia->navigateTo( url );
	};
}

//////////////////////////////////////////////////////////////////////////////
//
void LLFloaterTools::updateMediaTitle()
{
	// only get the media name if we need it
	if ( ! mNeedMediaTitle )
		return;

	// get plugin impl
	LLPluginClassMedia* media_plugin = mTitleMedia->getMediaPlugin();
	if ( media_plugin )
	{
		// get the media name (asynchronous - must call repeatedly)
		std::string media_title = media_plugin->getMediaName();

		// only replace the title if what we get contains something
		if ( ! media_title.empty() )
		{
			// update the UI widget
			LLTextBox* media_title_field = getChild<LLTextBox>("media_info");
			if ( media_title_field )
			{
				media_title_field->setText( media_title );

				// stop looking for a title when we get one
				// FIXME: check this is the right approach
				mNeedMediaTitle = false;
			};
		};
	};
}

//////////////////////////////////////////////////////////////////////////////
//
void LLFloaterTools::updateMediaSettings()
{
    bool identical( false );
    std::string base_key( "" );
    std::string value_str( "" );
    int value_int = 0;
    bool value_bool = false;
	LLObjectSelectionHandle selected_objects =LLSelectMgr::getInstance()->getSelection();
    // TODO: (CP) refactor this using something clever or boost or both !!

    const LLMediaEntry default_media_data;

    // controls 
    U8 value_u8 = default_media_data.getControls();
    struct functor_getter_controls : public LLSelectedTEGetFunctor< U8 >
    {
		functor_getter_controls(const LLMediaEntry &entry) : mMediaEntry(entry) {}
		
        U8 get( LLViewerObject* object, S32 face )
        {
            if ( object )
                if ( object->getTE(face) )
                    if ( object->getTE(face)->getMediaData() )
                        return object->getTE(face)->getMediaData()->getControls();
            return mMediaEntry.getControls();
        };
		
		const LLMediaEntry &mMediaEntry;
		
    } func_controls(default_media_data);
    identical = selected_objects->getSelectedTEValue( &func_controls, value_u8 );
    base_key = std::string( LLMediaEntry::CONTROLS_KEY );
    mMediaSettings[ base_key ] = value_u8;
    mMediaSettings[ base_key + std::string( LLPanelContents::TENTATIVE_SUFFIX ) ] = ! identical;
	
    // First click (formerly left click)
    value_bool = default_media_data.getFirstClickInteract();
    struct functor_getter_first_click : public LLSelectedTEGetFunctor< bool >
    {
		functor_getter_first_click(const LLMediaEntry& entry): mMediaEntry(entry) {}		
		
        bool get( LLViewerObject* object, S32 face )
        {
            if ( object )
                if ( object->getTE(face) )
                    if ( object->getTE(face)->getMediaData() )
                        return object->getTE(face)->getMediaData()->getFirstClickInteract();
            return mMediaEntry.getFirstClickInteract();
        };
		
		const LLMediaEntry &mMediaEntry;
		
    } func_first_click(default_media_data);
    identical = selected_objects->getSelectedTEValue( &func_first_click, value_bool );
    base_key = std::string( LLMediaEntry::FIRST_CLICK_INTERACT_KEY );
    mMediaSettings[ base_key ] = value_bool;
    mMediaSettings[ base_key + std::string( LLPanelContents::TENTATIVE_SUFFIX ) ] = ! identical;
	
    // Home URL
    value_str = default_media_data.getHomeURL();
    struct functor_getter_home_url : public LLSelectedTEGetFunctor< std::string >
    {
		functor_getter_home_url(const LLMediaEntry& entry): mMediaEntry(entry) {}		
		
        std::string get( LLViewerObject* object, S32 face )
        {
            if ( object )
                if ( object->getTE(face) )
                    if ( object->getTE(face)->getMediaData() )
                        return object->getTE(face)->getMediaData()->getHomeURL();
            return mMediaEntry.getHomeURL();
        };
		
		const LLMediaEntry &mMediaEntry;
		
    } func_home_url(default_media_data);
    identical = selected_objects->getSelectedTEValue( &func_home_url, value_str );
    base_key = std::string( LLMediaEntry::HOME_URL_KEY );
    mMediaSettings[ base_key ] = value_str;
    mMediaSettings[ base_key + std::string( LLPanelContents::TENTATIVE_SUFFIX ) ] = ! identical;
	
    // Current URL
    value_str = default_media_data.getCurrentURL();
    struct functor_getter_current_url : public LLSelectedTEGetFunctor< std::string >
    {
		functor_getter_current_url(const LLMediaEntry& entry): mMediaEntry(entry) {}
        
		std::string get( LLViewerObject* object, S32 face )
        {
            if ( object )
                if ( object->getTE(face) )
                    if ( object->getTE(face)->getMediaData() )
                        return object->getTE(face)->getMediaData()->getCurrentURL();
            return mMediaEntry.getCurrentURL();
        };
		
		const LLMediaEntry &mMediaEntry;
		
    } func_current_url(default_media_data);
    identical = selected_objects->getSelectedTEValue( &func_current_url, value_str );
    base_key = std::string( LLMediaEntry::CURRENT_URL_KEY );
    mMediaSettings[ base_key ] = value_str;
    mMediaSettings[ base_key + std::string( LLPanelContents::TENTATIVE_SUFFIX ) ] = ! identical;
	
    // Auto zoom
    value_bool = default_media_data.getAutoZoom();
    struct functor_getter_auto_zoom : public LLSelectedTEGetFunctor< bool >
    {
		
		functor_getter_auto_zoom(const LLMediaEntry& entry)	: mMediaEntry(entry) {}	
		
        bool get( LLViewerObject* object, S32 face )
        {
            if ( object )
                if ( object->getTE(face) )
                    if ( object->getTE(face)->getMediaData() )
                        return object->getTE(face)->getMediaData()->getAutoZoom();
            return mMediaEntry.getAutoZoom();
        };
		
		const LLMediaEntry &mMediaEntry;
		
    } func_auto_zoom(default_media_data);
    identical = selected_objects->getSelectedTEValue( &func_auto_zoom, value_bool );
    base_key = std::string( LLMediaEntry::AUTO_ZOOM_KEY );
    mMediaSettings[ base_key ] = value_bool;
    mMediaSettings[ base_key + std::string( LLPanelContents::TENTATIVE_SUFFIX ) ] = ! identical;
	
    // Auto play
    //value_bool = default_media_data.getAutoPlay();
	// set default to auto play TRUE -- angela  EXT-5172
	value_bool = true;
    struct functor_getter_auto_play : public LLSelectedTEGetFunctor< bool >
    {
		functor_getter_auto_play(const LLMediaEntry& entry)	: mMediaEntry(entry) {}	
			
        bool get( LLViewerObject* object, S32 face )
        {
            if ( object )
                if ( object->getTE(face) )
                    if ( object->getTE(face)->getMediaData() )
                        return object->getTE(face)->getMediaData()->getAutoPlay();
            //return mMediaEntry.getAutoPlay(); set default to auto play TRUE -- angela  EXT-5172
			return true;
        };
		
		const LLMediaEntry &mMediaEntry;
		
    } func_auto_play(default_media_data);
    identical = selected_objects->getSelectedTEValue( &func_auto_play, value_bool );
    base_key = std::string( LLMediaEntry::AUTO_PLAY_KEY );
    mMediaSettings[ base_key ] = value_bool;
    mMediaSettings[ base_key + std::string( LLPanelContents::TENTATIVE_SUFFIX ) ] = ! identical;
	
	
    // Auto scale
	// set default to auto scale TRUE -- angela  EXT-5172
    //value_bool = default_media_data.getAutoScale();
	value_bool = true;
    struct functor_getter_auto_scale : public LLSelectedTEGetFunctor< bool >
    {
		functor_getter_auto_scale(const LLMediaEntry& entry): mMediaEntry(entry) {}	

        bool get( LLViewerObject* object, S32 face )
        {
            if ( object )
                if ( object->getTE(face) )
                    if ( object->getTE(face)->getMediaData() )
                        return object->getTE(face)->getMediaData()->getAutoScale();
           // return mMediaEntry.getAutoScale();  set default to auto scale TRUE -- angela  EXT-5172
			return true;
		};
		
		const LLMediaEntry &mMediaEntry;
		
    } func_auto_scale(default_media_data);
    identical = selected_objects->getSelectedTEValue( &func_auto_scale, value_bool );
    base_key = std::string( LLMediaEntry::AUTO_SCALE_KEY );
    mMediaSettings[ base_key ] = value_bool;
    mMediaSettings[ base_key + std::string( LLPanelContents::TENTATIVE_SUFFIX ) ] = ! identical;
	
    // Auto loop
    value_bool = default_media_data.getAutoLoop();
    struct functor_getter_auto_loop : public LLSelectedTEGetFunctor< bool >
    {
		functor_getter_auto_loop(const LLMediaEntry& entry)	: mMediaEntry(entry) {}	

        bool get( LLViewerObject* object, S32 face )
        {
            if ( object )
                if ( object->getTE(face) )
                    if ( object->getTE(face)->getMediaData() )
                        return object->getTE(face)->getMediaData()->getAutoLoop();
            return mMediaEntry.getAutoLoop();
        };
		
		const LLMediaEntry &mMediaEntry;
		
    } func_auto_loop(default_media_data);
    identical = selected_objects->getSelectedTEValue( &func_auto_loop, value_bool );
    base_key = std::string( LLMediaEntry::AUTO_LOOP_KEY );
    mMediaSettings[ base_key ] = value_bool;
    mMediaSettings[ base_key + std::string( LLPanelContents::TENTATIVE_SUFFIX ) ] = ! identical;
	
    // width pixels (if not auto scaled)
    value_int = default_media_data.getWidthPixels();
    struct functor_getter_width_pixels : public LLSelectedTEGetFunctor< int >
    {
		functor_getter_width_pixels(const LLMediaEntry& entry): mMediaEntry(entry) {}		

        int get( LLViewerObject* object, S32 face )
        {
            if ( object )
                if ( object->getTE(face) )
                    if ( object->getTE(face)->getMediaData() )
                        return object->getTE(face)->getMediaData()->getWidthPixels();
            return mMediaEntry.getWidthPixels();
        };
		
		const LLMediaEntry &mMediaEntry;
		
    } func_width_pixels(default_media_data);
    identical = selected_objects->getSelectedTEValue( &func_width_pixels, value_int );
    base_key = std::string( LLMediaEntry::WIDTH_PIXELS_KEY );
    mMediaSettings[ base_key ] = value_int;
    mMediaSettings[ base_key + std::string( LLPanelContents::TENTATIVE_SUFFIX ) ] = ! identical;
	
    // height pixels (if not auto scaled)
    value_int = default_media_data.getHeightPixels();
    struct functor_getter_height_pixels : public LLSelectedTEGetFunctor< int >
    {
		functor_getter_height_pixels(const LLMediaEntry& entry)	: mMediaEntry(entry) {}
        
		int get( LLViewerObject* object, S32 face )
        {
            if ( object )
                if ( object->getTE(face) )
                    if ( object->getTE(face)->getMediaData() )
                        return object->getTE(face)->getMediaData()->getHeightPixels();
            return mMediaEntry.getHeightPixels();
        };
		
		const LLMediaEntry &mMediaEntry;
		
    } func_height_pixels(default_media_data);
    identical = selected_objects->getSelectedTEValue( &func_height_pixels, value_int );
    base_key = std::string( LLMediaEntry::HEIGHT_PIXELS_KEY );
    mMediaSettings[ base_key ] = value_int;
    mMediaSettings[ base_key + std::string( LLPanelContents::TENTATIVE_SUFFIX ) ] = ! identical;
	
    // Enable Alt image
    value_bool = default_media_data.getAltImageEnable();
    struct functor_getter_enable_alt_image : public LLSelectedTEGetFunctor< bool >
    {
		functor_getter_enable_alt_image(const LLMediaEntry& entry): mMediaEntry(entry) {}
        
		bool get( LLViewerObject* object, S32 face )
        {
            if ( object )
                if ( object->getTE(face) )
                    if ( object->getTE(face)->getMediaData() )
                        return object->getTE(face)->getMediaData()->getAltImageEnable();
            return mMediaEntry.getAltImageEnable();
        };
		
		const LLMediaEntry &mMediaEntry;
		
    } func_enable_alt_image(default_media_data);
    identical = selected_objects->getSelectedTEValue( &func_enable_alt_image, value_bool );
    base_key = std::string( LLMediaEntry::ALT_IMAGE_ENABLE_KEY );
    mMediaSettings[ base_key ] = value_bool;
    mMediaSettings[ base_key + std::string( LLPanelContents::TENTATIVE_SUFFIX ) ] = ! identical;
	
    // Perms - owner interact
    value_bool = 0 != ( default_media_data.getPermsInteract() & LLMediaEntry::PERM_OWNER );
    struct functor_getter_perms_owner_interact : public LLSelectedTEGetFunctor< bool >
    {
		functor_getter_perms_owner_interact(const LLMediaEntry& entry): mMediaEntry(entry) {}
        
		bool get( LLViewerObject* object, S32 face )
        {
            if ( object )
                if ( object->getTE(face) )
                    if ( object->getTE(face)->getMediaData() )
                        return (0 != (object->getTE(face)->getMediaData()->getPermsInteract() & LLMediaEntry::PERM_OWNER));
            return 0 != ( mMediaEntry.getPermsInteract() & LLMediaEntry::PERM_OWNER );
        };
		
		const LLMediaEntry &mMediaEntry;
		
    } func_perms_owner_interact(default_media_data);
    identical = selected_objects->getSelectedTEValue( &func_perms_owner_interact, value_bool );
    base_key = std::string( LLPanelContents::PERMS_OWNER_INTERACT_KEY );
    mMediaSettings[ base_key ] = value_bool;
    mMediaSettings[ base_key + std::string( LLPanelContents::TENTATIVE_SUFFIX ) ] = ! identical;
	
    // Perms - owner control
    value_bool = 0 != ( default_media_data.getPermsControl() & LLMediaEntry::PERM_OWNER );
    struct functor_getter_perms_owner_control : public LLSelectedTEGetFunctor< bool >
    {
		functor_getter_perms_owner_control(const LLMediaEntry& entry)	: mMediaEntry(entry) {}
        
        bool get( LLViewerObject* object, S32 face )
        {
            if ( object )
                if ( object->getTE(face) )
                    if ( object->getTE(face)->getMediaData() )
                        return (0 != (object->getTE(face)->getMediaData()->getPermsControl() & LLMediaEntry::PERM_OWNER));
            return 0 != ( mMediaEntry.getPermsControl() & LLMediaEntry::PERM_OWNER );
        };
		
		const LLMediaEntry &mMediaEntry;
		
    } func_perms_owner_control(default_media_data);
    identical = selected_objects ->getSelectedTEValue( &func_perms_owner_control, value_bool );
    base_key = std::string( LLPanelContents::PERMS_OWNER_CONTROL_KEY );
    mMediaSettings[ base_key ] = value_bool;
    mMediaSettings[ base_key + std::string( LLPanelContents::TENTATIVE_SUFFIX ) ] = ! identical;
	
    // Perms - group interact
    value_bool = 0 != ( default_media_data.getPermsInteract() & LLMediaEntry::PERM_GROUP );
    struct functor_getter_perms_group_interact : public LLSelectedTEGetFunctor< bool >
    {
		functor_getter_perms_group_interact(const LLMediaEntry& entry): mMediaEntry(entry) {}
        
        bool get( LLViewerObject* object, S32 face )
        {
            if ( object )
                if ( object->getTE(face) )
                    if ( object->getTE(face)->getMediaData() )
                        return (0 != (object->getTE(face)->getMediaData()->getPermsInteract() & LLMediaEntry::PERM_GROUP));
            return 0 != ( mMediaEntry.getPermsInteract() & LLMediaEntry::PERM_GROUP );
        };
		
		const LLMediaEntry &mMediaEntry;
		
    } func_perms_group_interact(default_media_data);
    identical = selected_objects->getSelectedTEValue( &func_perms_group_interact, value_bool );
    base_key = std::string( LLPanelContents::PERMS_GROUP_INTERACT_KEY );
    mMediaSettings[ base_key ] = value_bool;
    mMediaSettings[ base_key + std::string( LLPanelContents::TENTATIVE_SUFFIX ) ] = ! identical;
	
    // Perms - group control
    value_bool = 0 != ( default_media_data.getPermsControl() & LLMediaEntry::PERM_GROUP );
    struct functor_getter_perms_group_control : public LLSelectedTEGetFunctor< bool >
    {
		functor_getter_perms_group_control(const LLMediaEntry& entry): mMediaEntry(entry) {}
        
        bool get( LLViewerObject* object, S32 face )
        {
            if ( object )
                if ( object->getTE(face) )
                    if ( object->getTE(face)->getMediaData() )
                        return (0 != (object->getTE(face)->getMediaData()->getPermsControl() & LLMediaEntry::PERM_GROUP));
            return 0 != ( mMediaEntry.getPermsControl() & LLMediaEntry::PERM_GROUP );
        };
		
		const LLMediaEntry &mMediaEntry;
		
    } func_perms_group_control(default_media_data);
    identical = selected_objects->getSelectedTEValue( &func_perms_group_control, value_bool );
    base_key = std::string( LLPanelContents::PERMS_GROUP_CONTROL_KEY );
    mMediaSettings[ base_key ] = value_bool;
    mMediaSettings[ base_key + std::string( LLPanelContents::TENTATIVE_SUFFIX ) ] = ! identical;
	
    // Perms - anyone interact
    value_bool = 0 != ( default_media_data.getPermsInteract() & LLMediaEntry::PERM_ANYONE );
    struct functor_getter_perms_anyone_interact : public LLSelectedTEGetFunctor< bool >
    {
		functor_getter_perms_anyone_interact(const LLMediaEntry& entry): mMediaEntry(entry) {}
        
        bool get( LLViewerObject* object, S32 face )
        {
            if ( object )
                if ( object->getTE(face) )
                    if ( object->getTE(face)->getMediaData() )
                        return (0 != (object->getTE(face)->getMediaData()->getPermsInteract() & LLMediaEntry::PERM_ANYONE));
            return 0 != ( mMediaEntry.getPermsInteract() & LLMediaEntry::PERM_ANYONE );
        };
		
		const LLMediaEntry &mMediaEntry;
		
    } func_perms_anyone_interact(default_media_data);
    identical = LLSelectMgr::getInstance()->getSelection()->getSelectedTEValue( &func_perms_anyone_interact, value_bool );
    base_key = std::string( LLPanelContents::PERMS_ANYONE_INTERACT_KEY );
    mMediaSettings[ base_key ] = value_bool;
    mMediaSettings[ base_key + std::string( LLPanelContents::TENTATIVE_SUFFIX ) ] = ! identical;
	
    // Perms - anyone control
    value_bool = 0 != ( default_media_data.getPermsControl() & LLMediaEntry::PERM_ANYONE );
    struct functor_getter_perms_anyone_control : public LLSelectedTEGetFunctor< bool >
    {
		functor_getter_perms_anyone_control(const LLMediaEntry& entry)	: mMediaEntry(entry) {}
        
        bool get( LLViewerObject* object, S32 face )
        {
            if ( object )
                if ( object->getTE(face) )
                    if ( object->getTE(face)->getMediaData() )
                        return (0 != (object->getTE(face)->getMediaData()->getPermsControl() & LLMediaEntry::PERM_ANYONE));
            return 0 != ( mMediaEntry.getPermsControl() & LLMediaEntry::PERM_ANYONE );
        };
		
		const LLMediaEntry &mMediaEntry;
		
    } func_perms_anyone_control(default_media_data);
    identical = selected_objects->getSelectedTEValue( &func_perms_anyone_control, value_bool );
    base_key = std::string( LLPanelContents::PERMS_ANYONE_CONTROL_KEY );
    mMediaSettings[ base_key ] = value_bool;
    mMediaSettings[ base_key + std::string( LLPanelContents::TENTATIVE_SUFFIX ) ] = ! identical;
	
    // security - whitelist enable
    value_bool = default_media_data.getWhiteListEnable();
    struct functor_getter_whitelist_enable : public LLSelectedTEGetFunctor< bool >
    {
		functor_getter_whitelist_enable(const LLMediaEntry& entry)	: mMediaEntry(entry) {}
        
        bool get( LLViewerObject* object, S32 face )
        {
            if ( object )
                if ( object->getTE(face) )
                    if ( object->getTE(face)->getMediaData() )
                        return object->getTE(face)->getMediaData()->getWhiteListEnable();
            return mMediaEntry.getWhiteListEnable();
        };
		
		const LLMediaEntry &mMediaEntry;
		
    } func_whitelist_enable(default_media_data);
    identical = selected_objects->getSelectedTEValue( &func_whitelist_enable, value_bool );
    base_key = std::string( LLMediaEntry::WHITELIST_ENABLE_KEY );
    mMediaSettings[ base_key ] = value_bool;
    mMediaSettings[ base_key + std::string( LLPanelContents::TENTATIVE_SUFFIX ) ] = ! identical;
	
    // security - whitelist URLs
    std::vector<std::string> value_vector_str = default_media_data.getWhiteList();
    struct functor_getter_whitelist_urls : public LLSelectedTEGetFunctor< std::vector<std::string> >
    {
		functor_getter_whitelist_urls(const LLMediaEntry& entry): mMediaEntry(entry) {}
        
        std::vector<std::string> get( LLViewerObject* object, S32 face )
        {
            if ( object )
                if ( object->getTE(face) )
                    if ( object->getTE(face)->getMediaData() )
                        return object->getTE(face)->getMediaData()->getWhiteList();
            return mMediaEntry.getWhiteList();
        };
		
		const LLMediaEntry &mMediaEntry;
		
    } func_whitelist_urls(default_media_data);
    identical = selected_objects->getSelectedTEValue( &func_whitelist_urls, value_vector_str );
    base_key = std::string( LLMediaEntry::WHITELIST_KEY );
	mMediaSettings[ base_key ].clear();
    std::vector< std::string >::iterator iter = value_vector_str.begin();
    while( iter != value_vector_str.end() )
    {
        std::string white_list_url = *iter;
        mMediaSettings[ base_key ].append( white_list_url );
        ++iter;
    };
	
    mMediaSettings[ base_key + std::string( LLPanelContents::TENTATIVE_SUFFIX ) ] = ! identical;
}

