/********************************************************************
Copyright (C), 2016, tianying All rights reserved.
File:	    common.h
Author:		tianying
Create:		2016/01/05
Purpose:	3D building 3ds Max model
*********************************************************************/
#pragma once

#include <iostream>
#include <vector>
#include <string>
using namespace std;

/*
** This constant should already be defined (in the "WinDef.h" SDK file).
*/
#ifndef MAX_PATH
#  define MAX_PATH                      (260)
#endif

// 根据指定字符分割字符串
vector<string> SplitString(const string &a_strIn, const string &a_strDelimiters, bool a_bSkipEmpty = false);

// 解析形状点数据
vector<double> GetPosition(char *a_pBuf, int a_i4Size);


//02坐标转84坐标
int do_decode(double indLongitude, double indLatitude, double &outLongitude, double &outLatitude);
