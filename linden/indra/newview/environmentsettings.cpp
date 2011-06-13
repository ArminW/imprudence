//Copyright (C)2011 Armin.Weatherwax at googlemail dot com
//License: gpl v2 +floss exception as in Imprudence or at your choice lgpl v2.1 as in Kokua 

#include "llviewerprecompiledheaders.h"

#include "environmentsettings.h"
#include "llsdutil.h"
#include "llsdserialize.h"
#include "llhttpclient.h"
#include "llcallbacklist.h"
#include "llagent.h"
#include "llviewerregion.h"
#include "llwaterparammanager.h"
#include "llwlparammanager.h"
#include "llwldaycycle.h"

class EnvironmentSettingsResponder : public LLHTTPClient::Responder
{
public:

	virtual void result(const LLSD& content)
	{
/*
		std::string filename = "gDirUtilp->getExpandedFilename( LL_PATH_USER_SETTINGS ,"windlight/region","regionwl.";
		filename += gAgent.getRegion()->getName();
		filename += ".xml";

		llstat s;
		if(LLFile::stat(filename, &s))//only save if not already there
		{
			llofstream out(filename);
			if (!out.good())
			{
				llwarns << "Unable to open " << filename << " for output." << llendl;
				return;
			}
			LLSDSerialize::toPrettyXML(content, out);
			out.close();
		}
*/
		if( 4 != content.size() ) return;//accept 3 leave out water?
		LLSD::array_const_iterator it = content.beginArray();
		LLSD message = *it++;
		LLSD day_cycle = *it++;
		LLSD skys = *it++;
		LLSD water = *it;
		LLUUID region_id = message["regionID"];
		LLViewerRegion* region = gAgent.getRegion();

		if( region_id != region->getRegionID() )
	 	{
			llwarns << "wrong region" << llendl;
			return;
		}
		llwarns << "set water" << llendl;
		LLWaterParamManager* water_mgr = LLWaterParamManager::instance();
		std::string water_name = "RegionWater_";
		water_name.append(region->getName());
		water_mgr->loadPresetFromRegion(water_name, water, true);

		LLWLParamManager* wl_mgr = LLWLParamManager::instance();
		LLSD::map_const_iterator sky_it = skys.beginMap();
		LLSD::map_const_iterator sky_end = skys.endMap();
		for(;sky_it != sky_end; sky_it++)
		{
			LL_DEBUGS("EnvironmentSettings") << "preset:" <<(sky_it->first) <<"\n" 
			<< ll_pretty_print_sd(sky_it->second) << llendl;
			wl_mgr->loadPresetFromRegion(sky_it->first, sky_it->second, true);
		}
		wl_mgr->mDay.loadRegionDayCycle(day_cycle);
		wl_mgr->resetAnimator(0.5, true);
		wl_mgr->mAnimator.mUseLindenTime = true;
	}

	virtual void error(U32 status, const std::string& reason)
	{
		llwarns << "EnvironmentSettings::error("
		<< status << ": " << reason << ")" << llendl;
	}
};

IMPEnvironmentSettings::IMPEnvironmentSettings()
{
	mRegionID.setNull();
}

void IMPEnvironmentSettings::init()
{
	gIdleCallbacks.addFunction(idle, this);
}

void IMPEnvironmentSettings::idle(void* user_data)
{
	IMPEnvironmentSettings* self = (IMPEnvironmentSettings*)user_data;

	LLViewerRegion* region = gAgent.getRegion();
	if(region && region->capabilitiesReceived())
	{
		LLUUID region_id = region->getRegionID();
		if( region_id != self->mRegionID)
		{
			self->mRegionID = region_id;
			self->getEnvironmentSettings();
		}
	}
}

void IMPEnvironmentSettings::getEnvironmentSettings()
{
	std::string url = gAgent.getRegion()->getCapability("EnvironmentSettings");
	if (!url.empty())
	{
 		LLHTTPClient::get(url, new EnvironmentSettingsResponder);
	}
}
