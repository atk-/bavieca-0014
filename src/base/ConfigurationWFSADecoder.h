/*---------------------------------------------------------------------------------------------*
 * Copyright (C) 2012 Daniel Bolaños - www.bltek.com - Boulder Language Technologies           *
 *                                                                                             *
 * Licensed under the Apache License, Version 2.0 (the "License");                             *
 * you may not use this file except in compliance with the License.                            *
 * You may obtain a copy of the License at                                                     *
 *                                                                                             *
 *         http://www.apache.org/licenses/LICENSE-2.0                                          *
 *                                                                                             *
 * Unless required by applicable law or agreed to in writing, software                         *
 * distributed under the License is distributed on an "AS IS" BASIS,                           *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.                    *
 * See the License for the specific language governing permissions and                         *
 * limitations under the License.                                                              *
 *---------------------------------------------------------------------------------------------*/

#ifndef CONFIGURATIONWFSADECODER_H
#define CONFIGURATIONWFSADECODER_H

#include "Global.h"
#include "ConfigurationFile.h"
#include "ParameterManager.h"

/**
	@author daniel <dani.bolanos@gmail.com>
*/
class ConfigurationWFSADecoder : public ParameterManager, public ConfigurationFile {

	private:
	
		// parameters
		string m_strFile;
		
		// define the parameters
		void defineParameters();

	public:

		// constructor
		ConfigurationWFSADecoder(const char *strFile);

		// destructor
		~ConfigurationWFSADecoder();
		
		// load the configuration parameters
		void load();
		
};

#endif
