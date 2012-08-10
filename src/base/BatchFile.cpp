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

#include "BatchFile.h"
#include "FileInput.h"
#include "IOBase.h"
#include <stdlib.h>

// constructor
BatchFile::BatchFile(const char *strFile, const char *strType) {

	m_strFile = strFile;
	m_strType = strType;
	m_iColumns = -1;
}

// destructor
BatchFile::~BatchFile() {

	for(VBatchEntry::iterator it = m_vBatchEntry.begin() ; it != m_vBatchEntry.end() ; ++it) {
		delete *it;
	}
}

// load the content of the batch file
void BatchFile::load() {

	// get the column names
	int iColumn = 0;
	char strColumnName[1024];
	int iIndex = 0;
	const char *str = m_strType.c_str();
	while(1) {
		if ((*str == '|') || (*str == 0)) {
			if (iIndex < 1) {
				ERROR << "wrong type" << endl;
			}
			strColumnName[iIndex] = 0;
			m_mColumnName.insert(map<string,int>::value_type(strColumnName,iColumn++));
			iIndex = 0;
		} else {
			strColumnName[iIndex++] = *str;	
		}
		if (*str == 0) {
			break;
		}
		++str;
	}	
	
	m_iColumns = iColumn;
	
	FileInput file(m_strFile.c_str(),false);
	file.open();
	
	int iLine = 1;
	string strLine;
	while(std::getline(file.getStream(),strLine).good()) {
		if (strLine.empty()) {
			break;
		}
		std::stringstream s(strLine);
		BatchEntry *batchEntry = new BatchEntry();	
		for(int i=0 ; i < m_iColumns ; ++i) {
			string strField;
			IOBase::readString(s,strField);	
			batchEntry->vStrElement.push_back(strField);	
		}	
		if (batchEntry->vStrElement.size() != m_iColumns) {
			ERROR << "wrong number of columns in line :" << iLine << endl;
		}
		m_vBatchEntry.push_back(batchEntry);
		++iLine;
	}
	
	file.close();
}


// return the field in the given entry and column
const char *BatchFile::getField(int iEntry, int iColumn) {
	
	assert((iEntry >= 0) && (iEntry < m_vBatchEntry.size()));
	assert((iColumn >= 0) && (iColumn < m_iColumns));
	
	return m_vBatchEntry[iEntry]->vStrElement[iColumn].c_str();
}		

// return the field in the given entry by its name
const char *BatchFile::getField(int iEntry, const char *strColumnName) {
	
	// get the column by its name
	map<string,int>::iterator it = m_mColumnName.find(strColumnName);
	assert(it != m_mColumnName.end());
	
	return getField(iEntry,it->second);	
}