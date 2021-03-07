/**
 * Copyright (C) 2015 Roman Hiestand
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "R3DFontHandler.h"

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include <sstream>

R3DFontHandler R3DFontHandler::instance_;

R3DFontHandler::R3DFontHandler()
	: initialized_(false)
{
}

R3DFontHandler::~R3DFontHandler()
{
}

void R3DFontHandler::initialize()
{
	if(!initialized_)
	{
		/*wxString fontPath(Regard3DSettings::getInstance().getFontFilename());

		if(!wxFileName::FileExists(fontPath))
		{
			wxFileName fontFN(wxStandardPaths::Get().GetResourcesDir(), wxT("DejaVuSans.ttf"));
			fontPath = fontFN.GetFullPath();
		}*/

		// TODO: Make configurable
		boost::filesystem::path fontPath("C:/Projects/Regard3D/thirdparty/font/dejavu-fonts-ttf-2.34/ttf/DejaVuSans.ttf");

		if(boost::filesystem::is_regular_file(fontPath))
		{
			boost::filesystem::ifstream ifs(fontPath, std::ios::binary);
			if(ifs.good())
			{
				font_ = osgText::readFontStream(ifs);
			}
		}
		initialized_ = true;
	}
}
