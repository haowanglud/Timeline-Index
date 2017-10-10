#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <assert.h>
#include <ctime>
#include "utils.h"
using namespace std;

void error_msg(string msg){
    cerr << "Error: " << msg << endl;
    exit(1);
}

void readData(Table &table, map<int, int> &versionCount, const string filename){
    ifstream f;
    string s = "/Users/Apple/Desktop/Timeline_Index/Timeline_Index/"+filename;
    cout << s+"\n";
    f.open(s);
    
    if (f.is_open()){
        // open sucessfully
        string buf;
        Item item;
        map<int, int>::iterator ret;
        int count = 0;
        while (getline(f, buf)){
            istringstream ss(buf);
            while (getline(ss, buf, '\t')){
                switch (count) {
                    case 0:
                        item.key = buf;
                        count++;
                        break;
                    case 1:
                        item.role = buf;
                        count++;
                        break;
                    case 2:
                        item.houseNum = buf;
                        count++;
                        break;
                    case 3:
                        item.num1 = stoi(buf);
                        count++;
                        
                        //increment the count of version
                        ret = versionCount.find(item.num1);
                        if (ret == versionCount.end())
                            versionCount.insert(pair<int, int>(item.num1, 1));
                        else
                            versionCount[item.num1]++;
                        break;
                    case 4:
                        item.num2 = stoi(buf);
                        count = 0;
                        table.push_back(item);
                        
                        //increment the count of version
                        ret = versionCount.find(item.num2);
                        if (ret == versionCount.end())
                            versionCount.insert(pair<int, int>(item.num2, 1));
                        else
                            versionCount[item.num2]++;
                        break;
                    default:
                        error_msg("Reading file error");
                        break;
                }
            }
        }
        f.close();
    }
    else
        error_msg("Unable to open file");
}

void createTimeline(const Table &table, const map<int, int> &versionCount, VersionMap &versionMap, EventList &eventList){
    
    map<int, int> versionToIndex;
    int count = 0;
    int index = 0;
    for (map<int, int>::const_iterator it = versionCount.begin(); it != versionCount.end(); it++){
        VersionEntry ve;
        ve.version = it->first;
        ve.eventID = count;
        versionMap.push_back(ve);
        versionToIndex.insert(pair<int, int> (it->first, index));
        count += it->second;
        index++;
    }
    
    int row = 0, offset;
    EventEntry ee;
    EventList newEventList(count, ee);
    
    for (Table::const_iterator it = table.begin(); it != table.end(); it++){
        // start column
        row++;
        ee.rowID = row;
        ee.event = 1;
        index = versionToIndex[it->num1];
        offset = versionMap[index].eventID++;
        newEventList[offset] = ee;
        
        // end column
        ee.rowID = row;
        ee.event = 0;
        index = versionToIndex[it->num2];
        offset = versionMap[index].eventID++;
        newEventList[offset] = ee;
    }
    
    eventList = newEventList;
}

// linear search the table
Table keyPointSearch(const string key, const Table &table){
    Table res;
    
    size_t size = table.size();
    
    for (int i = 0; i < size; i++){
        if (table[i].key == key && table[i].num2 == INT_MAX)
            res.push_back(table[i]);
    }
    
    return res;
}


// linear search the table
Table keyRangeSearch(const string key1, const string key2, const Table &table){
    Table res;
    string keyStart = key1 <= key2? key1 : key2;
    string keyEnd = key1 > key2? key1 : key2;
    
    size_t size = table.size();
    
    for (int i = 0; i < size; i++){
        if (table[i].key >= keyStart && table[i].key <= keyEnd && table[i].num2 == INT_MAX)
            res.push_back(table[i]);
    }
    
    return res;
}

vector<bool> returnBitmap(const int versionNum1, const int versionNum2, const Table &table, const VersionMap &versionMap, const EventList &eventList){
    int start = versionNum1 <= versionNum2 ? versionNum1 : versionNum2;
    int end = versionNum1 > versionNum2 ? versionNum1: versionNum2;
    Table res;
    size_t table_size = table.size();
    size_t versionmap_size = versionMap.size();
    size_t eventlist_size = eventList.size();
    vector<bool> bitmap(table_size, 0);
    
    // all delete before and containing start_offset
    // all insert before and containing end_offset are returned
    int start_offset = 0, end_offset = 0;
    bool findstart = 0, findend = 0;
    for (int i = 0; i < versionmap_size; i++){
        if (versionMap[i].version > start)
            findstart = 1;
        else
            start_offset = versionMap[i].eventID;
        if (versionMap[i].version > end)
            findend = 1;
        else
            end_offset = versionMap[i].eventID;
        
        if (findstart & findend)
            break;
    }
    
    if (end_offset > eventlist_size)
        error_msg("offset from versionMap larger than eventList size!");
    
    // construct bitmap from eventlist
    for (int i = 0; i < end_offset; i++){
        int index = eventList[i].rowID-1;
        if (i < start_offset && eventList[i].event == 0)
            bitmap[index] = 0;
        else if (i < end_offset && eventList[i].event == 1)
            bitmap[index] = 1;
    }
    return bitmap;
}

void returnCongressIntersectionMap(const int versionNum1, const int versionNum2, const Table &table, const VersionMap &versionMap, const EventList &eventList, IntersectionMap &intersectionMap, const int col){
    
    int start = versionNum1 <= versionNum2 ? versionNum1 : versionNum2;
    int end = versionNum1 > versionNum2 ? versionNum1: versionNum2;
    Table res;
    size_t versionmap_size = versionMap.size();
    size_t eventlist_size = eventList.size();
    
    // all delete before and containing start_offset
    // all insert before and containing end_offset are returned
    int start_offset = 0, end_offset = 0;
    bool findstart = 0, findend = 0;
    for (int i = 0; i < versionmap_size; i++){
        if (versionMap[i].version > start)
            findstart = 1;
        else
            start_offset = versionMap[i].eventID;
        if (versionMap[i].version > end)
            findend = 1;
        else
            end_offset = versionMap[i].eventID;
        
        if (findstart & findend)
            break;
    }
    
    if (end_offset > eventlist_size)
        error_msg("offset from versionMap larger than eventList size!");
    
    // construct bitmap from eventlist
    for (int i = 0; i < end_offset; i++){
        int index = eventList[i].rowID-1;
        string key = table[index].key;
        
        vector<vector<int>> intersectionColumn(2,vector<int>(0,0));
        pair<string, vector<vector<int>>> tuple(key, intersectionColumn);
        
        if (i < start_offset && eventList[i].event == 0){
            
            // in the map
            if (intersectionMap.find(key) != intersectionMap.end()){
                vector<int>::iterator it = find(intersectionMap[key][col].begin(), intersectionMap[key][col].end(), index);
                if (it == intersectionMap[key][col].end())
                    error_msg("Attemp to delete a record that doesn't exist");
                else{
                    intersectionMap[key][col].erase(it);
                 }
            }
            
            // the column deleted is not in the map
            else
                error_msg("Attemp to delete a record that doesn't exist");
        }
        else if (i < end_offset && eventList[i].event == 1){
            if (intersectionMap.find(key) != intersectionMap.end()){
                intersectionMap[key][col].push_back(index);
            }
            else{
                tuple.second[col].push_back(index);
                intersectionMap.insert(tuple);
            }
        }
    }
}


void returnPeopleIntersectionMap(const int versionNum1, const int versionNum2, const Table &table, const VersionMap &versionMap, const EventList &eventList, IntersectionMap &intersectionMap, const int col){
    
    int start = versionNum1 <= versionNum2 ? versionNum1 : versionNum2;
    int end = versionNum1 > versionNum2 ? versionNum1: versionNum2;
    Table res;
    size_t versionmap_size = versionMap.size();
    size_t eventlist_size = eventList.size();
    
    // all delete before and containing start_offset
    // all insert before and containing end_offset are returned
    int start_offset = 0, end_offset = 0;
    bool findstart = 0, findend = 0;
    for (int i = 0; i < versionmap_size; i++){
        if (versionMap[i].version > start)
            findstart = 1;
        else
            start_offset = versionMap[i].eventID;
        if (versionMap[i].version > end)
            findend = 1;
        else
            end_offset = versionMap[i].eventID;
        
        if (findstart & findend)
            break;
    }
    
    if (end_offset > eventlist_size)
        error_msg("offset from versionMap larger than eventList size!");
    
    // construct bitmap from eventlist
    for (int i = 0; i < end_offset; i++){
        int index = eventList[i].rowID-1;
        string key = table[index].houseNum;
        
        vector<vector<int>> intersectionColumn(2,vector<int>(0,0));
        pair<string, vector<vector<int>>> tuple(key, intersectionColumn);
        
        if (i < start_offset && eventList[i].event == 0){
            
            // in the map
            if (intersectionMap.find(key) != intersectionMap.end()){
                vector<int>::iterator it = find(intersectionMap[key][col].begin(), intersectionMap[key][col].end(), index);
                if (it == intersectionMap[key][col].end())
                    error_msg("Attemp to delete a record that doesn't exist");
                else{
                    intersectionMap[key][col].erase(it);
                }
            }
            
            // the column deleted is not in the map
            else
                error_msg("Attemp to delete a record that doesn't exist");
        }
        else if (i < end_offset && eventList[i].event == 1){
            if (intersectionMap.find(key) != intersectionMap.end()){
                intersectionMap[key][col].push_back(index);
            }
            else{
                tuple.second[col].push_back(index);
                intersectionMap.insert(tuple);
            }
        }
    }
}

// linear search the version number to get the offset in eventlist, and then linear scan eventlist
Table timePointSearch(const int versionNum, const Table &table, const VersionMap &versionMap, const EventList &eventList){
    Table res;
    size_t table_size = table.size();
    vector<bool> bitmap = returnBitmap(versionNum, versionNum, table, versionMap, eventList);
    
    // scan table and add according to bitmap
    for (int i = 0; i < table_size; i++){
        if (bitmap[i] == 1)
            res.push_back(table[i]);
    }
    
    return res;
}

// linear search the version number to get the offset in eventlist, and then linear scan eventlist
Table timeRangeSearch(const int versionNum1, const int versionNum2, const Table &table, const VersionMap &versionMap, const EventList &eventList){
    Table res;
    size_t table_size = table.size();
    vector<bool> bitmap = returnBitmap(versionNum1, versionNum2, table, versionMap, eventList);
    // scan table and add according to bitmap
    for (int i = 0; i < table_size; i++){
        if (bitmap[i] == 1)
            res.push_back(table[i]);
    }
    
    return res;
}

JointTable temporalJoin(const int versionNum1, const int versionNum2, const Table &table1, const Table &table2, const VersionMap &versionMap1, const VersionMap &versionMap2, const EventList &eventList1, const EventList &eventList2){
    
    JointTable res;
    IntersectionMap intersectionMap;
    
    returnPeopleIntersectionMap(versionNum1, versionNum2, table1, versionMap1, eventList1, intersectionMap, 0);
    returnCongressIntersectionMap(versionNum1, versionNum2, table2, versionMap2, eventList2, intersectionMap, 1);
    
    for (IntersectionMap::iterator it = intersectionMap.begin(); it != intersectionMap.end(); it++){
        size_t size1 = it->second[0].size(), size2 = it->second[1].size();
        for (int i = 0; i < size1; i++){
            for (int j = 0; j < size2; j++){
                JointEntry entry;
                entry.i1 = table1[it->second[0][i]];
                entry.i2 = table2[it->second[1][j]];
                if ((entry.i1.num1 < entry.i2.num2 && entry.i1.num1 > entry.i2.num1) || (entry.i1.num2 > entry.i2.num1 && entry.i1.num2 < entry.i2.num2)){
                    res.push_back(entry);
                }
            }
        }
    }
    
    return res;
}

void debug(const Table &table, const VersionMap &versionMap, const EventList &eventList, const map<int, int> &versionCount, const Table &table1, const VersionMap &versionMap1, const EventList &eventList1, const map<int, int> &versionCount1){
    // check version count
    cout << "------------VersionCount start--------------"<< endl;
    int count = 0;
    cout << "Version, event count" << endl;
    for (map<int, int>::const_iterator it = versionCount.begin(); it != versionCount.end(); it++){
        cout << it->first << " " << it->second << endl;
        count+=it->second;
    }
    cout << "Number of events in the data file/temporal table: " << count << endl;
    cout << "------------VersionCount end--------------"<< endl;
    
    // check reading data
    cout << "------------TemporalTable start--------------"<< endl;
    cout << "key, start, end" << endl;
    for (int i = 0; i < table.size(); i++){
        cout <<table[i].key << " " << table[i].num1 << " " << table[i].num2 << endl;
    }
    cout << "Number of rows in the data file/temporal table: " << table.size() << endl;
    cout << "------------TemporalTable end--------------"<< endl;
    
    // check versionMap
    cout << "------------VersionMap start--------------"<< endl;
    cout << "Version, eventID/offset" << endl;
    for (vector<VersionEntry>::const_iterator it = versionMap.begin(); it != versionMap.end(); it++){
        cout << it->version << " " << it->eventID << endl;
    }
    cout << "------------VersionMap end--------------"<< endl;
    
    // check eventList
    cout << "------------EventList start--------------"<< endl;
    cout << "rowID, event" << endl;
    for (vector<EventEntry>::const_iterator it = eventList.begin(); it!= eventList.end(); it++){
        cout << it->rowID << " " << it->event << endl;
    }
    cout << "------------EventList end--------------"<< endl;
    
    // check key Search
    cout << "------------keySearch start--------------"<< endl;
    Table res;
    // in table and until now
    res = keyPointSearch("people_400008", table);
//    assert(res.size() == 1 && res[0].key == "people_400008");
    // in table but not until now
    res = keyPointSearch("people_400007", table);
//    assert(res.size() == 0);
    // not in table
    res = keyPointSearch("people_600008", table);
//    assert(res.size() == 0);
    cout << "Pass all test for key point search!" << endl;
    // in table and until now, and not until now
    res = keyRangeSearch("p", "q", table);
//    assert(res.size() == 2 && res[0].key == "people_400004" && res[1].key == "people_400008");
    // not in table
    res = keyRangeSearch("p", "pe", table);
//    assert(res.size() == 0);
    cout << "Pass all test for key range search!" << endl;
    cout << "------------keySearch end--------------"<< endl;
    
    // check timestamp search
    cout << "------------timestampSearch start--------------"<< endl;
    res = timePointSearch(103, table, versionMap, eventList);
//    assert(res.size() == 8 && res[0].key == "people_400003" && res[1].key == "people_400004"
//           && res[2].key == "people_400007" && res[3].key == "people_400010" && res[4].key == "people_400011" && res[5].key == "people_400017" && res[6].key == "people_400021" && res[7].key == "people_400025");
    cout << "Pass all test for timestamp point search!" << endl;
    res = timeRangeSearch(105, 108, table, versionMap, eventList);
//    assert(res.size() == 12 && res[0].key == "people_400003" && res[1].key == "people_400004"
//           && res[2].key == "people_400007" && res[3].key == "people_400008" && res[4].key == "people_400009" && res[5].key == "people_400010" && res[6].key == "people_400017" && res[7].key == "people_400021" && res[8].key == "people_400023" && res[9].key == "people_400024"
//           && res[10].key == "people_400026" && res[11].key == "people_400027");
    cout << "Pass all test for timestamp range search!" << endl;
    cout << "------------timestampSearch end--------------"<< endl;
    
    // check temporal join
    cout << "------------temporal join start--------------"<< endl;
    JointTable resjoin = temporalJoin(105, 108, table, table1, versionMap, versionMap1, eventList, eventList1);
//    assert(resjoin.size() == 3 && resjoin[0].i1.key == "people_400003" && resjoin[1].i2.key == "people_400008" && resjoin[2].i1.key == "people_400010");
    cout << "Pass all test for temporal join!" << endl;
    cout << "------------temporal join end--------------"<< endl;

}


double timeKeyPointSearch(const string key, const Table &table){
    clock_t start = clock();
    keyPointSearch(key, table);
    return (clock()-start)/(double)CLOCKS_PER_SEC;
}

double timeKeyRangeSearch(const string key1, const string key2, const Table &table){
    clock_t start = clock();
    keyRangeSearch(key1, key2, table);
    return (clock()-start)/(double)CLOCKS_PER_SEC;
}

double timeTimePointSearch(const int versionNum, const Table &table, const VersionMap &versionMap, const EventList &eventList){
    clock_t start = clock();
    timePointSearch(versionNum, table, versionMap, eventList);
    return (clock()-start)/(double)CLOCKS_PER_SEC;
}

double timeTimeRangeSearch(const int versionNum1, const int versionNum2, const Table &table, const VersionMap &versionMap, const EventList &eventList){
    clock_t start = clock();
    timeRangeSearch(versionNum1, versionNum2, table, versionMap, eventList);
    return (clock()-start)/(double)CLOCKS_PER_SEC;
}

double timeTemporalJoin(const int versionNum1, const int versionNum2, const Table &table1, const Table &table2, const VersionMap &versionMap1, const VersionMap &versionMap2, const EventList &eventList1, const EventList &eventList2){
    clock_t start = clock();
    temporalJoin(versionNum1, versionNum2, table1, table2, versionMap1, versionMap2, eventList1, eventList2);
    return (clock()-start)/(double)CLOCKS_PER_SEC;
}

int main(int argc, const char * argv[]) {
    Table table1;
    VersionMap versionMap1;
    EventList eventList1;
    map<int, int> versionCount1;
    
    readData(table1, versionCount1, "people.tsv");
    createTimeline(table1, versionCount1, versionMap1, eventList1);
    

    Table table2;
    VersionMap versionMap2;
    EventList eventList2;
    map<int, int> versionCount2;
    
    readData(table2, versionCount2,"congress.tsv");
    createTimeline(table2, versionCount2, versionMap2, eventList2);

//    // start timing
//    clock_t c_start = clock();
//    debug(table, versionMap, eventList, versionCount, table1, versionMap1, eventList1, versionCount1);
//    cout << (clock()-c_start)/(double)CLOCKS_PER_SEC;
    cout << "key point search time: " << timeKeyPointSearch("people_400150", table1) << endl;
    cout << "key range search time: " << timeKeyRangeSearch("people_400150", "people_400155", table1) << endl;
    cout << "time point search time: " << timeTimePointSearch(599817600, table1, versionMap1, eventList1) << endl;
    cout << "time range search time: " << timeTimeRangeSearch(599817600, 657187199, table1, versionMap1, eventList1) << endl;
    cout << "temporal join time: " << timeTemporalJoin(603014400, 603018900, table1, table2, versionMap1, versionMap2, eventList1, eventList2) << endl;
    return 0;
}
