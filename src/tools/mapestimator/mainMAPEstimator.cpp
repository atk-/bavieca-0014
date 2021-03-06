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


#include <iostream>
#include <cstdlib>

#include "CommandLineManager.h"
#include "FeatureExtractor.h"
#include "LexiconManager.h"
#include "PhoneSet.h"
#include "HMMManager.h"
#include "MAPEstimator.h"

using namespace std;

#include <string>

using namespace Bavieca;

// main for the HMM estimation tool: "mapestimator"
int main(int argc, char *argv[]) {

	try {

		// (1) define command line parameters  
		CommandLineManager commandLineManager("mapestimator",SYSTEM_VERSION,SYSTEM_AUTHOR,SYSTEM_DATE);
		commandLineManager.defineParameter("-pho","file containing the phonetic symbol set",PARAMETER_TYPE_FILE,false);
		commandLineManager.defineParameter("-mod","input acoustic models",PARAMETER_TYPE_FILE,false);
		commandLineManager.defineParameter("-acc","input accumulator filelist",PARAMETER_TYPE_FILE,false);
		commandLineManager.defineParameter("-pkw","prior knowledge weight",PARAMETER_TYPE_FLOAT,true,"[2|20]","2");	
		commandLineManager.defineParameter("-out","output acoustic models",PARAMETER_TYPE_FILE,false);
		
		// parse the command line parameters
		if (commandLineManager.parseParameters(argc,argv) == false) {
			return -1;
		}
		
		// get the parameters
		const char *strFilePhoneSet = commandLineManager.getParameterValue("-pho");
		const char *strFileModels = commandLineManager.getParameterValue("-mod");
		const char *strFileAccList = commandLineManager.getParameterValue("-acc");
		float fPriorKnowledgeWeight = atof(commandLineManager.getParameterValue("-pkw"));
		const char *strFileModelsOutput = commandLineManager.getParameterValue("-out");
		
		// load the phone set
		PhoneSet phoneSet(strFilePhoneSet);
		phoneSet.load();
			
		// load the HMMs
		HMMManager hmmManager(&phoneSet,HMM_PURPOSE_ESTIMATION);	
		hmmManager.load(strFileModels);
		
		// load the accumulators
		AccMetadata metadata;
		MAccumulatorPhysical mAccumulatorPhysical;
		Accumulator::loadAccumulatorList(strFileAccList,mAccumulatorPhysical,metadata);
		
		// initialize the HMMs 
		hmmManager.initializeEstimation(ACCUMULATOR_TYPE_PHYSICAL,UCHAR_MAX,UCHAR_MAX);
		
		HMMManager &hmmManagerUpdate = hmmManager;
		
		// estimate the HMM parameters
		MAPEstimator mapEstimator(&hmmManagerUpdate);
		mapEstimator.estimateParameters(mAccumulatorPhysical,fPriorKnowledgeWeight);
		
		Accumulator::destroy(mAccumulatorPhysical);
		
		// create the output HMMs
		hmmManagerUpdate.store(strFileModelsOutput);
		
	} catch (ExceptionBase &e) {
	
		std::cerr << e.what() << std::endl;
		return -1;
	}	
	
	return 0;
}
