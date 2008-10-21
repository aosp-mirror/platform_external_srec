/* FILE:		grph.h
 *  DATE MODIFIED:	31-Aug-07
 *  DESCRIPTION:	Part of the  SREC graph compiler project source files.
 *
 *  Copyright 2007, 2008 Nuance Communciations, Inc.                               *
 *                                                                           *
 *  Licensed under the Apache License, Version 2.0 (the 'License');          *
 *  you may not use this file except in compliance with the License.         *
 *                                                                           *
 *  You may obtain a copy of the License at                                  *
 *      http://www.apache.org/licenses/LICENSE-2.0                           *
 *                                                                           *
 *  Unless required by applicable law or agreed to in writing, software      *
 *  distributed under the License is distributed on an 'AS IS' BASIS,        *
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. * 
 *  See the License for the specific language governing permissions and      *
 *  limitations under the License.                                           *
 *                                                                           *
 *---------------------------------------------------------------------------*/

#include "sub_grph.h"

class Graph
{
public:
    Graph (char *name)
    {
        int count= strlen(name);
        title= new char [count+1];
        strcpy (title, name);
        numSubGraph= 0;
        return;
    };

    ~Graph()
    {
	  delete [] subGraph;
	  delete [] subIndex;
	if (title) {
	    delete [] title;
	}
    }

    int addSubGraph (SubGraph *subGraph);
    int getSubGraphIndex (SubGraph *subGraph);

    void BeginRule (SubGraph *subg);
    void EndRule (SubGraph *subg);
    void BeginItem (SubGraph *subg);
    void BeginItemRepeat (SubGraph *subg, int min_count, int max_count);
    void AddRuleRef (SubGraph *subg, int ruleNo);
    void AddLabel (SubGraph *subg, int labNo);
    void AddTag (SubGraph *subg, int tagNo);
    void EndItem (SubGraph *subg);
    void BeginOneOf (SubGraph *subg);
    void EndOneOf (SubGraph *subg);
    void BeginCount (SubGraph *subg, int minCount, int maxCount);
    void EndCount (SubGraph *subg);
    void BeginOptional (SubGraph *subg);

    void ExpandRules (SubGraph *subg);

    int         numSubGraph;
    SubGraph    **subGraph;
    int         *subIndex;

private:

    int getSubGraphIndex (int subId);

    char        *title;

};


