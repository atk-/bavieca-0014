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

#include "BaviecaAPI.h"

#include "Viterbi.h"
#include "ViterbiX.h"
#include "AlignmentFile.h"
#include "AudioFile.h"
#include "BatchFile.h"
#include "BestPath.h"
#include "ConfigurationBavieca.h"
#include "ConfigurationFeatures.h"
#include "DynamicNetworkX.h"
#include "DynamicDecoderX.h"
#include "NetworkBuilderX.h"
#include "FeatureExtractor.h"
#include "FeatureFile.h"
#include "FileUtils.h"
#include "FillerManager.h"
#include "HMMManager.h"
#include "LexiconManager.h"
#include "LexUnitsFile.h"
#include "LMManager.h"
#include "LogMessage.h"
#include "PhoneSet.h"
#include "SADModule.h"
#include "TextAligner.h"
#include "TextAlignment.h"
#include <windows.h>

namespace Bavieca {

// constructor (receives default configuration parameters)
BaviecaAPI::BaviecaAPI(const char *strFileConfiguration) {

	// enable this for DLL output into a new console window
	if (false) {
		AllocConsole();
		freopen("CONIN$", "r", stdin);
		freopen("CONOUT$", "w", stdout);
		freopen("CONOUT$", "w", stderr);
	}

	printf("constructor called\n");
	assert(strFileConfiguration);
	m_strFileConfiguration = new char[strlen(strFileConfiguration)+1];
	strcpy(m_strFileConfiguration,strFileConfiguration);
	printf("config file name stored\n");
	m_configuration = NULL;
	m_featureExtractor = NULL;
	m_phoneSet = NULL;
	m_lexiconManager = NULL;
	m_sadModule = NULL;
	m_lmManager = NULL;
	m_viterbiX = NULL;
	m_hmmManager = NULL;
	m_network = NULL;
	m_networkBuilder = NULL;
	m_dynamicDecoder = NULL;
	m_bInitialized = false;
}

// destructor
BaviecaAPI::~BaviecaAPI() {
	delete[] m_strFileConfiguration;
}

BaviecaAPI* ADDCALL BaviecaAPI::create(const char *configFile,
		unsigned char iFlags, ParamValuesI *paramValues, int paramCount) {
	printf("constructing...\n");
	BaviecaAPI* api = new BaviecaAPI(configFile);
	printf("constructed, initializing...\n");
	api->initialize(iFlags, paramValues, paramCount);
	printf("initialization complete!\n");
	return api;
}

// initialize API (overriding parameters as needed)
bool ADDCALL BaviecaAPI::initialize(unsigned char iFlags,
		ParamValuesI *paramValues, unsigned int paramCount) {

	assert(m_bInitialized == false);

	try {

		m_iFlags = iFlags;

		printf("loading config parameters...\n");
		// (1) load configuration parameters
		m_configuration = new ConfigurationBavieca(m_strFileConfiguration);
		m_configuration->load();

		printf("applying overrides...\n");
		// (2) override parameters if needed
		if (paramValues) {
			printf("overriding params (%d in total):\n", paramCount);
			for (unsigned int i = 0; i < paramCount; ++i) {
				ParamValueI *p = paramValues->getParamValue(i);
//				const char *pp = (*p).getParameter();
//				const char *pv = p->getValue();

				printf("%d: %s = %s\n", i,
						paramValues->getParamValue(i)->getParameter(),
						paramValues->getParamValue(i)->getValue());
				m_configuration->setParameterValue(
						paramValues->getParamValue(i)->getParameter(),
						paramValues->getParamValue(i)->getValue());
			}
		}

		printf("reading config...\n");
		// (3) get configuration parameters

		// phone set
		const char *m_strFilePhoneSet = m_configuration->getStrParameterValue(
				"phoneticSymbolSet.file");

		// acoustic models
		const char *m_strFileAcousticModels =
				m_configuration->getStrParameterValue("acousticModels.file");

		// features
		const char *m_strCepstralNormalizationMode =
				m_configuration->getStrParameterValue(
						"feature.cepstralNormalization.mode");
		const char *m_strCepstralNormalizationMethod =
				m_configuration->getStrParameterValue(
						"feature.cepstralNormalization.method");
		int m_iCepstralNormalizationBufferSize =
				m_configuration->getIntParameterValue(
						"feature.cepstralNormalization.bufferSize");
		float m_fWarpFactor = 1.0;
		if (m_configuration->isParameterSet("feature.warpFactor")) {
			m_fWarpFactor = (float) atof(
					m_configuration->getParameterValue("feature.warpFactor"));
		}

		printf("loading phone set from %s...\n", m_strFilePhoneSet);
		// load the phone set
		m_phoneSet = new PhoneSet(m_strFilePhoneSet);
		m_phoneSet->load();

		printf("loading HMMs...\n");
		// load the HMMs used for the estimation
		m_hmmManager = new HMMManager(m_phoneSet, HMM_PURPOSE_EVALUATION);
		m_hmmManager->load(m_strFileAcousticModels);
		m_hmmManager->initializeDecoding();

		printf("loading feature config...\n");
		// load the feature configuration
		const char *m_strFileConfigurationFeatures =
				m_configuration->getStrParameterValue(
						"feature.configurationFile");
		ConfigurationFeatures *configurationFeatures =
				new ConfigurationFeatures(m_strFileConfigurationFeatures);
		configurationFeatures->load();

		// get the feature normalization mode and method
		int m_iCepstralNormalizationMode =
				FeatureExtractor::getNormalizationMode(
						m_strCepstralNormalizationMode);
		int m_iCepstralNormalizationMethod =
				FeatureExtractor::getNormalizationMethod(
						m_strCepstralNormalizationMethod);

		printf("creating feature extractor...");
		// create the feature extractor
		m_featureExtractor = new FeatureExtractor(configurationFeatures,
				m_fWarpFactor, m_iCepstralNormalizationBufferSize,
				m_iCepstralNormalizationMode, m_iCepstralNormalizationMethod);
		m_featureExtractor->initialize();
		printf("done\n");
		delete configurationFeatures;

		// speech activity detection
		if (m_iFlags & INIT_SAD) {

			// make sure required configuration parameters are defined
			if (!m_configuration->areSADParametersSet()) {
				BVC_ERROR
						<< "wrong configuration parameters: not all the required parameters are set";
			}

			// speech activity detection
			int m_iMaxGaussianComponentsSilence =
					m_configuration->getIntParameterValue(
							"sad.maxGaussianSilence");
			int m_iMaxGaussianComponentsSpeech =
					m_configuration->getIntParameterValue(
							"sad.maxGaussianSpeech");
			float m_fPenaltySilenceToSpeech =
					m_configuration->getFloatParameterValue(
							"sad.speechPenalty");
			int m_iFramesPadding = m_configuration->getIntParameterValue(
					"sad.speechPadding");

			m_sadModule = new SADModule(m_phoneSet, m_hmmManager,
					m_iMaxGaussianComponentsSilence,
					m_iMaxGaussianComponentsSpeech, m_fPenaltySilenceToSpeech,
					m_iFramesPadding);
			m_sadModule->initialize();
		}

		// aligner
		if (m_iFlags & INIT_ALIGNER) {
			printf("initializing aligner...\n");

			// make sure required configuration parameters are defined
			if (!m_configuration->areAlignerParametersSet()) {
				BVC_ERROR
						<< "wrong configuration parameters: not all the required parameters are set";
			}

			// lexicon
			const char *m_strFileLexicon =
					m_configuration->getStrParameterValue("lexicon.file");

			// load the lexicon
			m_lexiconManager = new LexiconManager(m_strFileLexicon, m_phoneSet);
			m_lexiconManager->load();

			float fBeamWidth = 1000;
			m_viterbiX = new ViterbiX(m_phoneSet, m_lexiconManager,
					m_hmmManager, fBeamWidth, 2000, true, 2000);
		}

		// decoder
		if (m_iFlags & INIT_DECODER) {
			printf("initializing decoder...\n");

			// make sure required configuration parameters are defined
			if (!m_configuration->areDecodingParametersSet()) {
				BVC_ERROR
						<< "wrong configuration parameters: not all the required parameters are set";
			}

			// lexicon
			if (!m_lexiconManager) {
				const char *m_strFileLexicon =
						m_configuration->getStrParameterValue("lexicon.file");

				// load the lexicon
				m_lexiconManager = new LexiconManager(m_strFileLexicon,
						m_phoneSet);
				m_lexiconManager->load();
			}

			// language model
			const char *m_strLanguageModelFile =
					m_configuration->getStrParameterValue("languageModel.file");
			const char *m_strLanguageModelFormat =
					m_configuration->getStrParameterValue(
							"languageModel.format");
			const char *m_strLanguageModelType =
					m_configuration->getStrParameterValue("languageModel.type");
			float m_fLanguageModelScalingFactor =
					m_configuration->getFloatParameterValue(
							"languageModel.scalingFactor");
			const char *m_strLanguageModelNGram =
					m_configuration->getStrParameterValue(
							"languageModel.ngram");
			int m_iNGram = LMManager::getNGram(m_strLanguageModelNGram);
			//bool m_bLanguageCrossUtterance = 
			//m_configuration->getBoolParameterValue("languageModel.crossUtterance");

			// insertion penalty
			float m_fInsertionPenaltyStandard =
					m_configuration->getFloatParameterValue(
							"insertionPenalty.standard");
			float m_fInsertionPenaltyFiller =
					m_configuration->getFloatParameterValue(
							"insertionPenalty.filler");
			const char *m_strFileInsertionPenaltyFiller =
					m_configuration->getStrParameterValue(
							"insertionPenalty.filler.file");

			// pruning parameters
			int m_iMaxActiveArcs = m_configuration->getIntParameterValue(
					"pruning.maxActiveArcs");
			int m_iMaxActiveArcsWE = m_configuration->getIntParameterValue(
					"pruning.maxActiveArcsWE");
			int m_iMaxActiveTokensArc = m_configuration->getIntParameterValue(
					"pruning.maxActiveTokensArc");
			float m_fBeamWidthArcs = m_configuration->getFloatParameterValue(
					"pruning.likelihoodBeam");
			float m_fBeamWidthArcsWE = m_configuration->getFloatParameterValue(
					"pruning.likelihoodBeamWE");
			float m_fBeamWidthTokensArc =
					m_configuration->getFloatParameterValue(
							"pruning.likelihoodBeamTokensArc");

			// output lattice?
			bool m_bLatticeGeneration = m_configuration->isParameterSet(
					"output.lattice.maxWordSequencesState");
			//m_bLatticeGeneration = false;
			int m_iMaxWordSequencesState = -1;
			if (m_bLatticeGeneration) {
				m_iMaxWordSequencesState =
						m_configuration->getIntParameterValue(
								"output.lattice.maxWordSequencesState");
			}

			// attach insertion penalties to lexical units
			m_lexiconManager->attachLexUnitPenalties(
					m_fInsertionPenaltyStandard, m_fInsertionPenaltyFiller);

			FillerManager m_fillerManager(m_strFileInsertionPenaltyFiller);
			m_fillerManager.load();
			m_fillerManager.attachInsertionPenaltyFillers(m_lexiconManager);

			// load the language model
			m_lmManager = new LMManager(m_lexiconManager,
					m_strLanguageModelFile, m_strLanguageModelFormat,
					m_strLanguageModelType, m_strLanguageModelNGram);
			m_lmManager->load();
			printf("building LM graph...\n");
			m_lmManager->buildLMGraph();

			printf("creating decoding network...\n");
			m_networkBuilder = new NetworkBuilderX(m_phoneSet, m_hmmManager,
					m_lexiconManager);

			// build the decoding network
			m_network = m_networkBuilder->build();
			if (m_network == NULL) {
				BVC_ERROR << "unable to build the network";
			}

			printf("constructing decoder...\n");
			m_dynamicDecoder = new DynamicDecoderX(m_phoneSet, m_hmmManager,
					m_lexiconManager, m_lmManager,
					m_fLanguageModelScalingFactor, m_iNGram, m_network,
					m_iMaxActiveArcs, m_iMaxActiveArcsWE, m_iMaxActiveTokensArc,
					m_fBeamWidthArcs, m_fBeamWidthArcsWE, m_fBeamWidthTokensArc,
					m_bLatticeGeneration, m_iMaxWordSequencesState);
			printf("initializing decoder...\n");
			m_dynamicDecoder->initialize();
		}

		// speaker adaptation
		if (m_iFlags & INIT_ADAPTATION) {

		}

	} catch (ExceptionBase ex) {
		printf("%s\n", ex.what());
		return false;
	}

	m_bInitialized = true;
	return true;
}

// uninitialize the API	
void ADDCALL BaviecaAPI::uninitialize() {

	assert(m_bInitialized);

	delete m_configuration;
	delete m_phoneSet;
	delete m_lexiconManager;
	delete m_hmmManager;
	delete m_featureExtractor;

	if (m_sadModule) {
		delete m_sadModule;
	}
	if (m_viterbiX) {
		delete m_viterbiX;
	}
	if (m_dynamicDecoder) {
		m_dynamicDecoder->uninitialize();
		delete m_dynamicDecoder;
		delete m_network;
		delete m_networkBuilder;
	}
	if (m_lmManager) {
		delete m_lmManager;
	}

	m_configuration = NULL;
	m_featureExtractor = NULL;
	m_phoneSet = NULL;
	m_lexiconManager = NULL;
	m_sadModule = NULL;
	m_lmManager = NULL;
	m_viterbiX = NULL;
	m_hmmManager = NULL;
	m_network = NULL;
	m_networkBuilder = NULL;
	m_dynamicDecoder = NULL;
	m_bInitialized = false;
}

// extract features from an audio file
float ADDCALL *BaviecaAPI::extractFeatures(const char* wavFile,
		unsigned int* iFeatures) {
	return m_featureExtractor->extractFeatures(wavFile, iFeatures);
}

// extract features from the audio
float ADDCALL *BaviecaAPI::extractFeatures(short *sSamples,
		unsigned int iSamples, unsigned int *iFeatures) {

	assert(m_bInitialized);
	printf("is initialized...\n");
	return m_featureExtractor->extractFeaturesStream(sSamples, iSamples,
			iFeatures);
	//return m_featureExtractor->extractFeatures(sSamples,iSamples,iFeatures);
}

float ADDCALL *BaviecaAPI::extractFeaturesStatic(short *sSamples,
		unsigned int iSamples, unsigned int *iFeatures) {
	assert(m_bInitialized);
	return m_featureExtractor->extractFeatures(sSamples, iSamples, iFeatures);
}

float ADDCALL *BaviecaAPI::readFeatures(const char* featureFile, unsigned int *iFeatures) {
	FeatureFile *file = new FeatureFile(featureFile, MODE_READ);
	file->load();
	float *features = file->getFeatureVectors(iFeatures);
	return features;
}

// return feature dimensionality
int ADDCALL BaviecaAPI::getFeatureDim() {

	assert(m_bInitialized);
	assert(m_featureExtractor);
	return m_featureExtractor->getFeatureDimensionality();
}

// free features extracted using extractFeatures(...)
void ADDCALL BaviecaAPI::free(float *fFeatures) {

	delete[] fFeatures;
}

// start a SAD session
void ADDCALL BaviecaAPI::sadBeginSession() {

	assert(m_bInitialized);
	m_featureExtractor->resetStream();
	m_sadModule->beginSession();
}

// terminate a SAD session
void ADDCALL BaviecaAPI::sadEndSession() {

	assert(m_bInitialized);
	m_sadModule->endSession();
}

// proces the given features
void ADDCALL BaviecaAPI::sadFeed(float *fFeatures, unsigned int iFeatures) {

	assert(m_bInitialized);
	m_sadModule->processFeatures(fFeatures, iFeatures);
}

// recover speech segments by doing back-tracking on the grid
SpeechSegmentsI ADDCALL *BaviecaAPI::sadRecoverSpeechSegments() {

	//m_sadModule->printGrid();
	assert(m_bInitialized);
	VSpeechSegment vSpeechSegment;
	m_sadModule->recoverSpeechSegments(vSpeechSegment);
	if (vSpeechSegment.empty()) {
		return NULL;
	}
	vector<SpeechSegmentI*> vSpeechSegmentI;
	int i = 0;
	for (VSpeechSegment::iterator it = vSpeechSegment.begin();
			it != vSpeechSegment.end(); ++it, ++i) {
		vSpeechSegmentI.push_back(
				new SpeechSegmentI((*it)->iFrameStart, (*it)->iFrameEnd));
		delete *it;
	}

	return new SpeechSegmentsI(vSpeechSegmentI);
}

// forced alignment between features and audio
Alignment* ADDCALL BaviecaAPI::align(float *fFeatures, unsigned int iFeatures,
		const char *strText, bool bMultiplePronunciations) {

	assert(m_bInitialized);
	VLexUnit vLexUnit;
	bool bAllKnown;

	if (m_lexiconManager->getLexUnits(strText, vLexUnit, bAllKnown) == false) {
		printf("lex units are NULL\n");
		return NULL;
	} else
		printf("got lexunits\n");

//	printf("vlexunit has %d elements\n", vLexUnit.size());
//	if (vLexUnit.size() > 0) {
//		for (VLexUnit::iterator it = vLexUnit.begin(); it != vLexUnit.end(); it++)
//			printf("%d %d\n", (*it)->iIndex, (*it)->iLexUnit);
//	}

	if (bAllKnown == false) {
		printf("allKnown is false\n");
		return NULL;
	}
	printf("all known ok\n");

	// create the aligner
	double dUtteranceLikelihood;
	int iErrorCode;

	VLexUnit vLexUnitOptional;
	vLexUnitOptional.push_back(m_lexiconManager->getLexUnitSilence());

	printf("processing utterance...");

	// build a new Viterbi aligner if necessary
	if (m_viterbiX == NULL) {
		// TODO parameterize the values
		m_viterbiX = new ViterbiX(m_phoneSet, m_lexiconManager, m_hmmManager,
				1000, 2000, true, 2000);
	}

	Alignment *alignment = m_viterbiX->processUtterance(vLexUnit,
			bMultiplePronunciations, vLexUnitOptional, fFeatures, iFeatures,
			&dUtteranceLikelihood, iErrorCode);
	printf("ok\n");

	if (alignment == NULL) {
		printf("alignment is null\n");
		return NULL;
	}

	return alignment;

//	VPhoneAlignment *vPhoneAlignment = alignment->getPhoneAlignment(m_lexiconManager);
//	if (vPhoneAlignment == NULL) {
//		return NULL;
//	}
//
//	VLexUnitAlignment *vLexUnitAlignment = AlignmentFile::getVLexUnitAlignment(*vPhoneAlignment);
//	assert(vLexUnitAlignment);
//	assert(vLexUnitAlignment->size() >= vLexUnit.size());
//	vector<WordAlignmentI*> vWordAlignmentI;
//	int iPhone = 0;
//	int i=0;
//	for(VLexUnitAlignment::iterator it = vLexUnitAlignment->begin() ; it != vLexUnitAlignment->end() ; ++it, ++i) {
//		const char *strLexUnit = m_lexiconManager->getStrLexUnitPron((*it)->lexUnit->iLexUnitPron);
//		// phone-alignment
//		vector<PhoneAlignmentI*> vPhoneAlignmentI;
//		for(unsigned int j=0 ; j<(*it)->lexUnit->vPhones.size() ; ++j) {
//			const char *strPhone = m_phoneSet->getStrPhone((*vPhoneAlignment)[iPhone]->iPhone);
//			assert(strPhone);
//			PhoneAlignmentI *phoneAlignmentI = new PhoneAlignmentI(strPhone,
//				(*vPhoneAlignment)[iPhone]->iStateBegin[0],
//				(*vPhoneAlignment)[iPhone]->iStateEnd[NUMBER_HMM_STATES-1]);
//			vPhoneAlignmentI.push_back(phoneAlignmentI);
//			++iPhone;
//		}
//		WordAlignmentI *wordAlignmentI = new WordAlignmentI(strLexUnit,(*it)->iBegin,(*it)->iEnd,vPhoneAlignmentI);
//		vWordAlignmentI.push_back(wordAlignmentI);
//	}
//
//	AlignmentFile::destroyPhoneAlignment(vPhoneAlignment);
//	AlignmentFile::destroyLexUnitAlignment(vLexUnitAlignment);
//	delete alignment;
//
//	return new AlignmentI(vWordAlignmentI);
}

AlignmentI* ADDCALL BaviecaAPI::alignI(float *fFeatures, unsigned int iFeatures,
		const char *strText, bool bMultiplePronunciations) {
	Alignment *alignment = align(fFeatures, iFeatures, strText,
			bMultiplePronunciations);

	if (alignment == NULL)
		return NULL;

	VPhoneAlignment *vPhoneAlignment = alignment->getPhoneAlignment(
			m_lexiconManager);
	if (vPhoneAlignment == NULL) {
		return NULL;
	}

	VLexUnitAlignment *vLexUnitAlignment = AlignmentFile::getVLexUnitAlignment(
			*vPhoneAlignment);
	assert(vLexUnitAlignment);
	//assert(vLexUnitAlignment->size() >= vLexUnit.size());
	vector<WordAlignmentI*> vWordAlignmentI;
	int iPhone = 0;
	int i = 0;
	for (VLexUnitAlignment::iterator it = vLexUnitAlignment->begin();
			it != vLexUnitAlignment->end(); ++it, ++i) {
		const char *strLexUnit = m_lexiconManager->getStrLexUnitPron(
				(*it)->lexUnit->iLexUnitPron);
		// phone-alignment
		vector<PhoneAlignmentI*> vPhoneAlignmentI;
		for (unsigned int j = 0; j < (*it)->lexUnit->vPhones.size(); ++j) {
			const char *strPhone = m_phoneSet->getStrPhone(
					(*vPhoneAlignment)[iPhone]->iPhone);
			assert(strPhone);
			PhoneAlignmentI *phoneAlignmentI =
					new PhoneAlignmentI(strPhone,
							(*vPhoneAlignment)[iPhone]->iStateBegin[0],
							(*vPhoneAlignment)[iPhone]->iStateEnd[NUMBER_HMM_STATES
									- 1]);
			vPhoneAlignmentI.push_back(phoneAlignmentI);
			++iPhone;
		}
		WordAlignmentI *wordAlignmentI = new WordAlignmentI(strLexUnit,
				(*it)->iBegin, (*it)->iEnd, vPhoneAlignmentI);
		vWordAlignmentI.push_back(wordAlignmentI);
	}

	AlignmentFile::destroyPhoneAlignment(vPhoneAlignment);
	AlignmentFile::destroyLexUnitAlignment(vLexUnitAlignment);
	delete alignment;

	return new AlignmentI(vWordAlignmentI);

}

Alignment* ADDCALL BaviecaAPI::alignFile(const char *strFeatureFile,
		const char *strText, const char *alignmentFile) {
	FeatureFile *featureFile = new FeatureFile(strFeatureFile, MODE_READ);
	featureFile->load();
	unsigned int iFeatures = -1;
	float *fFeatures = featureFile->getFeatureVectors(&iFeatures);
	Alignment *a = align(fFeatures, iFeatures, strText, true);
	if (a == NULL) {
		printf("warning: alignment was null\n");
	} else {
		a->store(alignmentFile);
	}
	return a;
}

// DECODING ------------------------------------------------------------------------------------------

// signal beginning of utterance
void ADDCALL BaviecaAPI::decBeginUtterance() {
	printf("decBeginUtterance()\n");
	assert(m_bInitialized);
	printf("initialized\n");
	assert(m_iFlags & INIT_DECODER);
	printf("decoding flag set\n");
	m_dynamicDecoder->beginUtterance();
}

// process feature vectors from an utterance
void ADDCALL BaviecaAPI::decProcess(float *fFeatures, unsigned int iFeatures) {

	assert(m_bInitialized);
	assert(m_iFlags & INIT_DECODER);
	m_dynamicDecoder->process(fFeatures, iFeatures);
}

// get decoding results
HypothesisI ADDCALL *BaviecaAPI::decGetHypothesis(
		const char *strFileHypothesisLattice) {
	printf("initialized?\n");
	assert(m_bInitialized);
	printf("yes\n");
	printf("decoding?\n");
	assert(m_iFlags & INIT_DECODER);
	printf("yes\n");

	// best path
	vector<WordHypothesisI*> vWordHypothesisI;
	BestPath *bestPath = m_dynamicDecoder->getBestPath();

	if (bestPath != NULL) {
		printf("BEST PATH:\n");
		bestPath->print(true);

		VLexUnit vLexUnit;
		LBestPathElement *lBestPathElements = bestPath->getBestPathElements();
		// extract standard lexical units with alignment information
		for (LBestPathElement::iterator it = lBestPathElements->begin();
				it != lBestPathElements->end(); ++it) {
			if (m_lexiconManager->isStandard((*it)->lexUnit)) {
				const char *strLexUnit = m_lexiconManager->getStrLexUnitPron(
						(*it)->lexUnit->iLexUnitPron);
				WordHypothesisI *wordHypothesisI = new WordHypothesisI(
						strLexUnit, (*it)->iFrameStart, (*it)->iFrameEnd);
				vWordHypothesisI.push_back(wordHypothesisI);
			}
		}
		delete bestPath;
	}

	// hypothesis lattice
	if (strFileHypothesisLattice) {
		//assert(m_bLatticeGeneration);
		HypothesisLattice *hypothesisLattice =
				m_dynamicDecoder->getHypothesisLattice();
		if (hypothesisLattice != NULL) {
			try {
				// write the text lattice...
				char filename[strlen(strFileHypothesisLattice) + 4];
				strcpy(filename, strFileHypothesisLattice);
				strcat(filename, ".txt");
				hypothesisLattice->store(filename, FILE_FORMAT_TEXT);
				// ...and the binary lattice
				strcpy(filename, strFileHypothesisLattice);
				strcat(filename, ".bin");
				hypothesisLattice->store(filename, FILE_FORMAT_BINARY);
			} catch (ofstream::failure &error) {
				// silently fail to write lattice
			}
			delete hypothesisLattice;
		}
	}

	return new HypothesisI(vWordHypothesisI);
}

// signal end of utterance
void ADDCALL BaviecaAPI::decEndUtterance() {

	assert(m_bInitialized);
	assert(m_iFlags & INIT_DECODER);
	m_dynamicDecoder->endUtterance();
}

/*
// return a word-level assessment given a hypothesis and a reference text
TextAlignmentI ADDCALL *BaviecaAPI::getAssessment(HypothesisI *hypothesisI,
		const char *strReference) {

	// get lexical units from reference (there might be out-of-vocabulary words <unk>)
	VLexUnit vLexUnitRef;
	bool bAllKnown;
	if (m_lexiconManager->getLexUnits(strReference, vLexUnitRef, bAllKnown)
			== false) {
		return NULL;
	}

	// get lexical units from the hypothesis
	VLexUnit vLexUnitHyp;
	for (unsigned int i = 0; i < hypothesisI->size(); ++i) {
		LexUnit *lexUnit = m_lexiconManager->getLexUnitPronunciation(
				hypothesisI->getWordHypothesis(i)->getWord());
		assert(lexUnit);
		vLexUnitHyp.push_back(lexUnit);
	}

	// perform the actual alignment
	TextAligner textAligner(m_lexiconManager);
	TextAlignment *textAlignment = textAligner.align(vLexUnitHyp, vLexUnitRef);
	VTAElement &vTAElement = textAlignment->getAlignment();
	int i = 0;
	vector<TextAlignmentElementI*> vElements;
	int iEvent;
	for (VTAElement::iterator it = vTAElement.begin(); it != vTAElement.end();
			++it, ++i) {
		if ((*it)->iAlignmentEvent == TEXT_ALIGNMENT_EVENT_CORRECT) {
			iEvent = TAE_CORRECT;
		} else if ((*it)->iAlignmentEvent
				== TEXT_ALIGNMENT_EVENT_SUBSTITUTION) {
			iEvent = TAE_SUBSTITUTION;
		} else if ((*it)->iAlignmentEvent == TEXT_ALIGNMENT_EVENT_DELETION) {
			iEvent = TAE_DELETION;
		} else {
			assert((*it)->iAlignmentEvent == TEXT_ALIGNMENT_EVENT_INSERTION);
			iEvent = TAE_INSERTION;
		}
		const char *strWordRef = NULL;
		const char *strWordHyp = NULL;
		if ((*it)->iIndexReference != -1) {
			strWordRef = m_lexiconManager->getStrLexUnit(
					(*it)->iLexUnitReference);
		}
		if ((*it)->iIndexHypothesis != -1) {
			strWordHyp = m_lexiconManager->getStrLexUnit(
					(*it)->iLexUnitHypothesis);
		}
		vElements.push_back(
				new TextAlignmentElementI(iEvent, (*it)->iIndexReference,
						strWordRef, (*it)->iIndexHypothesis, strWordHyp));
	}
	delete textAlignment;

	return new TextAlignmentI(vElements);
}
*/

// feed data into speaker adaptation
void ADDCALL BaviecaAPI::mllrFeed(const char *strReference, float *fFeatures,
		unsigned int iFeatures) {

}

// adapt current acoustic models using fed adaptation data
void ADDCALL BaviecaAPI::mllrAdapt() {

}

void ADDCALL BaviecaAPI::addPathToLattice(HypothesisLattice *lattice,
		const char *featureFile, const char *alignmentFile, bool isBest) {
	Alignment *alignment = Alignment::load(alignmentFile, m_lexiconManager);

	attachLM(lattice);
	attachAM(lattice, featureFile);
	//computePP(lattice);
	lattice->storeTextFormat("lattice-pre.txt");
	lattice->addPath(alignment, isBest);
	alignment->store("align.txt");
	lattice->storeTextFormat("lattice-post.txt");
	computePP(lattice);
}

void ADDCALL BaviecaAPI::rescore(float amScore, float lmScore) {
	HypothesisLattice *lattice = m_dynamicDecoder->getHypothesisLattice();
	rescoreLattice(lattice, amScore, lmScore);
}

BestPath* ADDCALL BaviecaAPI::rescoreLattice(HypothesisLattice *lattice,
		float amFactor, float lmFactor) {
	lattice->setScalingFactors(amFactor, lmFactor);
	BestPath *path = lattice->rescore(RESCORING_METHOD_POSTERIORS);
	lattice->computeConfidenceScore(CONFIDENCE_MEASURE_POSTERIORS);
	printf("1. POSTERIORS\n");
	path->print(true);

	path = lattice->rescore(RESCORING_METHOD_POSTERIORS);
	lattice->computeConfidenceScore(CONFIDENCE_MEASURE_ACCUMULATED);
	printf("2. ACCUMULATED\n");
	path->print(true);

	path = lattice->rescore(RESCORING_METHOD_POSTERIORS);
	lattice->computeConfidenceScore(CONFIDENCE_MEASURE_MAXIMUM);
	printf("3. MAXIMUM\n");
	path->print(true);

	return path;
}

void ADDCALL BaviecaAPI::attachLM(HypothesisLattice *lattice) {
	lattice->attachLMProbabilities(m_lmManager);
	// also attach insertion penalties
	lattice->attachInsertionPenalty(m_lexiconManager);
}

void ADDCALL BaviecaAPI::attachAM(HypothesisLattice *lattice,
		const char *strFeatureFile) {
	FeatureFile featureFile = FeatureFile(strFeatureFile, MODE_READ);
	featureFile.load();
	unsigned int iFeatures = -1;
	printf("writing lattice...");
	lattice->storeTextFormat("lattice.am.txt");
	printf("done\n");
	printf("reading features from %s... ", strFeatureFile);
	float *fFeatures = featureFile.getFeatureVectors(&iFeatures);
	printf("done\n");

	attachAM(lattice, fFeatures, iFeatures);
}

void ADDCALL BaviecaAPI::attachAM(HypothesisLattice *lattice, float *fFeatures,
		unsigned int iFeatures) {
	// HMM marking is required for acoustic model scoring
	printf("attempting hmm marking...");
	printf("is hmm manager null? %d\n", (m_hmmManager == NULL));
	lattice->hmmMarking(m_hmmManager);
	printf("done\n");

	printf("feature count is %d\n", iFeatures);
	printf("constructing Viterbi aligner...\n");
	// create the Viterbi aligner
	Viterbi viterbi(m_phoneSet, m_hmmManager, m_lexiconManager, 1000.0);

	printf("aligning... ");
	if (viterbi.align(fFeatures, iFeatures, lattice) == false) {
		BVC_ERROR
				<< "unable to generate phone-level alignments for the lattice";
	}
	printf("done.\n");
}

void ADDCALL BaviecaAPI::computePP(HypothesisLattice *lattice) {
	lattice->computeForwardBackwardScores(10.0f, 10.0f);
	lattice->computePosteriorProbabilities();
}

void ADDCALL BaviecaAPI::saveFeatures(float* fFeatures, unsigned int iFeatures,
		const char* featureFile) {
	m_featureExtractor->saveFeatures(fFeatures, iFeatures, featureFile);
}

void ADDCALL BaviecaAPI::alignReferenceText(HypothesisLattice *lattice,
		const char *refText) {
	// get lexical units from the string
	VLexUnit *vRef = new VLexUnit();
	bool allKnown = false;
	bool result = m_lexiconManager->getLexUnits(refText, *vRef, allKnown);
	printf("getLexUnits(): %s\n", result ? "true" : "false");

	// get lexical units from the hypothesis lattice
	VLexUnit *vHyp = new VLexUnit();
	BestPath *bestPath = lattice->getBestPath();
	bestPath->getLexUnits(*vHyp);

	// perform the alignment
	TextAligner aligner = TextAligner(m_lexiconManager);
	TextAlignment *alignment = aligner.align(*vHyp, *vRef);

	printf("store alignment\n");
	alignment->store("alignment.txt", true);
}

HypothesisLattice* ADDCALL BaviecaAPI::getHypothesisLattice() {
	return m_dynamicDecoder->getHypothesisLattice();
}

void ADDCALL BaviecaAPI::storeLatticeAsText(HypothesisLattice *lattice,
		const char *filename) {
	lattice->storeTextFormat(filename);
}

}
;
// end-of-namespace

