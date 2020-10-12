#include"Compression.h"
#include<iostream>
#include<fstream>
#include<ctime>
using namespace std;

Compression::Compression() 
{
	_tableName = "test";//tbl_paraminfo
}

bool Compression::connectMysql() 
{
	mysql_init(&_mysql);
	mysql_options(&_mysql, MYSQL_SET_CHARSET_NAME, "gbk");
	//mysql_real_connect参数：2.本地地址  3.你的mysql用户名  4.你的mysql密码   5.数据库名字  6.端口号
	if (mysql_real_connect(&_mysql, "localhost", "root", "123456", "mydb", 3306, NULL, 0) == NULL) 
	{
		return false;
	}
	else 
	{
		return true;
	}
}

byte Compression::compressData(string str, bool* isCorrect)
{
	int num = 0;
	int temp = 0;
	for (int i = 0; i < 2; i++) 
	{
		if (((str[i] - '0') >= 0) && ((str[i] - '0') <= 9)) 
		{
			temp = str[i] - '0';
			num = (num << 4) | (temp & 0xf);
		}
		else if (((str[i] - 'a') >= 0) && ((str[i] - 'a') <= 5)) 
		{
			temp = 10 + (str[i] - 'a');
			num = (num << 4) | (temp & 0xf);
		}
		else if (((str[i] - 'A') >= 0) && ((str[i] - 'A') <= 5)) 
		{
			temp = 10 + (str[i] - 'A');
			num = (num << 4) | (temp & 0xf);
		}
		else 
		{
			*isCorrect = false;
		}
	}
	byte tempByte = (byte)(num & 0xff);
	return tempByte;
}

string Compression::decompressData(int num) 
{
	string str = "";
	int temp[2];
	temp[0] = (num >> 4) & 0xf;
	temp[1] = num & 0xf;
	for (int i = 0; i < 2; i++) 
	{
		if (temp[i] >= 0 && temp[i] <= 9) 
		{
			str = str + to_string(temp[i]);
		}
		else if (temp[i] >= 10 && temp[i] <= 15) 
		{
			char t = 'A';
			t = t + (temp[i] - 10);
			str = str + t;
		}
	}
	return str;
}

string Compression::getCardnetInfo()
{
	string sql = "SELECT DISTINCT cardnet FROM " + _tableName + " ORDER BY CardNet ASC";
	char* p = _strdup(sql.c_str());
	mysql_query(&_mysql, p);
	MYSQL_RES* res = mysql_store_result(&_mysql);
	string str = "";
	CardNetInfo cardNetInfo;
	MYSQL_ROW row;
	while (row = mysql_fetch_row(res)) 
	{
		str = row[0];
		cardNetInfo.cardnet = str;
		cardNetInfo.count = 0;
		_cardNetInfos.push_back(cardNetInfo);
	}
	int category = _cardNetInfos.size();
	for (int i = 0; i < category; i++) 
	{
		sql = "select COUNT(*) from " + _tableName + " where CardNet = " + _cardNetInfos[i].cardnet;
		p = _strdup(sql.c_str());
		mysql_query(&_mysql, p);
		res = mysql_store_result(&_mysql);
		while (row = mysql_fetch_row(res)) 
		{
			str = row[0];
			_cardNetInfos[i].count = stoi(str);
		}
	}
	mysql_free_result(res);
	string index = "";
	for (int i = 0; i < category; i++) 
	{
		index = index + _cardNetInfos[i].cardnet + ":" + to_string(_cardNetInfos[i].count) + "#";
	}
	cout << index << endl;
	return index;
}

void Compression::storeData() 
{
	if(connectMysql()==false)
	{
		cout << (mysql_error(&_mysql));
		return;
	}
	string index = getCardnetInfo();
	ofstream outfile(_path + "/CompressedData.dat", ios::app | ios::binary);
	outfile << index << endl;

	string sql = "";
	char* p;
	bool* isCorrect=new bool;
	*isCorrect = true;
	int temp = 0;
	string str;
	MYSQL_ROW row;
	MYSQL_RES* res = NULL;
	int category = _cardNetInfos.size();
	byte cardID[8];
	for (int i = 0; i < category; i++) 
	{
		cout << i << endl;
		sql = "SELECT * FROM " + _tableName + " WHERE CardNet = " 
			+ _cardNetInfos[i].cardnet + " ORDER BY CardID ASC";
		p = _strdup(sql.c_str());
		mysql_query(&_mysql, p);
		res = mysql_store_result(&_mysql);
		while (row = mysql_fetch_row(res))
		{
			str = row[1];
			temp = str.size();
			if (temp < 16) 
			{
				for (int i = temp; i < 16; i++)
				{
					str = str + "0";
				}
			}
			else if(temp > 16)
			{
				cout << "数据有误: " << str << endl;
				continue;
			}
			for (int i = 0; i < 15; i = i + 2) 
			{
				cardID[(i/2)] = compressData(str.substr(i, 2), isCorrect);
			}
			if (*isCorrect == false) 
			{
				cout << "数据有误: " << str << endl;
				*isCorrect = true;
				continue;
			}
			for (int i = 0; i < 8; i++) 
			{
				outfile.write((char*)&cardID[i], sizeof(cardID[i]));
			}
			str = row[2];
			temp = stoi(str);
			cardID[0] = (byte)(temp & 0xff);
			outfile.write((char*)&cardID[0], sizeof(cardID[0]));
		}
		
	}
	mysql_free_result(res);
	mysql_close(&_mysql);
	outfile.close();
}

void Compression::checkData() 
{
	ifstream inFile(_path + "/CompressedData.dat", ios::binary);
	char index;
	string str = "";
	CardNetInfo cardNetInfo;
	int sequence = 0;
	//读取dat文件中索引
	while (inFile.read((char*)&index, sizeof(char))) 
	{
		if (index == ':') 
		{
			cardNetInfo.cardnet = str;
			cardNetInfo.count = 0;
			_cardNetInfos.push_back(cardNetInfo);
			str = "";
			continue;
		}
		else if (index == '#') 
		{
			_cardNetInfos[sequence].count = stoi(str);
			sequence++;
			str = "";
			continue;
		}
		else if (index == '\n') 
		{
			break;
		}
		str = str + index;
	}
	inFile.close();
	int category = _cardNetInfos.size();

	//读取压缩数据
	ifstream inFile2(_path + "/CompressedData.dat", ios::binary);
	byte dataStream;
	bool isBinary = false;
	while (inFile2.read((char*)&dataStream, sizeof(char))) 
	{
		//cout << tempB << endl;
		if (isBinary == false) 
		{
			if (dataStream == '\n') 
			{
				isBinary = true;
				continue;
			}
			continue;
		}
		_loadedData.push_back(dataStream);
	}
	//cout << _loadedData.size() << endl;
	inFile.close();
	int loadDataLength = _loadedData.size();
	string strs = "";
	int num = 0;
	int bListType = 0;
	string cardID = "";
	string cardNet = "";
	int count = 0;
	sequence = 0;
	ofstream outfile(_path + "/OriginalData.txt", ios::app);
	for (int i = 0; i < loadDataLength; i = i + 9) 
	{
		strs = "";
		for (int j = 0; j < 8; j++) 
		{
			num = _loadedData[(j + i)] & 0xff;
			strs += decompressData(num);
		}
		num = _loadedData[(i + 8)] & 0xff;
		bListType = num;
		cardID = strs;
		cardNet = _cardNetInfos[sequence].cardnet;
		count++;
		if (count >= _cardNetInfos[sequence].count) 
		{
			sequence++;
			count = 0;
		}
		cout << cardNet << " " << cardID << " " << bListType << endl;
		outfile << cardNet << " " << cardID << " " << bListType << endl;
	}

	outfile.close();
}

void Compression::loadData()
{
	clock_t start = clock();
	ifstream inFile(_path + "/CompressedData.dat", ios::binary);
	char index;
	CardNetInfo cardNetInfo;
	string str = "";
	int sequence = 0;
	//读取dat文件中索引
	while (inFile.read((char*)&index, sizeof(char))) 
	{
		if (index == ':') 
		{
			cardNetInfo.cardnet = str;
			cardNetInfo.count = 0;
			_cardNetInfos.push_back(cardNetInfo);
			str = "";
			continue;
		}
		else if (index == '#') 
		{
			_cardNetInfos[sequence].count = stoi(str);
			sequence++;
			str = "";
			continue;
		}
		else if (index == '\n') 
		{
			break;
		}
		str = str + index;
	}
	inFile.close();
	int category = _cardNetInfos.size();

	ifstream inFile2(_path + "/CompressedData.dat", ios::binary);
	byte dataStream;
	bool isBinary = false;
	//读取压缩数据
	while (inFile2.read((char*)&dataStream, sizeof(char))) 
	{
		//cout << tempB << endl;
		if (isBinary == false) 
		{
			if (dataStream == '\n') 
			{
				isBinary = true;
				continue;
			}
			continue;
		}
		_loadedData.push_back(dataStream);
	}
	inFile.close();
	//cout << "111" << endl;
	int loadedDataLength = _loadedData.size();
	cout << "loadedDataLength: " << (loadedDataLength/9) << endl;
	clock_t finish = clock();
	float timediff = (float)(finish - start) / CLOCKS_PER_SEC;
	cout << "读取文件时间: " << timediff << endl;
	ofstream outfile(_path + "/Log.txt", ios::app);
	outfile << "读取文件时间: " << timediff << endl;
	outfile.close();
	int flag = 0;
	cout << "是否进行查询(0否，1是): ";
	cin >> flag;
	if (flag == 1) {
		searchCardID();
	}
}

void Compression::searchCardID() 
{
	if (connectMysql() == false)
	{
		cout << (mysql_error(&_mysql));
		return;
	}
	int isContinue = 1;
	string str = "";
	vector<string> strings;
	float start;
	int pre = 0;
	int post = 0;
	int pos = 0;
	int lbound = 0;
	int rbound = -1;
	bool flag = true;
	bool isExist = false;
	int count = 0;
	bool* isCorrect = new bool;
	*isCorrect = true;
	ByteArray byteArray;
	ByteArray tempByteArray;
	int category = _cardNetInfos.size();
	ofstream outfile(_path + "/Log.txt", ios::app);
	while (isContinue == 1) {
		str = "";
		strings.clear();
		//cout << "输入查询的CardID: ";
		int a = 121;
		for (int i = 0; i < a; i++) 
		{
			cin >> str;
			strings.push_back(str);
		}
		
		for (int i = 0; i < strings.size(); i++) {
			start = clock();
			pre = 0;
			post = 0;
			pos = 0;
			lbound = 0;
			rbound = -1;
			flag = true;
			isExist = false;
			count = 0;
			str = strings[i];
			if (str.size() < 16) 
			{
				for (int j = str.size(); j < 16; j++)
				{
					str = str + "0";
				}
			}
			else if(str.size()> 16)
			{
				cout << "数据有误: " << str << endl;
				outfile << "数据有误: " << str << endl;
				continue;
			}
			cout << "str: " << str << endl;
			cout << strings[i] << " - ";
			outfile << strings[i] << " - ";
			for (int j = 0; j < 15; j = j + 2) 
			{
				byteArray.array[count++] = compressData(str.substr(j, 2), isCorrect);
			}
			if (*isCorrect == false)
			{
				cout << "数据有误!" << endl;
				outfile << "数据有误!" << endl;
				*isCorrect = true;
				continue;
			}
			for (int j = 0; j < category; j++) 
			{
			
				lbound = pre = rbound + 1;
				rbound = post = lbound + _cardNetInfos[j].count - 1;
				
				while (post >= pre) 
				{
					//cout << j << endl;
					pos = (post + pre) / 2;
					pos = pos * 9;
					for (int k = 0; k < 8; k++) {
						tempByteArray.array[k] = _loadedData[pos + k];
					}
					if (byteArray > tempByteArray) {
						pre = (pos / 9) + 1;
					}
					else if (byteArray < tempByteArray) {
						post = (pos / 9) - 1;
					}
					else{
						isExist = true;
						cout << _cardNetInfos[j].cardnet << " - " << (_loadedData[pos + 8] & 0xff) << "   ";
						outfile << _cardNetInfos[j].cardnet << " - " << (_loadedData[pos + 8] & 0xff) << "   ";

						
						flag = true;
						int num = pos;
						while (flag == true) {
							pos = pos - 9;
						
							if ((pos / 9) < lbound) {
								flag = false;
								break;
							}
							for (int k = 0; k < 8; k++) {
								tempByteArray.array[k] = _loadedData[pos + k];
							}
							if (byteArray == tempByteArray) {
								cout << (_loadedData[pos + 8] & 0xff) << "   ";
								outfile << (_loadedData[pos + 8] & 0xff) << "   ";
							}
							else {
								flag = false;
								break;
							}
						}
					
						flag = true;
						pos = num;
						while (flag == true) {
							pos = pos + 9;
							
							if ((pos / 9) > rbound) {
								flag = false;
								break;
							}
							for (int k = 0; k < 8; k++) {
								tempByteArray.array[k] = _loadedData[pos + k];
							}
						
							if (byteArray == tempByteArray) {
								cout << (_loadedData[pos + 8] & 0xff) << "   ";
								outfile << (_loadedData[pos + 8] & 0xff) << "   ";
							}
							else {
								flag = false;
								break;
							}
						}
					
						break;
					}
					
				}
			
			}
			if (isExist == false) {
				cout << "无此CardID          ";
				outfile << "无此CardID          ";
			}
			
			float finish = clock();
			float timediff = (float)(finish - start) / CLOCKS_PER_SEC;
			cout << strings[i] << "搜索时间: " << timediff << endl;
			outfile << strings[i] << "搜索时间: " << timediff << endl;
			
		}

		cout << "是否继续(0结束，1继续): ";
		cin >> isContinue;
	}
	outfile.close();
}