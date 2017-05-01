#ifndef _STAT_MAP_INFO_DEF_H_
#define _STAT_MAP_INFO_DEF_H_

#include "api/include/pet_info_def.h"

namespace snslib{

typedef struct tagPetZoneInfo
{
    unsigned int uiZoneID;
}PetZoneInfo;

typedef struct tagFriendShowInfo
{
    char szPetName[PET_NAME_LEN+1];
    char szQQName[PET_NAME_LEN+1];
    char cVipFlag;
    unsigned short ushPetLevel;
    unsigned int uiPermission;
    unsigned short ushTitleID;
}FriendShowInfo;

typedef struct tagHomeShowInfo
{
    char szPetName[PET_NAME_LEN+1];
    char szQQName[PET_NAME_LEN+1];
    unsigned short ushPetLevel;
    unsigned short ushStatus;
    unsigned short ushAvatarVersion;
}HomeShowInfo;

}
#endif
