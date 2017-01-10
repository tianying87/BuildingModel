/********************************************************************
Copyright (C), 2016, tianying All rights reserved.
File:	    common.cpp
Author:		tianying
Create:		2016/01/05
Purpose:	3D building 3ds Max model
*********************************************************************/
#include "common.h"

// 根据指定字符分割字符串
vector<string> SplitString(const string &a_strIn, const string &a_strDelimiters, bool a_bSkipEmpty)
{
	vector<string> vecTokens;

	// Find first "non-delimiter".
	string::size_type lastPos = 0;
	string::size_type pos = a_strIn.find_first_of(a_strDelimiters, lastPos);

	while (string::npos != pos || string::npos != lastPos)
	{
		// Found a token, add it to the vector.
		vecTokens.push_back(a_strIn.substr(lastPos, pos - lastPos));

		if (a_bSkipEmpty){
			// Skip delimiters.  Note the "not_of"
			lastPos = a_strIn.find_first_not_of(a_strDelimiters, pos);
		}else{
			if (string::npos == pos){
				break;
			}
			lastPos = pos + 1;
		}
		// Find next "non-delimiter"
		pos = a_strIn.find_first_of(a_strDelimiters, lastPos);
	}
	return vecTokens;
}

// 解析形状点数据
vector<double> GetPosition(char *a_pBuf, int a_i4Size)
{
	int iPartCnt = 0;
	memcpy(&iPartCnt, a_pBuf, sizeof(int));

	int iPositionSize = (a_i4Size - sizeof(int) * (iPartCnt + 1)) / sizeof(int);
	int *pPosition = (int *)(a_pBuf + sizeof(int) * (iPartCnt + 1));

	int i4StartIndex = 0;
	memcpy(&i4StartIndex, a_pBuf + sizeof(int), sizeof(int));

	vector<double> vecPosition;
	for (int i = i4StartIndex; i < iPositionSize; i++)
	{
		vecPosition.push_back(pPosition[i]/460800.0);
	}

	return vecPosition;
}

#define DEF_SPAN 20

#include "math.h"
#include "stdlib.h"

const double pi = 3.14159265358979324;

//
// Krasovsky 1940
//
// a = 6378245.0, 1/f = 298.3
// b = a * (1 - f)
// ee = (a^2 - b^2) / a^2;
const double a = 6378245.0;
const double ee = 0.00669342162296594323;

static bool outOfChina(double lat, double lon)
{
	if (lon < 72.004 || lon > 137.8347)
		return true;
	if (lat < 0.8293 || lat > 55.8271)
		return true;
	return false;
}

static double transformLat(double x, double y)
{
	double ret = -100.0 + 2.0 * x + 3.0 * y + 0.2 * y * y + 0.1 * x * y + 0.2 * sqrt(abs(x));
	ret += (20.0 * sin(6.0 * x * pi) + 20.0 * sin(2.0 * x * pi)) * 2.0 / 3.0;
	ret += (20.0 * sin(y * pi) + 40.0 * sin(y / 3.0 * pi)) * 2.0 / 3.0;
	ret += (160.0 * sin(y / 12.0 * pi) + 320 * sin(y * pi / 30.0)) * 2.0 / 3.0;
	return ret;
}

static double transformLon(double x, double y)
{
	double ret = 300.0 + x + 2.0 * y + 0.1 * x * x + 0.1 * x * y + 0.1 * sqrt(abs(x));
	ret += (20.0 * sin(6.0 * x * pi) + 20.0 * sin(2.0 * x * pi)) * 2.0 / 3.0;
	ret += (20.0 * sin(x * pi) + 40.0 * sin(x / 3.0 * pi)) * 2.0 / 3.0;
	ret += (150.0 * sin(x / 12.0 * pi) + 300.0 * sin(x / 30.0 * pi)) * 2.0 / 3.0;
	return ret;
}

void gps_transform( double wgLat, double wgLon, double& mgLat, double& mgLon)
{
	if (outOfChina(wgLat, wgLon))
	{
		mgLat = wgLat;
		mgLon = wgLon;
		return;
	}
	double dLat = transformLat(wgLon - 105.0, wgLat - 35.0);
	double dLon = transformLon(wgLon - 105.0, wgLat - 35.0);
	double radLat = wgLat / 180.0 * pi;
	double magic = sin(radLat);
	magic = 1 - ee * magic * magic;
	double sqrtMagic = sqrt(magic);
	dLat = (dLat * 180.0) / ((a * (1 - ee)) / (magic * sqrtMagic) * pi);
	dLon = (dLon * 180.0) / (a / sqrtMagic * cos(radLat) * pi);
	mgLat = wgLat + dLat;
	mgLon = wgLon + dLon;
};

struct MYPT
{
	int iLon;
	int iLat;
};

bool GpsCoorEncrypt(int inlLongitude, int inlLatitude,int &outlLongitude, int &outlLatitude)
{
	double inlLon = (double)inlLongitude/100000.0;
	double inlLat = (double)inlLatitude/100000.0;

	double outLon = 0.0;
	double outLat = 0.0;

	gps_transform(inlLat, inlLon, outLat, outLon);

	outlLongitude = (int) (outLon  * 100000.0);
	outlLatitude = (int) (outLat  * 100000.0);

	return true;
}

//int do_decode(double indLongitude,double indLatitude,double * outdLongitude,double * outdLatitude)
int do_decode(double indLongitude, double indLatitude, double &outLongitude, double &outLatitude)
{

	double lon,lat;
	double enc_lon = indLongitude;
	double enc_lat = indLatitude;

	// 	double enc_lon = 116.5;
	// 	double enc_lat = 40.68609597;

	double decode_lon = 0.0f;
	double decode_lat = 0.0f;

	//依次向左找到和他最近的x值

	int enc_iLon = enc_lon * 100000;
	int enc_iLat = enc_lat * 100000;

	int ilon = enc_lon * 100000;
	int ilat = enc_lat * 100000;

	int spanx = DEF_SPAN;
	int spany = DEF_SPAN;
	// 
	// 	MYPT encPt;
	// 
	// 	encPt.iLat = i_enc_lat;
	// 	encPt.iLon = i_enc_lon;

	int cur_iLat,cur_iLon;
	cur_iLon = enc_iLon;
	cur_iLat = enc_iLat;

	int iLeft = 0;
	int iRight = 0;
	int iTop = 0;
	int iBottom =  0;
	int outLon,outLat;

	GpsCoorEncrypt(cur_iLon,cur_iLat,outLon,outLat);

	if(outLon < enc_iLon)
	{
		iLeft = cur_iLon;
		spanx = DEF_SPAN;
	}
	else if(outLon > enc_iLon)
	{
		iRight = cur_iLon;
		spanx = -DEF_SPAN;
	}
	else
	{
		iLeft = cur_iLon;
		iRight = cur_iLon;
		spanx = 0;
	}

	if(outLat < enc_iLat)
	{
		iBottom = cur_iLat;

		spany = DEF_SPAN;
	}
	else if(outLat > enc_iLat)
	{
		iTop = cur_iLat;
		spany = -DEF_SPAN;
	}
	else
	{
		iBottom = cur_iLat;
		iTop = cur_iLat;
		spany = 0;
	}

	while(iLeft == 0 || iRight == 0 || iTop == 0 || iBottom ==  0) 
	{
		if(iLeft == 0 || iRight == 0)
			cur_iLon += spanx;

		if(iTop == 0 || iBottom ==  0)
			cur_iLat += spany;

		GpsCoorEncrypt(cur_iLon,cur_iLat,outLon,outLat);

		if(iLeft == 0 || iRight == 0)
		{
			if(outLon < enc_iLon)
			{
				iLeft = cur_iLon;

			}
			else if(outLon > enc_iLon)
			{
				iRight = cur_iLon;

			}
			else
			{
				iLeft = cur_iLon;
				iRight = cur_iLon;

			}
		}

		if(iTop == 0 || iBottom ==  0)
		{
			if(outLat < enc_iLat)
			{
				iBottom = cur_iLat;


			}
			else if(outLat > enc_iLat)
			{
				iTop = cur_iLat;

			}
			else
			{
				iBottom = cur_iLat;
				iTop = cur_iLat;

			}
		}

	};

	//gaojian add 处理左右或上下恰好相等的情况 1
	if (iLeft == iRight) { iLeft--; iRight++; }
	if (iBottom == iTop) { iBottom--; iTop++; }

	double x0,y0,x1,y1;
	double xx0,yy0,xx1,yy1;
	double y00,y11;

	MYPT leftTop,rightTop,leftBottom,rightBottom;

	GpsCoorEncrypt(iLeft,iTop,leftTop.iLon,leftTop.iLat);
	GpsCoorEncrypt(iRight,iTop,rightTop.iLon,rightTop.iLat);
	GpsCoorEncrypt(iLeft,iBottom,leftBottom.iLon,leftBottom.iLat);
	GpsCoorEncrypt(iRight,iBottom,rightBottom.iLon,rightBottom.iLat);

	//二次线性插值

	//gaojian add 处理左右或上下恰好相等的情况 2 （辅助）
	if (leftTop.iLon == rightTop.iLon) { leftTop.iLon--; rightTop.iLon++; }
	if (leftBottom.iLon == rightBottom.iLon) { leftBottom.iLon--; rightBottom.iLon++; }
	if (leftBottom.iLat == leftTop.iLat) { leftBottom.iLat--; leftTop.iLat++; }
	if (rightBottom.iLat == rightTop.iLat) { rightBottom.iLat--; rightTop.iLat++; }



	x0 = leftTop.iLon/100000.0;
	y0 = leftTop.iLat/100000.0;
	x1 = rightTop.iLon/100000.0;	
	y1 = rightTop.iLat/100000.0;
	//top
	xx0 = iLeft + (iRight - iLeft) *(enc_lon - x0) / (x1 - x0);
	//上方中点
	y00= y0 + (y1 - y0) *(enc_lon - x0) / (x1 - x0);

	x0 = leftBottom.iLon/100000.0;
	y0 = leftBottom.iLat/100000.0;
	x1 = rightBottom.iLon/100000.0;
	y1 = rightBottom.iLat/100000.0;	
	//bottom
	xx1 = iLeft + (iRight - iLeft) *(enc_lon - x0) / (x1 - x0);
	//下方中点
	y11= y0 + (y1 - y0) *(enc_lon - x0) / (x1 - x0);


	decode_lon = xx0 + (xx1 - xx0) * (enc_lat - y11) / (y00 - y11);
	decode_lat = (double)iBottom + ((double)iTop - (double)iBottom) * (enc_lat - y11) / (y00 - y11);

	outLongitude = decode_lon / 100000;
	outLatitude = decode_lat / 100000;


	// 	*outdLongitude = decode_lon / 100000;
	// 	*outdLatitude = decode_lat / 100000;

	GpsCoorEncrypt(decode_lon,decode_lat,outLon,outLat);
	//m_strOutput.Format(L"再次加密后结果为 %f %f",(double)outLon / 100000 ,(double)outLat / 100000);
	//UpdateData(FALSE);
	// 	GpsCoorEncrypt(decode_lon,decode_lat,outLon,outLat);
	// 	GpsCoorEncrypt(decode_lon,decode_lat + 1,outLon,outLat);
	// 	GpsCoorEncrypt(decode_lon,decode_lat + 9,outLon,outLat);

	return 0;
}