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

#ifndef BAVIECAAPI_H
#define BAVIECAAPI_H

using namespace std;

#include "Alignment.h"
#include "BestPath.h"
#include <string>
#include <vector>

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#ifdef BUILDDLL
	#define ADDAPI __declspec(dllexport)
#else
	#define ADDAPI __declspec(dllimport)
#endif

#define ADDCALL __stdcall
#else
//defined __linux__ || defined __APPLE__ || defined SWIG
	#define ADDAPI
	#define ADDCALL
#endif

namespace Bavieca {

class Alignment;
class BestPath;
class ConfigurationBavieca;
class DynamicDecoderX;
class DynamicNetworkX;
class FeatureExtractor;
class FeatureTransformList;
class FileUtils;
class FillerManager;
class HMMManager;
class HypothesisLattice;
class LexiconManager;
class LMManager;
class NetworkBuilderX;
class PhoneSet;
class SADModule;
class ViterbiX;

// initialization modes
enum {INIT_SAD=1, INIT_ALIGNER=2, INIT_DECODER=4, INIT_ADAPTATION=8};

// text alignment events
enum {TAE_CORRECT=1, TAE_SUBSTITUTION=2, TAE_DELETION=4, TAE_INSERTION=8};

// helper classes
class AlignmentI;
class TextAlignmentI;
class ParamValuesI;
class PhoneAlignmentI;
class WordAlignmentI;
class AlignmentI;
class HypothesisI;
class WordHypothesisI;
class SpeechSegmentsI;


/**
	@author daniel <dani.bolanos@gmail.com>
*/

class WordHypothesisI {

	private:

		string m_strWord;
		int m_iFrameStart;
		int m_iFrameEnd;

	public:

		WordHypothesisI(const char *strWord, int iFrameStart, int iFrameEnd) {

			m_strWord = (strWord) ? strWord : "";
			m_iFrameStart = iFrameStart;
			m_iFrameEnd = iFrameEnd;
		}

		~WordHypothesisI() {
		}

		const char *getWord() {
			return m_strWord.c_str();
		}
		int getFrameStart() {
			return m_iFrameStart;
		}
		int getFrameEnd() {
			return m_iFrameEnd;
		}
};

class HypothesisI {

	friend class BaviecaAPI;

	private:

		vector<WordHypothesisI*> m_vWordHypothesisI;

	public:

		HypothesisI(vector<WordHypothesisI*> &vWordHypothesisI) {
			m_vWordHypothesisI.assign(vWordHypothesisI.begin(),vWordHypothesisI.end());
		}

		~HypothesisI() {
			for(vector<WordHypothesisI*>::iterator it = m_vWordHypothesisI.begin() ; it != m_vWordHypothesisI.end() ; ++it) {
				delete *it;
			}
			m_vWordHypothesisI.clear();
		}

		unsigned int size() {
			return (unsigned int)m_vWordHypothesisI.size();
		}

		WordHypothesisI *getWordHypothesis(unsigned int iWord) {
			if (iWord < size()) {
				return m_vWordHypothesisI[iWord];
			}
			return NULL;
		}
};

class BaviecaAPI {

	private:
	
		unsigned char m_iFlags;					// initialization mode
		char *m_strFileConfiguration;
		bool m_bInitialized;
		
		ConfigurationBavieca *m_configuration;
		PhoneSet *m_phoneSet;
		LexiconManager *m_lexiconManager;
		SADModule *m_sadModule;
		HMMManager *m_hmmManager;
		FeatureExtractor *m_featureExtractor;
		LMManager *m_lmManager;
		ViterbiX *m_viterbiX;
		DynamicNetworkX *m_network;
		NetworkBuilderX *m_networkBuilder;
		DynamicDecoderX *m_dynamicDecoder;
		bool m_bLatticeGeneration;

	public:

		// constructor (receives default configuration parameters)
		ADDAPI BaviecaAPI(const char *strFileConfiguration);

		// destructor
		ADDAPI ~BaviecaAPI();

//		ADDAPI static const char* ADDCALL foo(const char* const* values, unsigned int vsize) {
//			printf("foo() called with vector size %d\n", vsize);
//			std::vector<std::string> v(values, values + vsize);
////			std::vector<WordHypothesisI*> *vhyp = new std::vector<WordHypothesisI*>();
//			const char* ret[3];
//
//			for ( unsigned int i=0; i < vsize; i++ ) {
//				printf("%s\n", v[i].c_str());
//				//vhyp->push_back(new WordHypothesisI(v[i].c_str(), i, i+1));
//				//ret->push_back(std::string(v[i].rbegin(), v[i].rend()).c_str());
//				//ret->push_back(v[i]);
//				ret[i] = v[i].c_str();
//			}
//
////			HypothesisI* ret = new HypothesisI(*vhyp);
//			return *ret;
//		}
		
		static void bar(const char* str) {
			printf("api says '%s'\n", str);
		}

		// factory method
		ADDAPI static BaviecaAPI* ADDCALL create(const char *configFile, unsigned char iFlags, ParamValuesI *paramValues = NULL, int paramCount = 0);

		// API INITIALIZATION ------------------------------------------------------------------------------------
		
		// initialize API (overriding parameters as needed)
		ADDAPI bool ADDCALL initialize(unsigned char iFlags, ParamValuesI *paramValues = NULL, unsigned int paramCount = 0);
		
		// uninitialize the API	
		ADDAPI void ADDCALL uninitialize();
		
		// FEATURE EXTRACTION -------------------------------------------------------------------------------------
		ADDAPI float ADDCALL *extractFeatures(const char* wavFile, unsigned int* iFeatures);
		
		// extract features from the audio in stream mode
		ADDAPI float ADDCALL *extractFeatures(short *sSamples, unsigned int iSamples, unsigned int *iFeatures);

		// extract static features from the audio
		ADDAPI float ADDCALL *extractFeaturesStatic(short *sSamples, unsigned int iSamples, unsigned int *iFeatures);
		
		// return feature dimensionality
		ADDAPI int ADDCALL getFeatureDim();
		
		// free features extracted using extractFeatures(...)
		ADDAPI void ADDCALL free(float *fFeatures);

		// save features to file
		ADDAPI void ADDCALL saveFeatures(float *fFeatures, unsigned int iFeatures, const char *featureFile);
		
		// read features from a file
		ADDAPI float ADDCALL *readFeatures(const char* featureFile, unsigned int* iFeatures);

		// SPEECH ACTIVITY DETECTION ------------------------------------------------------------------------------
		
		// start a SAD session
		ADDAPI void ADDCALL sadBeginSession();
		
		// terminate a SAD session
		ADDAPI void ADDCALL sadEndSession();
		
		// proces the given features
		ADDAPI void ADDCALL sadFeed(float *fFeatures, unsigned int iFeatures);
		
		// recover speech segments by doing back-tracking on the grid
		ADDAPI SpeechSegmentsI ADDCALL *sadRecoverSpeechSegments();
		
		// FORCED ALIGNMENT --------------------------------------------------------------------------------------
		
		// forced alignment between features and audio
		ADDAPI Alignment ADDCALL *align(float *fFeatures, unsigned int iFeatures, const char *strText, bool bMultiplePronunciations);

		ADDAPI AlignmentI ADDCALL *alignI(float *fFeatures, unsigned int iFeatures, const char *strText,
			bool bMultiplePronunciations);

		ADDAPI Alignment ADDCALL *alignFile(const char *featureFile, const char *strText, const char *alignmentFile);

		// DECODING ----------------------------------------------------------------------------------------------
		
		// signals beginning of utterance
		ADDAPI void ADDCALL decBeginUtterance();
		
		// process feature vectors from an utterance
		ADDAPI void ADDCALL decProcess(float *fFeatures, unsigned int iFeatures);
		
		// get decoding results
		ADDAPI HypothesisI ADDCALL *decGetHypothesis(const char *strFileHypothesisLattice = NULL);
		
		// signals end of utterance
		ADDAPI void ADDCALL decEndUtterance();
		
		// return a word-level assessment given a hypothesis and a reference text
		//ADDAPI TextAlignmentI ADDCALL *getAssessment(HypothesisI *hypothesis, const char *strReference);
				
		// SPEAKER ADAPTATION ------------------------------------------------------------------------------------
		
		// feed data into speaker adaptation
		ADDAPI void ADDCALL mllrFeed(const char *strReference, float *fFeatures, unsigned int iFeatures);
		
		// adapt current acoustic models using fed adaptation data
		ADDAPI void ADDCALL mllrAdapt();

		// LATTICE TOOLS -----------------------------------------------------------------------------------------

		// add a new path into the lattice
		ADDAPI void ADDCALL addPathToLattice(HypothesisLattice *lattice, const char *featureFile, const char *alignmentFile, bool isBest);

		// rescore the lattice (lattice is recreated by the decoder)
		ADDAPI void ADDCALL rescore(float amScore, float lmScore);

		// rescore the given lattice
		ADDAPI BestPath* ADDCALL rescoreLattice(HypothesisLattice *lattice, float amFactor, float lmFactor);

		// attach LM values
		ADDAPI void ADDCALL attachLM(HypothesisLattice *lattice);

		// attach AM values
		ADDAPI void ADDCALL attachAM(HypothesisLattice *lattice, const char *strFeatureFile);
		ADDAPI void ADDCALL attachAM(HypothesisLattice *lattice, float *fFeatures, unsigned int iFeatures);

		// compute posterior probabilities
		ADDAPI void ADDCALL computePP(HypothesisLattice *lattice);

		ADDAPI HypothesisLattice* ADDCALL getHypothesisLattice();

		ADDAPI void ADDCALL storeLatticeAsText(HypothesisLattice *lattice, const char *filename);

		ADDAPI void ADDCALL alignReferenceText(HypothesisLattice *lattice, const char *refText);

};

// helper-classes

class SpeechSegmentI {

	private:

		int m_iFrameStart;     // starting time frame (zero-based hundredths of a second)
		int m_iFrameEnd;       // ending time frame (zero-based hundredths of a second)

	public:

		SpeechSegmentI(int iFrameStart, int iFrameEnd) {

			m_iFrameStart = iFrameStart;
			m_iFrameEnd = iFrameEnd;
		}

		int getFrameStart() {
			return m_iFrameStart;
		}
		int getFrameEnd() {
			return m_iFrameEnd;
		}
};

class SpeechSegmentsI {

	friend class BaviecaAPI;

	private: 

		vector<SpeechSegmentI*> m_vSpeechSegmentsI;

		SpeechSegmentsI(vector<SpeechSegmentI*> &vSpeechSegmentsI) {			
			m_vSpeechSegmentsI.assign(vSpeechSegmentsI.begin(),vSpeechSegmentsI.end());
		}

	public:

		~SpeechSegmentsI() {
			for(vector<SpeechSegmentI*>::iterator it = m_vSpeechSegmentsI.begin() ; it != m_vSpeechSegmentsI.end() ; ++it) {
				delete *it;
			}
			m_vSpeechSegmentsI.clear();
		}

		unsigned int size() {
			return (unsigned int)m_vSpeechSegmentsI.size();
		}
	
		SpeechSegmentI *getSpeechSegment(unsigned int iSegment) {
			if (iSegment < size()) {
				return m_vSpeechSegmentsI[iSegment];
			}
			return NULL;
		}
};

// configuration parameters (for overriding)
class ParamValueI {

	friend class BaviecaAPI;

	private:
	
		string m_strParameter;
		string m_strValue;

	public:

		ParamValueI(const char *strParameter, const char *strValue) {
				
			assert((strParameter) && (strValue)); 
			m_strParameter = strParameter;
			m_strValue = strValue;
		}

		const char *getParameter() {
			return m_strParameter.c_str();
		}
		const char *getValue() {
			return m_strValue.c_str();
		}
};

class ParamValuesI {

	friend class BaviecaAPI;

	private: 

		vector<ParamValueI*> m_vParamValuesI;

	public:
		ParamValuesI(vector<ParamValueI*> &vParamValuesI) {			
			m_vParamValuesI.assign(vParamValuesI.begin(),vParamValuesI.end());
		}

		~ParamValuesI() {
			for(vector<ParamValueI*>::iterator it = m_vParamValuesI.begin() ; it != m_vParamValuesI.end() ; ++it) {
				delete *it;
			}
			m_vParamValuesI.clear();
		}

		unsigned int size() {
			return (unsigned int)m_vParamValuesI.size();
		}

		ParamValueI *getParamValue(unsigned int i) {
			if (i < size()) {
				return m_vParamValuesI[i];
			}
			return NULL;
		}
};

// phone alignment
class PhoneAlignmentI {

	private:

		string m_strPhone;
		int m_iFrameStart;
		int m_iFrameEnd;

	public:

		PhoneAlignmentI(const char *strPhone, int iFrameStart, int iFrameEnd) {
			m_strPhone = strPhone ? strPhone : "";
			m_iFrameStart = iFrameStart;
			m_iFrameEnd = iFrameEnd;
		}

		const char *getPhone() {
			return m_strPhone.c_str();
		}
		int getFrameStart() {
			return m_iFrameStart;
		}
		int getFrameEnd() {
			return m_iFrameEnd;
		}
};

// word alignmnet
class WordAlignmentI {

	friend class AlignmentI;
	friend class BaviecaAPI;

	private:

		string m_strWord;								// word
		int m_iFrameStart;								// start time frame
		int m_iFrameEnd;								// end time frame
		vector<PhoneAlignmentI*> m_vPhoneAlignmentI;	// phone alignments

		WordAlignmentI(const char *strWord, int iFrameStart, int iFrameEnd, vector<PhoneAlignmentI*> &vPhoneAlignmentI) {
			m_strWord = strWord ? strWord : "";
			m_iFrameStart = iFrameStart;
			m_iFrameEnd = iFrameEnd;
			m_vPhoneAlignmentI.assign(vPhoneAlignmentI.begin(),vPhoneAlignmentI.end());
		}

	public:

		~WordAlignmentI() {
			for(vector<PhoneAlignmentI*>::iterator it = m_vPhoneAlignmentI.begin() ; it != m_vPhoneAlignmentI.end() ; ++it) {
				delete *it;
			}
			m_vPhoneAlignmentI.clear();
		}

		const char *getWord() {
			return m_strWord.c_str();
		}
		int getFrameStart() {
			return m_iFrameStart;
		}
		int getFrameEnd() {
			return m_iFrameEnd;
		}
		unsigned int size() {
			return (unsigned int)m_vPhoneAlignmentI.size();
		}

		PhoneAlignmentI *getPhoneAlignment(unsigned int iPhone) {
			if (iPhone < size()) {
				return m_vPhoneAlignmentI[iPhone];
			} 
			return NULL;			
		}
};

class AlignmentI {

	friend class BaviecaAPI;

	private:

		vector<WordAlignmentI*> m_vWordAlignmentI;

		AlignmentI(vector<WordAlignmentI*> &vWordAlignmentI) {	
			m_vWordAlignmentI.assign(vWordAlignmentI.begin(),vWordAlignmentI.end());
		}

	public:

		~AlignmentI() {
			for(vector<WordAlignmentI*>::iterator it = m_vWordAlignmentI.begin() ; it != m_vWordAlignmentI.end() ; ++it) {
				delete *it;
			}
			m_vWordAlignmentI.clear();
		}

		unsigned int size() {
			return (unsigned int)m_vWordAlignmentI.size();
		}
	
		WordAlignmentI *getWordAlignment(unsigned int iWord) {

			if (iWord < size()) {
				return m_vWordAlignmentI[iWord];
			}
			return NULL;
		}
};

// text alignment element
class TextAlignmentElementI {

	private:

		unsigned char m_iEvent;
		int m_iIndexRef;
		string m_strWordRef;
		int m_iIndexHyp;
		string m_strWordHyp;

	public:

		TextAlignmentElementI(unsigned char iEvent, int iIndexRef, const char *strWordRef, int iIndexHyp, const char *strWordHyp) {
			m_iEvent = iEvent;
			m_iIndexRef = iIndexRef;
			m_strWordRef = (strWordRef) ? strWordRef : "";
			m_iIndexHyp = iIndexHyp;
			m_strWordHyp = (strWordHyp) ? strWordHyp : "";
		}

		unsigned char getEvent() {
			return m_iEvent;
		}
		unsigned int getIndexRef() {
			return m_iIndexRef;
		}
		const char *getWordRef() {
			return m_strWordRef.c_str();
		}
		unsigned int getIndexHyp() {
			return m_iIndexHyp;
		}
		const char *getWordHyp() {
			return m_strWordHyp.c_str();
		}
};

// text alignment
class TextAlignmentI {

	friend class BaviecaAPI;

	private:

		std::vector<TextAlignmentElementI*> m_vElements;

		TextAlignmentI(vector<TextAlignmentElementI*> &vElements) {
			m_vElements.assign(vElements.begin(),vElements.end());
		}

	public:

		~TextAlignmentI() {
			for(std::vector<TextAlignmentElementI*>::iterator it = m_vElements.begin() ; it != m_vElements.end() ; ++it) {
				delete *it;
			}
			m_vElements.clear();
		}

		unsigned int size() {
			return (unsigned int)m_vElements.size();
		}

		TextAlignmentElementI *getElement(unsigned int iElement) {
			if (iElement < size()) {
				return m_vElements[iElement];
			}
			return NULL;
		}
};

};	// end-of-namespace

#endif

#ifdef __cplusplus
}
#endif
