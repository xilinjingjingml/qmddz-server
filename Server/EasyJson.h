#pragma once
#include "stdafx.h"

void RemoveString(string& str, string strPart)
{
	size_t index = 0;
	if (!str.empty())
	{
		while ((index = str.find(strPart, index)) != string::npos)
		{
			str.erase(index, strPart.size());
		}
	}
}

string GetStringValue(const string& strJson, const string& key, size_t pos_start, size_t pos_end)
{
	string strKey = "\"" + key + "\":";
	size_t pos_value_start = strJson.find(strKey, pos_start);
	if (pos_value_start == string::npos)
	{
		SERVER_LOG("find value:%s fail", key.c_str());
		return "";
	}

	pos_value_start += strKey.size();
	size_t pos_value_end = strJson.find(",", pos_value_start);
	if (pos_value_end == string::npos || pos_value_end > pos_end)
	{
		pos_value_end = pos_end;
	}

	return strJson.substr(pos_value_start, pos_value_end - pos_value_start);
}

string GetJsonData(const string& strJson, const string& name, const string& key, bool bVisible = true)
{
	string strData = "";
	do
	{
		string strName = "\"" + name + "\":{";
		size_t pos_start = strJson.find(strName);
		if (pos_start == string::npos)
		{
			SERVER_LOG("not find key:%s", name.c_str());
			break;
		}

		pos_start += strName.size();
		size_t pos_end = strJson.find("}", pos_start);
		if (pos_end == string::npos)
		{
			SERVER_LOG("not find \"}\" key:%s", name.c_str());
			break;
		}

		if (bVisible)
		{
			string strVisible = GetStringValue(strJson, "visible", pos_start, pos_end);
			if (strVisible != "true")
			{
				SERVER_LOG("find visible:%s key:%s fail", strVisible.c_str(), name.c_str());
				break;
			}
		}

		strData = GetStringValue(strJson, key, pos_start, pos_end);
	} while (0);

	return strData;
}

int GetJsonValue(const string& strJson, const string& name, bool bVisible = true)
{
	string strValue = GetJsonData(strJson, name, "value", bVisible);
	return std::atoi(strValue.c_str());
}