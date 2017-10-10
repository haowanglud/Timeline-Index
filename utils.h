//
//  Header.h
//  Timeline_Index
//
//  Created by kaze on 03/08/2017.
//  Copyright © 2017 th. All rights reserved.
//

#ifndef utils_h
#define utils_h

typedef std::vector<struct Item> Table;
typedef std::vector<struct EventEntry> EventList;
typedef std::vector<struct VersionEntry> VersionMap;
typedef std::vector<struct JointEntry> JointTable;
typedef std::map<std::string, std::vector<std::vector<int>>> IntersectionMap;

struct Item
{
    std::string key;
    std::string role;
    std::string houseNum;
    int num1;
    int num2;
};

struct VersionEntry
{
    int version;
    int eventID;
};

struct EventEntry
{
    int rowID;
    bool event;
};

struct JointEntry
{
    Item i1;
    Item i2;
};

#endif /* Header_h */
