/** 
 * @file llviewermedia_streamingaudio.h
 * @author Tofu Linden
 * @brief Definition of LLStreamingAudio_MediaPlugins implementation - an implementation of the streaming audio interface which is implemented as a client of the media plugins API.
 *
 * $LicenseInfo:firstyear=2009&license=viewergpl$
 * 
 * Copyright (c) 2009-2010, Linden Research, Inc.
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

#ifndef LL_VIEWERMEDIA_STREAMINGAUDIO_H
#define LL_VIEWERMEDIA_STREAMINGAUDIO_H


#include "stdtypes.h" // from llcommon
#include "llstreamingaudio.h"

class LLPluginClassMedia;

class LLStreamingAudio_MediaPlugins :
	public LLStreamingAudioInterface,
	public LLPluginClassMediaOwner
{
 public:
	LLStreamingAudio_MediaPlugins();
	/*virtual*/ ~LLStreamingAudio_MediaPlugins();

	/*virtual*/ void start(const std::string& url);
	/*virtual*/ void stop();
	/*virtual*/ void pause(int pause);
	/*virtual*/ void update();
	/*virtual*/ int isPlaying();
	/*virtual*/ void setGain(F32 vol);
	/*virtual*/ F32 getGain();
	/*virtual*/ std::string getURL();

	// inherited from LLPluginClassMediaOwner
	/*virtual*/ void handleMediaEvent(LLPluginClassMedia* self, EMediaEvent event);

private:
	LLPluginClassMedia* initializeMedia(const std::string& media_type);

	LLPluginClassMedia *mMediaPlugin;
	std::string mURL;
	F32 mGain;
};



#endif //LL_VIEWERMEDIA_STREAMINGAUDIO_H
