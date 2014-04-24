/*
 * BaviecaNET.cpp
 *
 *  Created on: 6.2.2014
 *      Author: ATK
 */

using namespace std;

#include "BaviecaNET.h"
#include <windows.h>

void ADDCALL foo(void) {
	bar("hello world");
}

const char* ADDCALL bar(const char* str) {
	printf("%s\n", str);

	return mstr(str);
}

///// CREATION, INITIALIZATION AND UNINITIALIZATION

Bavieca::BaviecaAPI* ADDCALL create(const char *configFile) {
	return new Bavieca::BaviecaAPI(configFile);
}

Bavieca::BaviecaAPI* ADDCALL initialize(const char* configFile, unsigned char iFlags,
		Bavieca::ParamValueI* paramValues, unsigned int paramCount) {
	printf("iflags: %d\n", iFlags);
	printf("param count: %d\n", paramCount);

	vector<Bavieca::ParamValueI*>* pVector = new vector<Bavieca::ParamValueI*>();
	for (unsigned int i=0; i < paramCount; i++) {
		pVector->push_back(&paramValues[i]);
		printf("%d\n", i);
	}

	Bavieca::ParamValuesI* pv = new Bavieca::ParamValuesI(*pVector);
	Bavieca::BaviecaAPI* api = new Bavieca::BaviecaAPI(configFile);
	bool b = api->initialize(iFlags, pv, paramCount);
	assert(b == true);
	return api;
}

void ADDCALL uninitialize(Bavieca::BaviecaAPI* api) {
	api->uninitialize();
}

///// FEATURE EXTRACTION

float* ADDCALL extractFeaturesFromFile(Bavieca::BaviecaAPI* api, const char* wavFile, unsigned int* iFeatures) {
	return api->extractFeatures(wavFile, iFeatures);
}

float* ADDCALL extractFeatures(Bavieca::BaviecaAPI* api, short* sSamples, unsigned int iSamples, unsigned int* iFeatures) {
	printf("extracting features (sample count %d)\n", iSamples);
	float* features = api->extractFeatures(sSamples, iSamples, iFeatures);
	printf("extracted %d features\n", *iFeatures);
	return features;
}

void ADDCALL freeFeatures(Bavieca::BaviecaAPI* api, float *features) {
	api->free(features);
}

///// SAD SESSION FUNCTIONS

void ADDCALL sadBeginSession(Bavieca::BaviecaAPI* api) {
	api->sadBeginSession();
}

void ADDCALL sadEndSession(Bavieca::BaviecaAPI* api) {
	api->sadEndSession();
}

void ADDCALL sadFeed(Bavieca::BaviecaAPI* api, float* features, unsigned int iFeatures) {
	api->sadFeed(features, iFeatures);
}

void ADDCALL freeSpeechSegments(Bavieca::BaviecaAPI* api, Bavieca::SpeechSegmentI* segments) {
	// TODO !
}

Bavieca::SpeechSegmentI* sadRecoverSpeechSegments(Bavieca::BaviecaAPI* api, int* iSegments) {
	// this should write the size of returned list into the iSegments parameter!
	Bavieca::SpeechSegmentsI* segs = api->sadRecoverSpeechSegments();
	*iSegments = segs->size();

	// return in array form
	Bavieca::SpeechSegmentI* ret[segs->size()];
	for (unsigned int i=0; i < segs->size(); i++) {
		ret[i] = segs->getSpeechSegment(i);
	}
	return *ret;
}

///// ALIGNMENT

sWordAlignment* ADDCALL align(Bavieca::BaviecaAPI* api, float* fFeatures, unsigned int iFeatures, const char* strText, unsigned int* iWords, bool bMultiple) {
	// call alignment function
	Bavieca::AlignmentI* alignment = api->alignI(fFeatures, iFeatures, strText, bMultiple); // TODO the boolean flag is not given

	// write the total word count
	*iWords = alignment->size();

	sWordAlignment* ret = new sWordAlignment[*iWords];
	for (unsigned int i = 0; i < *iWords; i++) {
		Bavieca::WordAlignmentI* a = alignment->getWordAlignment(i);
		ret[i].fStart = a->getFrameStart();
		ret[i].fEnd = a->getFrameEnd();
		ret[i].word = new char[strlen(a->getWord()) + 1];
		strcpy(ret[i].word, a->getWord());

		// copy phone alignments
		ret[i].phoneAlignment = new sPhoneAlignment[a->size()];
		for (unsigned int j = 0; j < a->size(); j++) {
			ret[i].phoneAlignment[j].fStart = a->getFrameStart();
			ret[i].phoneAlignment[j].fEnd = a->getFrameEnd();
			ret[i].phoneAlignment[j].phone = new char[strlen(a->getWord()) + 1];
			strcpy(ret[i].phoneAlignment[j].phone, a->getWord());
		}
	}
	return ret;
}

void ADDCALL alignFile(Bavieca::BaviecaAPI* api, const char *featureFile, const char *strText, const char *alignmentFile) {
	printf("**1\n");
	Bavieca::Alignment *alignment = api->alignFile(featureFile, strText, alignmentFile);
	printf("**2\n");
}


void ADDCALL freeWordAlignments(Bavieca::BaviecaAPI* api, Bavieca::WordAlignmentI* alignments, unsigned int iWords) {
	// TODO !
}

///// DECODING FUNCTIONS

void ADDCALL decBeginUtterance(Bavieca::BaviecaAPI* api) {
	api->decBeginUtterance();
}

void ADDCALL decEndUtterance(Bavieca::BaviecaAPI* api) {
	api->decEndUtterance();
}

void ADDCALL decProcess(Bavieca::BaviecaAPI* api, float* fFeatures, unsigned int iFeatures) {
	api->decProcess(fFeatures, iFeatures);
}

sWordHypothesis* ADDCALL decGetHypothesis(Bavieca::BaviecaAPI* api, int *iWords) {
	Bavieca::HypothesisI* hyp = api->decGetHypothesis("lattice");

	// unwrap the vector into an array
	//Bavieca::WordHypothesisI* ret[hyp->size()];
	sWordHypothesis* ret = NULL;
	*iWords = hyp->size();
	ret = new sWordHypothesis[hyp->size()];

	for (unsigned int i=0; i < hyp->size(); i++) {
		Bavieca::WordHypothesisI* h = hyp->getWordHypothesis(i);
		ret[i].word = new char[strlen(h->getWord()) + 1];
		strcpy(ret[i].word, h->getWord());
		ret[i].fStart = h->getFrameStart();
		ret[i].fEnd = h->getFrameEnd();
	}

	return ret;
}

void ADDCALL freeWordHypothesis(Bavieca::BaviecaAPI* api, Bavieca::WordHypothesisI* wordHypothesis, unsigned int iWords) {
	// TODO
	printf("free word hypotheses here\n");
}

sTAElement* ADDCALL getAssessment(Bavieca::BaviecaAPI* api, string* words, unsigned int iWordHyps,
		const char* strReference, int* iElements) {
	// this is very unwieldy but we can straighten it out later...

	// 1. embed word strings into a vectorful of Bavieca objects
	std::vector<Bavieca::WordHypothesisI*> *vhyp = new vector<Bavieca::WordHypothesisI*>();

	for (unsigned int i = 0; i < iWordHyps; i++) {
		Bavieca::WordHypothesisI* h = new Bavieca::WordHypothesisI(words[i].c_str(), -1, -1);
		vhyp->push_back(h);
	}

	// 2. construct a hypothesis object from a vectorful of word hypotheses
	Bavieca::HypothesisI* hypothesis = new Bavieca::HypothesisI(*vhyp);

	// 3. get the assessment data
	Bavieca::TextAlignmentI* assessment = api->getAssessment(hypothesis, strReference);

	// 4. unwrap the data from Bavieca objects into returnable structs
	sTAElement* ret = new sTAElement[assessment->size()];
	*iElements = assessment->size();

	for (unsigned int i = 0; i < assessment->size(); i++) {
		Bavieca::TextAlignmentElementI* e = assessment->getElement(i);
		ret[i].iEvent = e->getEvent();
		ret[i].iIndexRef = e->getIndexRef();
		ret[i].iIndexHyp = e->getIndexHyp();

		if ( ret[i].iIndexRef != -1 ) {
			ret[i].strWordRef = new char[strlen(e->getWordRef()) + 1];
			strcpy(ret[i].strWordRef, e->getWordRef());
		} else {
			ret[i].strWordRef = NULL;
		}

		if ( ret[i].iIndexHyp != -1 ) {
			ret[i].strWordHyp = new char[strlen(e->getWordHyp()) + 1];
			strcpy(ret[i].strWordHyp, e->getWordHyp());
		} else {
			ret[i].strWordHyp = NULL;
		}
	}

	return ret;
}

void ADDCALL freeAssessment(Bavieca::BaviecaAPI* api, sTAElement* elements, unsigned int elementCount) {
	return;
	printf("deleting %d elements...", elementCount);

	for (unsigned int i = 0; i < elementCount; ++i) {
		if ( elements[i].strWordHyp )
			printf("%s\n", elements[i].strWordHyp);
			delete [] elements[i].strWordHyp;
		if ( elements[i].strWordRef )
			printf("%s\n", elements[i].strWordRef);
			delete [] elements[i].strWordRef;
		printf("deleted #%d...\n", i+1);
	}
	delete[] elements;

	printf("deletion successful\n");
}

// LATTICE TOOLS

void ADDCALL addPathToLattice(Bavieca::BaviecaAPI* api, Bavieca::HypothesisLattice *lattice, const char *featureFile, const char* alignmentFile, bool isBest) {
	api->addPathToLattice(lattice, featureFile, alignmentFile, isBest);
}

void ADDCALL rescore(Bavieca::BaviecaAPI* api, float amScore, float lmScore) {
	api->rescore(amScore, lmScore);
}

//Bavieca::TextAlignmentElementI* ADDCALL getAssessment(Bavieca::BaviecaAPI* api, Bavieca::WordHypothesisI* wordHypothesis,
//		unsigned int iWordsHyp, const char* strReference, int* iElementCount) {
//	// must transform to vector first...
//	printf("vector transform...\n");
//	vector<Bavieca::WordHypothesisI*> hvector(&wordHypothesis, &wordHypothesis + iWordsHyp);
//	printf("building hypothesis...\n");
//	Bavieca::HypothesisI* h = new Bavieca::HypothesisI(hvector);
//	printf("getting assessment...\n");
//	Bavieca::TextAlignmentI* tav = api->getAssessment(h, strReference);
//	printf("converting to list...\n");
//	Bavieca::TextAlignmentElementI* ret[tav->size()];
//	for (unsigned int i = 0; i < tav->size(); i++) {
//		ret[i] = tav->getElement(i);
//	}
//	printf("returning list...\n");
//	return *ret;
//}
