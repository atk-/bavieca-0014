/*
 * BaviecaNET.h
 *
 * A .NET/C# wrapper for Bavieca API.
 *
 *  Created on: 6.2.2014
 *      Author: ATK
 */

#ifndef BAVIECANET_H_
#define BAVIECANET_H_

using namespace std;

#include "BaviecaAPI.h"
#include <windows.h> // must be linked with ole32.lib!

#ifdef __cplusplus
extern "C" {
#ifdef BUILDDLL
	#define ADDAPI __declspec(dllexport)
#else
	#define ADDAPI __declspec(dllimport)
#endif

#define ADDCALL __stdcall
#else
	#define ADDAPI
	#define ADDCALL
#endif

/** create a managed string with memory allocation */
const char* mstr(const char* str) {
	ULONG ulSize = strlen(str) + sizeof(char);
	char* ret = NULL;
	ret = (char*)::CoTaskMemAlloc(ulSize);
	strcpy(ret, str);
	return ret;
}

///// DEFINE STRUCTS

#pragma pack(push, 8)

typedef struct {
	char* word;
	int fStart;
	int fEnd;
} sWordHypothesis;

typedef struct {
	unsigned char iEvent;
	int iIndexRef;
	char *strWordRef;
	int iIndexHyp;
	char *strWordHyp;
} sTAElement;

typedef struct sPhoneAlignment {
	char* phone;
	int fStart;
	int fEnd;
} sPhoneAlignment;

typedef struct sWordAlignment {
	char* word;
	int fStart;
	int fEnd;
	int iPhones;
	sPhoneAlignment* phoneAlignment;
} sWordAlignment;

#pragma pack(pop)

// ****** TEST METHODS -- REMOVE

ADDAPI void ADDCALL foo(void);

ADDAPI const char* ADDCALL bar(const char* str);

///// CREATION, INITIALIZATION AND UNINITIALIZATION

ADDAPI Bavieca::BaviecaAPI* ADDCALL create(const char *configFile);

ADDAPI Bavieca::BaviecaAPI* ADDCALL initialize(const char* configFile, unsigned char iFlags, Bavieca::ParamValueI *paramValues = NULL, unsigned int paramCount = 0);

ADDAPI void ADDCALL uninitialize(Bavieca::BaviecaAPI* api);

///// FEATURE EXTRACTION
ADDAPI float* ADDCALL extractFeaturesFromFile(Bavieca::BaviecaAPI* api, const char* wavFile, unsigned int* iFeatures);

ADDAPI float* ADDCALL extractFeatures(Bavieca::BaviecaAPI* api, short* sSamples, unsigned int iSamples, unsigned int* iFeatures);

ADDAPI void ADDCALL freeFeatures(Bavieca::BaviecaAPI* api, float *features);

///// SAD SESSION FUNCTIONS

ADDAPI void ADDCALL sadBeginSession(Bavieca::BaviecaAPI* api);

ADDAPI void ADDCALL sadEndSession(Bavieca::BaviecaAPI* api);

ADDAPI void ADDCALL sadFeed(Bavieca::BaviecaAPI* api, float* features, unsigned int iFeatures);

ADDAPI void ADDCALL freeSpeechSegments(Bavieca::BaviecaAPI* api, Bavieca::SpeechSegmentI* segments);

ADDAPI Bavieca::SpeechSegmentI* ADDCALL sadRecoverSpeechSegments(Bavieca::BaviecaAPI* api, int* iSegments);

///// ALIGNMENT

//ADDAPI Bavieca::WordAlignmentI* ADDCALL align(Bavieca::BaviecaAPI* api, float* fFeatures, unsigned int iFeatures, const char* strText, int* iWords, bool bMultiple=false);
ADDAPI sWordAlignment* ADDCALL align(Bavieca::BaviecaAPI* api, float* fFeatures, unsigned int iFeatures, const char* strText, unsigned int* iWords, bool bMultiple);

ADDAPI void ADDCALL alignFile(Bavieca::BaviecaAPI* api, const char *featureFile, const char *strText, const char *alignmentFile);

ADDAPI void ADDCALL freeWordAlignments(Bavieca::BaviecaAPI* api, Bavieca::WordAlignmentI* alignments, unsigned int iWords);

///// DECODING FUNCTIONS

ADDAPI void ADDCALL decBeginUtterance(Bavieca::BaviecaAPI* api);

ADDAPI void ADDCALL decEndUtterance(Bavieca::BaviecaAPI* api);

// no results; they must be asked through a separate function
ADDAPI void ADDCALL decProcess(Bavieca::BaviecaAPI* api, float* fFeatures, unsigned int iFeatures);

//ADDAPI Bavieca::WordHypothesisI* ADDCALL decGetHypothesis(Bavieca::BaviecaAPI* api, int *iWords);
ADDAPI sWordHypothesis* ADDCALL decGetHypothesis(Bavieca::BaviecaAPI* api, int *iWords);

ADDAPI void ADDCALL freeWordHypothesis(Bavieca::BaviecaAPI* api, Bavieca::WordHypothesisI* wordHypothesis, unsigned int iWords);

//ADDAPI Bavieca::TextAlignmentElementI* ADDCALL getAssessment(Bavieca::BaviecaAPI* api, Bavieca::WordHypothesisI* wordHypothesis, unsigned int iWordsHyp, const char* strReference, int* iElementCount);
ADDAPI sTAElement* ADDCALL getAssessment(Bavieca::BaviecaAPI* api, string* words, unsigned int iWordsHyp, const char* strReference, int* iElementCount);

ADDAPI void ADDCALL freeAssessment(Bavieca::BaviecaAPI* api, sTAElement* elements, unsigned int elementCount);

// LATTICE TOOLS

ADDAPI void ADDCALL addPathToLattice(Bavieca::BaviecaAPI* api, Bavieca::HypothesisLattice *lattice, const char *featureFile, const char* alignmentFile, bool isBest);

ADDAPI void ADDCALL rescore(Bavieca::BaviecaAPI* api, float amScore, float lmScore);

// TODO make this return the best path instead of void
ADDAPI inline void ADDCALL rescoreLattice(Bavieca::BaviecaAPI* api, Bavieca::HypothesisLattice *lattice, float amFactor, float lmFactor) {
	Bavieca::BestPath *bestPath = api->rescoreLattice(lattice, amFactor, lmFactor);
	printf("best path:\n");
	printf("-----------\n");
	bestPath->print(true);
	printf("-----------\n");
}

ADDAPI inline Bavieca::HypothesisLattice* ADDCALL getLattice(Bavieca::BaviecaAPI* api) {
	printf("1...\n");
	Bavieca::HypothesisLattice *lattice = api->getHypothesisLattice();
	printf("2...\n");
	return lattice;
}

ADDAPI inline void ADDCALL attachAMFromFile(Bavieca::BaviecaAPI *api, Bavieca::HypothesisLattice *lattice, const char *featureFile) {
	printf("attaching AM values...");
	api->attachAM(lattice, featureFile);
	printf("done\n");
}

ADDAPI inline void ADDCALL attachAM(Bavieca::BaviecaAPI *api, Bavieca::HypothesisLattice *lattice, float *fFeatures, unsigned int iFeatures) {
	api->attachAM(lattice, fFeatures, iFeatures);
}

ADDAPI inline void ADDCALL attachLM(Bavieca::BaviecaAPI *api, Bavieca::HypothesisLattice *lattice) {
	api->attachLM(lattice);
}

ADDAPI inline void ADDCALL storeLatticeAsText(Bavieca::BaviecaAPI *api, Bavieca::HypothesisLattice *lattice, const char *filename) {
	api->storeLatticeAsText(lattice, filename);
}

ADDAPI inline void ADDCALL computePP(Bavieca::BaviecaAPI* api, Bavieca::HypothesisLattice *lattice) {
	api->computePP(lattice);
}

ADDAPI inline void ADDCALL alignReferenceText(Bavieca::BaviecaAPI *api, Bavieca::HypothesisLattice *lattice, const char *refText) {
	api->alignReferenceText(lattice, refText);
}

//ADDAPI inline void ADDCALL attachLM(Bavieca::BaviecaAPI* api) {
//	api->attachLM();
//}
//
//ADDAPI inline void ADDCALL attachAM(Bavieca::BaviecaAPI* api, const char *strFeatureFile) {
//	api->attachAM(strFeatureFile);
//}

#ifdef __cplusplus
}
#endif

#endif /* BAVIECANET_H_ */
