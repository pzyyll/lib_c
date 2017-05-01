/**
 * @file    pet_info_def.h
 * @brief   SESSION存储的一些数据接口，SESSIPN_API对外的数据传输基本采用该文件定义的数据结构
 * @author  jamieli@tencent.com
 * @date    2009-03-09
 */

#ifndef _PET_INFO_DEF_H_
#define _PET_INFO_DEF_H_

#include "petlib_common.h"

namespace snslib
{

const int PET_NAME_LEN = 16;
const int COMMUNITY_INFO_NUM = 8;
const int PET_STATUS_DATA_LEN = 16;
const int MOBILE_NO_LEN = 20;
const int MAX_STUDIED_COURSE_INFO_NUM = 64;
const int MAX_AVATAR_INFO_NUM = 20;
/**
 * 宠物ID的结构定义
 */
typedef union tagPetID
{
    unsigned long long ullID;
    struct
    {
        unsigned int uiUin;
        unsigned short ushSpec;
        unsigned char ucFamilyID;
        unsigned char ucSex;
    };
} PetID;

inline unsigned int GetUin(unsigned long long ullPetID)
{
    return ((PetID *)&ullPetID)->uiUin;
}

/**
 * 物品ID的结构定义
 */
typedef union tagGoodsID
{
    unsigned long long ullID;
    struct
    {
        //TODO 这里只是一种类型的物品，还存在其他类型的物品，结构定义不同，需要继续补充
        unsigned short ushGoodsType;
        union
        {
            struct
            {
                unsigned char ucSex;
                unsigned char ucPetLevel;
                unsigned short ushGoodsGroup;
                unsigned short ushGoodsID;

            };
        };

    };
}CompoundGoodsID;

}

#endif

