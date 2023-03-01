/*
 CopyRight(c)2006 by Banding,Shanghai, All Right Reserved.

 @Date:	        2006/11/11
 @Created:	    15:02
 @Filename: 	PlayerCreator.cpp
 @File path:	z:\workspace\BDLeyoyoV2\NewDDZ\Server 
 @Author:		wolfplus
 @Version:      200611111502
	
 @Description:	
*/

#include "stdafx.h"
#include "PlayerCreator.h"
#include "Player.h"

CPlayerCreator::CPlayerCreator(void)
{
}

CPlayerCreator::~CPlayerCreator(void)
{
}
IPlayerDelegate* CPlayerCreator::CreatePlayer()
{
	return new CPlayer();
}
