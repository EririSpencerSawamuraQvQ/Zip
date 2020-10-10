#pragma once
#include "mysql.h"
#include<iostream>
#include<string>
#include<vector>
#include<math.h>
#include<fstream>
#include<unordered_map>
#include<map>
#include<ctime>
#include <iomanip>
using namespace std;

struct UserInfo {
	string CardNet;
	string BListType;
	string status;
	string CardID;
};

struct Info {
	string cardnet;
	int count;
};

struct Detail {
	string CardNet;
	string BListType;
};

struct DetailInfo {
	byte CardID[8];
	string BListType;
};

struct ALLINFOS{
	string CardNet;
	int CardNetCount;
	vector<DetailInfo> DetailInfos;
};

struct ByteArray {
	byte array[8] = { 0 };
	friend bool operator < (const struct ByteArray& ls, const struct ByteArray& rs);
	friend bool operator > (const struct ByteArray& ls, const struct ByteArray& rs);
	friend bool operator == (const struct ByteArray& ls, const struct ByteArray& rs);
};

inline bool operator < (const struct ByteArray& ls, const struct ByteArray& rs) {

	bool flag = false;
	for (int i = 0; i < 8; i++) {
		if (ls.array[i] < rs.array[i]) {
			flag = true;
			break;
		}
		else if (ls.array[i] > rs.array[i])
			break;
	}
	return flag;
}

inline bool operator > (const struct ByteArray& ls, const struct ByteArray& rs) {

	bool flag = false;
	for (int i = 0; i < 8; i++) {
		if (ls.array[i] > rs.array[i]) {
			flag = true;
			break;
		}
		else if (ls.array[i] < rs.array[i])
			break;
	}
	return flag;
}

inline bool operator == (const struct ByteArray& ls, const struct ByteArray& rs) {

	bool flag = true;
	for (int i = 0; i < 8; i++) {
		if (ls.array[i] < rs.array[i]) {
			flag = false;
			break;
		}
		else if (ls.array[i] > rs.array[i]) {
			flag = false;
			break;
		}
	}
	return flag;
}

class DataZip {
private:
	MYSQL mysql;
	int category;
	vector<byte> bytes;
	int bytesLength;
	vector<Info> cardnets;
	string index;
	string tableName = "tbl_paraminfo";//tbl_paraminfo
	map<ByteArray, vector<Detail>> maps;
	vector<ALLINFOS> allInfos;

	void sqlconn();			//链接mysql数据库
	byte zip(string str, bool& flag);	//压缩数据
	string unzip(int num);
	void categories();		//得到cardnet类数以及其值，建立索引
public:
	void store();			//将压缩数据存入dat文件中
	void check();			//将压缩数据解压缩并存入txt文件中
	void loadMemory();
	void search();
};


void DataZip::search() {

	sqlconn();
	string sql = "";
	MYSQL_ROW row;
	MYSQL_RES* res;
	sql = "SELECT CardID FROM " + tableName + " ORDER BY RAND() LIMIT 100";
	char* p = _strdup(sql.c_str());

	string tempStr = "";
	vector<string> strings;
	int isContinue = 1;
	ByteArray byteArray;
	int count = 0;
	bool errorflag = false;
	map<ByteArray, vector<Detail>>::iterator got;
	float start, finish;
	//float starttotal, finishtotal;
	float timediff;
	ofstream outfile("D:/Load.txt", ios::app);
	vector<Detail> details;
	ByteArray tempByteArray;
	int pre = 0;
	int post = 0;
	int pos = 0;
	int lbound = 0;
	int rbound = -1;
	bool tempFlag = true;
	bool isExist = false;
	int tempInt = 0;

	while (isContinue == 1) {
		/*mysql_query(&mysql, p);
		res = mysql_store_result(&mysql);
		while (row = mysql_fetch_row(res)) {
			tempStr = row[0];
			strings.push_back(tempStr);
		}*/

		cout << "输入查询的CardID: ";
		cin >> tempStr;
		strings.push_back(tempStr);

		/*starttotal = clock();*/
		for (int i = 0; i < strings.size(); i++) {
			start = clock();
			pre = 0;
			post = 0;
			pos = 0;
			lbound = 0;
			rbound = -1;
			tempFlag = true;
			isExist = false;
			count = 0;

			tempStr = strings[i];
			if (tempStr.size() != 16) {
				cout << tempStr << "输入错误!" << endl;
				continue;
			}
			cout << tempStr << " - ";
			for (int j = 0; j < (tempStr.size() - 1); j = j + 2)
				byteArray.array[count++] = zip(tempStr.substr(j, 2), errorflag);
			
			for (int j = 0; j < category; j++) {
				lbound = pre = rbound + 1;
				rbound = post = pre + cardnets[j].count - 1;
				while (post >= pre) {
					pos = ((post + pre) / 2) * 9;
					for (int k = 0; k < 8; k++) {
						tempByteArray.array[k] = bytes[pos + k];
					}
					if (byteArray > tempByteArray) {
						pre = (pos / 9) + 1;
					}
					else if (byteArray < tempByteArray) {
						post = (pos / 9) - 1;
					}
					else {
						isExist = true;
						cout << cardnets[j].cardnet << " - " << (bytes[pos + 8] & 0xff) << "   ";

						tempFlag = true;
						tempInt = pos;
						while (tempFlag == true) {
							pos = pos - 9;
							if ((pos / 9) < lbound) {
								tempFlag = false;
								break;
							}
							for (int k = 0; k < 8; k++) {
								tempByteArray.array[k] = bytes[pos + k];
							}
							if (byteArray == tempByteArray) {
								cout << (bytes[pos + 8] & 0xff) << "   ";
							}
							else {
								tempFlag = false;
								break;
							}
						}

						tempFlag = true;
						pos = tempInt;
						while (tempFlag == true) {
							pos = pos + 9;
							if ((pos / 9) > rbound) {
								tempFlag = false;
								break;
							}
							for (int k = 0; k < 8; k++) {
								tempByteArray.array[k] = bytes[pos + k];
							}
							if (byteArray == tempByteArray) {
								cout << (bytes[pos + 8] & 0xff) << "   ";
							}
							else {
								tempFlag = false;
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

			finish = clock();
			timediff = (float)(finish - start) / CLOCKS_PER_SEC;
			cout << strings[i] << "搜索时间: " << timediff << endl;
			outfile << strings[i] << "搜索时间: " << timediff << endl;
		}
		/*finishtotal = clock();
		timediff = (float)(finishtotal - starttotal) / CLOCKS_PER_SEC;
		cout << "总搜索时间: " << timediff << endl;
		outfile << "总搜索时间: " << timediff << endl;*/

		strings.clear();

		cout << "是否继续(0结束，1继续): ";
		cin >> isContinue;
	}

	//cout << maps.size() << endl;
	int sizeSum = bytes.size();
	double kb = (double)sizeSum / 1024;
	double mb = (double)sizeSum / 1048576;
	cout << "总大小: " << sizeSum << "B / " << kb << "KB / " << mb << "MB" << endl;
	outfile << "总大小: " << sizeSum << "B / " << kb << "KB / " << mb << "MB" << endl;
	outfile.close();

}

void DataZip::loadMemory() {
	//cout << setiosflags(ios::fixed);
	clock_t start = clock();
	char tempChar;
	Info tempInfo;
	int tempInt = 0;
	string tempStr = "";
	string tempStrs = "";
	byte tempByte;
	struct ByteArray tempByteArray;
	int count = 0;
	int CardNetFlag = 0;
	Detail tempDetail;
	bool isBinary = false;
	string tempCardID;
	int tempBListType = 0;
	int tempCardNet = 0;
	ALLINFOS allInfo;
	DetailInfo detailInfo;

	ifstream inFile("D:/test.dat", ios::binary);
	ifstream iInFile("D:/test.dat", ios::in);
	ofstream outfile("D:/Load.txt", ios::app);

	//读取dat文件中索引
	while (iInFile.read((char*)&tempChar, sizeof(char))) {
		if (tempChar == ':') {
			tempInfo.cardnet = tempStr;
			tempInfo.count = 0;
			cardnets.push_back(tempInfo);
			tempStr = "";
			continue;
		}
		else if (tempChar == '#') {
			cardnets[tempInt].count = stoi(tempStr);
			tempInt++;
			tempStr = "";
			continue;
		}
		else if (tempChar == '\n') {
			break;
		}
		tempStr = tempStr + tempChar;
	}
	category = cardnets.size();


	//读取压缩数据

	while (inFile.read((char*)&tempByte, sizeof(char))) {
		//cout << tempB << endl;
		if (isBinary == false) {
			if (tempByte == '\n') {
				isBinary = true;
				continue;
			}
			continue;
		}
		bytes.push_back(tempByte);
	}
	inFile.close();
	//cout << "111" << endl;
	bytesLength = bytes.size();
	cout << "bytesLength: " << bytesLength << endl;
	clock_t finish = clock();
	float timediff = (float)(finish - start) / CLOCKS_PER_SEC;
	cout << "读取文件时间: " << timediff << endl;
	outfile << "读取文件时间: " << timediff << endl;

	int flag = 0;
	cout << "是否进行查询(0否，1是): ";
	cin >> flag;
	if (flag == 1)
		search();

}


void DataZip::sqlconn() {
	mysql_init(&mysql);
	mysql_options(&mysql, MYSQL_SET_CHARSET_NAME, "gbk");
	//    mysql_real_connect参数：2.本地地址  3.你的mysql用户名  4.你的mysql密码   5.数据库名字  6.端口号
	if (mysql_real_connect(&mysql, "localhost", "root", "123456", "mydb2", 3306, NULL, 0) == NULL) {
		cout << (mysql_error(&mysql));
	}
}

void DataZip::store() {
	categories();
	ofstream ioutfile("D:/test.dat", ios::app);
	ioutfile << index << endl;
	ioutfile.close();
	sqlconn();

	string sql = "";
	byte info;
	string strs[2];
	int temp = 0;
	char* p;
	string str;
	MYSQL_ROW row;
	bool errorflag = false;
	for (int i = 0; i < category; i++) {
		cout << i << endl;
		MYSQL_RES* res;
		ofstream outfile("D:/test.dat", ios::app | ios::binary);
		//tbl_paraminfo
		sql = "SELECT * FROM " + tableName + " WHERE CardNet = " +cardnets[i].cardnet+" ORDER BY CardID ASC";
		p = _strdup(sql.c_str());
		mysql_query(&mysql, p);
		res = mysql_store_result(&mysql);
		while (row = mysql_fetch_row(res))
		{
			strs[0] = row[1];
			strs[1] = row[2];

			str = strs[0];
			temp = str.size();

			for (int i = 0; i < (temp - 1); i = i + 2) {
				info = zip(str.substr(i, 2), errorflag);
				outfile.write((char*)&info, sizeof(info));
			}
			if (errorflag == true) {
				cout << str << endl;
				errorflag = false;
			}
			temp = stoi(strs[1]);
			info = (byte)(temp & 0xff);
			outfile.write((char*)&info, sizeof(info));
		}
		outfile.close();
		mysql_free_result(res);


	}
	mysql_close(&mysql);

}

void DataZip::check() {

	char tempChar;
	Info tempInfo;
	string tempStr = "";
	string tempStrs = "";
	int tempInt = 0;
	int count = 0;
	int CardNetFlag = 0;
	byte tempByte;
	UserInfo uinfo;
	bool isBinary = false;

	ifstream iInFile("D:/test.dat", ios::in);
	ifstream inFile("D:/test.dat", ios::binary);
	ofstream outfile("D:/return.txt", ios::app);

	//读取dat文件中索引
	while (iInFile.read((char*)&tempChar, sizeof(char))) {
		if (tempChar == ':') {
			
			tempInfo.cardnet = tempStr;
			tempInfo.count = 0;
			cardnets.push_back(tempInfo);
			tempStr = "";
			continue;
		}
		else if (tempChar == '#') {
			cardnets[count].count = stoi(tempStr);
			count++;
			tempStr = "";
			continue;
		}
		else if (tempChar == '\n') {
			break;
		}
		tempStr = tempStr + tempChar;
	}
	category = cardnets.size();
	//读取压缩数据
	iInFile.close();

	while (inFile.read((char*)&tempByte, sizeof(char))) {
		//cout << tempB << endl;
		if (isBinary == false) {
			if (tempByte == '\n') {
				isBinary = true;
				continue;
			}
			continue;
		}
		bytes.push_back(tempByte);
	}
	inFile.close();
	bytesLength = bytes.size();
	count = 0;
	for (int i = 0; i < bytesLength; i = i + 9) {
		tempStrs = "";
		for (int j = 0; j < 8; j++) {
			tempInt = bytes[(j + i)] & 0xff;
			tempStrs += unzip(tempInt);
		}
		tempInt = bytes[(i + 8)] & 0xff;
		uinfo.BListType = to_string(tempInt);
		uinfo.CardID = tempStrs;
		uinfo.CardNet = cardnets[CardNetFlag].cardnet;
		count++;
		if (count >= cardnets[CardNetFlag].count) {
			CardNetFlag++;
			count = 0;
			//cout << CardNetFlag << endl;
		}
		cout << uinfo.CardNet << " " << uinfo.CardID << " " << uinfo.BListType << endl;
		outfile << uinfo.CardNet << " " << uinfo.CardID << " " << uinfo.BListType << endl;
	}



	outfile.close();

}

void DataZip::categories() {
	sqlconn();
	string sql = "";

	MYSQL_ROW row;
	MYSQL_RES* res;
	//tbl_paraminfo
	sql = "SELECT DISTINCT cardnet FROM " + tableName + " ORDER BY CardNet ASC";
	char* p = _strdup(sql.c_str());
	mysql_query(&mysql, p);
	res = mysql_store_result(&mysql);
	string str = "";
	Info tempInfo;
	while (row = mysql_fetch_row(res)) {
		str = row[0];
		tempInfo.cardnet = str;
		tempInfo.count = 0;
		cardnets.push_back(tempInfo);
	}
	category = cardnets.size();

	for (int i = 0; i < category; i++) {
		//tbl_paraminfo
		sql = "select COUNT(*) from " + tableName + " where CardNet = "+ cardnets[i].cardnet;
		p = _strdup(sql.c_str());
		mysql_query(&mysql, p);
		res = mysql_store_result(&mysql);
		while (row = mysql_fetch_row(res)) {
			str = row[0];
			cardnets[i].count = stoi(str);
		}
		//cout << i << endl;
	}
	mysql_free_result(res);
	mysql_close(&mysql);
	index = "";
	for (int i = 0; i < category; i++) {
		index = index + cardnets[i].cardnet + ":" + to_string(cardnets[i].count) + "#";
	}
	cout << index << endl;
}

byte DataZip::zip(string str, bool& flag) {
	byte tempByte;
	int num = 0;
	int temp = 0;
	for (int i = 0; i < 2; i++) {
		if (str[i] - '0' >= 0 && str[i] - '0' <= 9) {
			temp = str[i] - '0';
			num = (num << 4) | (temp & 0xf);
		}
		else if (str[i] - 'a' >= 0 && str[i] - 'a' <= 5) {
			temp = 10 + (str[i] - 'a');
			num = (num << 4) | (temp & 0xf);
		}
		else if (str[i] - 'A' >= 0 && str[i] - 'A' <= 5) {
			temp = 10 + (str[i] - 'A');
			num = (num << 4) | (temp & 0xf);
		}
		else {
			cout << "error:" << str << endl;
			flag = true;
		}
	}

	tempByte = (byte)(num & 0xff);
	return tempByte;
}

string DataZip::unzip(int num) {
	string str = "";
	int temp[2];
	temp[0] = (num >> 4) & 0xf;
	temp[1] = num & 0xf;
	for (int i = 0; i < 2; i++) {
		if (temp[i] >= 0 && temp[i] <= 9) {
			str = str + to_string(temp[i]);
		}
		else if (temp[i] >= 10 && temp[i] <= 15) {
			char t = 'A';
			t = t + (temp[i] - 10);
			str = str + t;
		}
	}
	return str;
}

