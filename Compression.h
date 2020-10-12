#ifndef __COMPRESSION_H__
#define __COMPRESSION_H__
#include<vector>
#include<string>
#include"mysql.h"

struct CardNetInfo {
	std::string cardnet;
	int count;
};
struct ByteArray
{
	byte array[8];
	friend bool operator < (const struct ByteArray& ls, const struct ByteArray& rs);
	friend bool operator > (const struct ByteArray& ls, const struct ByteArray& rs);
	friend bool operator == (const struct ByteArray& ls, const struct ByteArray& rs);

};

inline bool operator < (const struct ByteArray& ls, const struct ByteArray& rs)
{
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
inline bool operator > (const struct ByteArray& ls, const struct ByteArray& rs)
{
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
inline bool operator == (const struct ByteArray& ls, const struct ByteArray& rs)
{
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

class Compression {
public:
	Compression();
	void storeData();			//��ѹ�����ݴ���dat�ļ���
	void checkData();			//��ѹ�����ݽ�ѹ��������txt�ļ���
	void loadData();
	void searchCardID();
private:
	const std::string _path = "./data";
	bool connectMysql();			//����mysql���ݿ�
	byte compressData(std::string str, bool* isCorrect);	//ѹ������
	std::string decompressData(int num);
	std::string getCardnetInfo();		//�õ�cardnet�����Լ���ֵ����������
	std::vector<byte> _loadedData;
	std::vector<CardNetInfo> _cardNetInfos;
	MYSQL _mysql;
	std::string _tableName;
};

#endif // !__COMPRESSION_H__

