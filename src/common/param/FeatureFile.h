/*---------------------------------------------------------------------------------------------*
 * Copyright (C) 2012 Daniel Bolaños - www.bltek.com - Boulder Language Technologies           *
 *                                                                                             *
 * www.bavieca.org is the website of the Bavieca Speech Recognition Toolkit                    *
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


#ifndef FEATUREFILE_H
#define FEATUREFILE_H

#include <stdio.h>
#include <stdlib.h>

#include <xmmintrin.h>
//#include <smmintrin.h>
//#include <pmmintrin.h>

#if defined __linux__ || defined _WIN32
#include <malloc.h>
#elif __APPLE__
#include <sys/malloc.h>
#else
	#error "unsupported platform"
#endif

using namespace std;

#include <string>
#include "Global.h"

namespace Bavieca {

#define MODE_READ		0
#define MODE_WRITE		1

// default feature dimensionality
#define FEATURE_DIMENSIONALITY_DEFAULT			39

// features file format
#define FORMAT_FEATURES_FILE_DEFAULT			0
#define FORMAT_FEATURES_FILE_HTK				1

/**
	@author daniel <dani.bolanos@gmail.com>
*/
class FeatureFile {

	private:
	
		string m_strFile;						// file name
	#ifdef SIMD		
		__attribute__((aligned(16))) float *m_fFeatureVectors;		// aligned to 16 bytes to speed up memory access
		int m_iDimAligned16;					// dimensionality needed to store a 16 bytes aligned feature vector
	#else
		float *m_fFeatureVectors;			// feature vectors
	#endif
		unsigned int m_iFeatureVectors;	// # feature vectors
		char m_iMode;							// mode
		char m_iFormat;						// file format (default, HTK, etc)
		int m_iDim;								// dimensionality

	public:
    	
    	// constructor
		FeatureFile(const char *strFileName, const char iMode, const char iFormat = FORMAT_FEATURES_FILE_DEFAULT, int iDim = FEATURE_DIMENSIONALITY_DEFAULT);

		// destructor
		~FeatureFile();
		
		// load the features
		void load();
		
		// store the features 
		void store(float *fFeatureVectors, unsigned int iFeatureVectors);
		
		// return a reference to the features
		float *getFeatureVectors(unsigned int *iFeatureVectors);		
		
		// print the features (debugging)
		static void print(float *fFeatures, unsigned int iFeatures, unsigned int iDim, unsigned int iDelta);
};

};	// end-of-namespace


#endif
