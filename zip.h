#pragma once
#include "mysql.h"
#include<iostream>
#include<string>
#include<vector>
#include<math.h>
#include<fstream>
using namespace std;

struct UserInfo {
	int CardNet;
	int BListType;
	int status;
	string CardID;
};

struct Info {
	string cardnet;
	int count;
};

class DataZip {
private:
	MYSQL mysql;
	int category;
	vector<Info> cardnets;
	string index;
public:
	void sqlconn();			//链接mysql数据库
	byte zip(string str);	//压缩数据
	void categories();		//得到cardnet类数以及其值，建立索引
	void store();			//将压缩数据存入dat文件中
	void check();			//将压缩数据解压缩并存入txt文件中
};

void DataZip::store() {
	categories();
	ofstream ioutfile("D:/test.dat", ios::app);
	ioutfile << index << endl;
	ioutfile.close();
	sqlconn();

	string sql = "";
	byte infos;
	string strs[3];
	char* p;
	string str;
	string temp;
	MYSQL_ROW row;
	for (int i = 0; i < category; i++) {
		cout << i << endl;
		MYSQL_RES* res;
		ofstream outfile("D:/test.dat", ios::app | ios::binary);
		sql = "SELECT * FROM tbl_paraminfo WHERE CardNet = ";
		sql = sql + cardnets[i].cardnet;//  + " LIMIT" + to_string(pos) + "," + to_string(count)
		p = _strdup(sql.c_str());
		mysql_query(&mysql, p);
		res = mysql_store_result(&mysql);
		while (row = mysql_fetch_row(res))
		{
			strs[0] = row[1];
			strs[1] = row[2];
			strs[2] = row[3];

			if (strs[1].size() == 1)
				strs[1] = "00" + strs[1];
			else if (strs[1].size() == 2)
				strs[1] = "0" + strs[1];

			str = strs[0] + strs[1] + strs[2];
			string str2;
			int b = 0;
			for (int i = 0; i < 19; i = i + 2) {
				str2 = "";
				temp = str.substr(i, 2);
				infos = zip(temp);
				outfile.write((char*)&infos, sizeof(infos));
			}
			strs[0] = "";
			strs[1] = "";
			strs[2] = "";
			str = "";
			temp = "";
		}
		outfile.close();
		mysql_free_result(res);
			
		
	}
	mysql_close(&mysql);
	
}

void DataZip::check() {
	ifstream iInFile("D:test.dat", ios::in);
	if (!iInFile) {
		cout << "error" << endl;
		return;
	}
	char temp;
	string str="";
	int count = 0;
	//读取dat文件中索引
	while (iInFile.read((char*)&temp, sizeof(char))) {
		if (temp == ':') {
			Info inf;
			inf.cardnet = str;
			inf.count = 0;
			cardnets.push_back(inf);
			str = "";
			continue;
		}
		else if (temp == '#') {
			cardnets[count].count = stoi(str);
			count++;
			str = "";
			continue;
		}
		else if (temp == '\n') {
			break;
		}
		str = str + temp;
	}
	category = cardnets.size();
	//读取压缩数据
	ifstream inFile("D:test.dat", ios::binary);
	if (!inFile) {
		cout << "error" << endl;
		return;
	}
	byte tempB;
	int num;
	str = "";
	string sum = "";
	int flag = 0;
	int sumflag = 1;
	UserInfo uinfo;
	bool isBinary = false;
	ofstream outfile("D:/return.txt", ios::app);
	while (inFile.read((char*)&tempB, sizeof(char))) {
		//cout << tempB << endl;
		if (isBinary == false && tempB == '\n') {
			isBinary = true;
			continue;
		}
		if (isBinary == false)
			continue;
		num = tempB & 0xFF;
		str = str + to_string(num);
		if (str.size() == 1)
			str = "0" + str;
		else if (str.size() == 0)
			str = "00" + str;
		sum = sum + str;
		str = "";
		flag++;
		if (flag == 10) {
			int cou = 0;
			for (int i = 0; i < category; i++) {
				cou = cou + cardnets[i].count;
				if (sumflag <= cou) {
					uinfo.CardNet = stoi(cardnets[i].cardnet);
					break;
				}
			}
			uinfo.CardID= sum.substr(0, 16);
			uinfo.BListType = stoi(sum.substr(16, 3));
			uinfo.status = stoi(sum.substr(19, 1));
			cout << uinfo.CardNet << " " << uinfo.CardID << " " << uinfo.BListType << " " << uinfo.status << endl;
			outfile << uinfo.CardNet << " " << uinfo.CardID << " " << uinfo.BListType << " " << uinfo.status << endl;
			sum = "";
			flag = 0;
			sumflag++;
		}
	}
	inFile.close();
	outfile.close();
}

void DataZip::sqlconn() {
	mysql_init(&mysql);
	mysql_options(&mysql, MYSQL_SET_CHARSET_NAME, "gbk");
	//    mysql_real_connect参数：2.本地地址  3.你的mysql用户名  4.你的mysql密码   5.数据库名字  6.端口号
	if (mysql_real_connect(&mysql, "localhost", "root", "123456", "mydb", 3306, NULL, 0) == NULL) {
		cout << (mysql_error(&mysql));
	}
}

byte DataZip::zip(string str) {
	byte temp;
	int num = 0;
	num = str[0] - '0';
	num = num * 10 + (str[1] - '0');
	temp = (byte)(num & 0xff);
	return temp;
}

void DataZip::categories() {
	sqlconn();
	string sql = "";
	
	MYSQL_ROW row;
	MYSQL_RES* res;
	sql = "SELECT DISTINCT cardnet FROM tbl_paraminfo";
	char* p = _strdup(sql.c_str());
	mysql_query(&mysql, p);
	res = mysql_store_result(&mysql);
	string str = "";
	Info inf;
	while (row = mysql_fetch_row(res)) {
		str = row[0];
		inf.cardnet = str;
		inf.count = 0;
		cardnets.push_back(inf);
	}
	category = cardnets.size();
	
	for (int i = 0; i < category; i++) {
		sql = "select COUNT(*) from tbl_paraminfo where CardNet = ";
		sql = sql + cardnets[i].cardnet;
		p = _strdup(sql.c_str());
		mysql_query(&mysql, p);
		res = mysql_store_result(&mysql);
		while (row = mysql_fetch_row(res)) {
			str = row[0];
			cardnets[i].count = stoi(str);
		}
		//cout << i << endl;
	}
	/*for (int i = 0; i < category; i++)
		cout << cardnets[i].cardnet << ":" << cardnets[i].count << endl;*/
	mysql_free_result(res);
	mysql_close(&mysql);
	index = "";
	for (int i = 0; i < category; i++) {
		index = index + cardnets[i].cardnet + ":" + to_string(cardnets[i].count) + "#";
	}
	cout << index << endl;
}