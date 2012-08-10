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

#include "AudioFile.h"
#include "FileInput.h"
#include "FileOutput.h"
#include "IOBase.h"

// constructor
AudioFile::AudioFile() {
}

// destructor
AudioFile::~AudioFile() {
}

// load the data
short *AudioFile::load(const char *strFile, int *iSamples) {
	
	try {
		FileInput file(strFile,true);
		file.open();
		
		*iSamples = file.size()/2;
		assert(*iSamples > 0);
		short *sSamples = new short[*iSamples];
		IOBase::readBytes(file.getStream(),reinterpret_cast<char*>(sSamples),*iSamples*sizeof(short));
	
		file.close();
		
		return sSamples;
		
	} catch (ExceptionBase &e) {
		EXCEPTION("unable to load the audio file");
	}	
}

// store the data
void AudioFile::store(const char *strFile, short *sSamples, int iSamples) {

	assert((iSamples >= 0) && (sSamples));

	FileOutput file(strFile,true);
	IOBase::writeBytes(file.getStream(),reinterpret_cast<char*>(sSamples),iSamples*sizeof(short));
	file.close();
}

