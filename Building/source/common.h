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

// ����ָ���ַ��ָ��ַ���
vector<string> SplitString(const string &a_strIn, const string &a_strDelimiters, bool a_bSkipEmpty = false);

// ������״������
vector<double> GetPosition(char *a_pBuf, int a_i4Size);


//02����ת84����
int do_decode(double indLongitude, double indLatitude, double &outLongitude, double &outLatitude);
