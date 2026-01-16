#include <stdio.h>
#include <string.h>
#pragma warning(disable:4996)
 
//https://www.jianshu.com/p/aadd86f8d38e

/*
* 函数名：			GetIniKeyString
* 入口参数：		title
*						配置文件中一组数据的标识
*					key
*						这组数据中要读出的值的标识
*					filename
*						要读取的文件路径
* 返回值：			找到需要查的值则返回正确结果
*					否则返回NULL
*/


int* String2IntAarray(char* str)
{
	char *p = str;
	static int num[100];
	int i, r, v, n;
	i = 0;
	//从字符串中读取出内容
	while (1)
	{
		r = sscanf(p, "%d,%n", &v, &n);
		if (1 == r)
		{
			num[i++] = v;
			p += n;
		}
		else if (0 == r)
		{
			p++;
		}
		else
		{
			break;
		}
	}
	//打印读取到的内容
	// for (n = 0; n < i - 1; n++)
	// 	printf("%d ", num[n]);
	// printf("%d\n", num[i - 1]);
	return num;
}

double* String2DoubleAarray(char* str)
{
	char *p = str;
	static double num[100];
	int i, r, n;
	double v;
	i = 0;
	//从字符串中读取出内容
	while (1)
	{
		r = sscanf(p, "%lf,%n", &v, &n);//lf for double in sscanf
		if (1 == r)
		{
			num[i++] = v;
			p += n;
		}
		else if (0 == r)
		{
			p++;
		}
		else
		{
			break;
		}
	}
	//打印读取到的内容
	// for (n = 0; n < i - 1; n++)
	// 	printf("%d ", num[n]);
	// printf("%d\n", num[i - 1]);
	return num;
}


char *GetIniKeyString(char *title, char *key, char *filename)
{
	FILE *fp=NULL;
	int  flag = 0;
	char sTitle[32], *wTmp;
	static char sLine[1024];
 
	sprintf(sTitle, "[%s]", title);
	if (NULL == (fp = fopen(filename, "r"))) {
		fclose(fp);
		perror("fopen");
		return NULL;
	}
	else {
		while (NULL != fgets(sLine, 1024, fp)) {
			// 这是注释行
			if (0 == strncmp("//", sLine, 2)) continue;
			if ('#' == sLine[0])              continue;
 
			wTmp = strchr(sLine, '=');//position of "="
			char *space_posi = strchr(sLine, ' ');//position of space, compare to delete
			int cmp_len = wTmp - sLine;//len if no space before "="
			if (space_posi < wTmp) {
				cmp_len=space_posi-sLine;
			}
			if ((NULL != wTmp) && (1 == flag)) {
				if (0 == strncmp(key, sLine, cmp_len)) { // 长度依文件读取的为准
					sLine[strlen(sLine) - 1] = '\0';
					fclose(fp);
					return wTmp + 1;
				}
			}
			else {
				if (0 == strncmp(sTitle, sLine, strlen(sLine) - 1)) { // 长度依文件读取的为准
					flag = 1; // 找到标题位置
				}
			}
		}
	}
	fclose(fp);
	return NULL;
}
 
/*
* 函数名：			GetIniKeyInt
* 入口参数：		title
*						配置文件中一组数据的标识
*					key
*						这组数据中要读出的值的标识
*					filename
*						要读取的文件路径
* 返回值：			找到需要查的值则返回正确结果
*					否则返回NULL
*/
int GetIniKeyInt(char *title, char *key, char *filename)
{
	return atoi(GetIniKeyString(title, key, filename));
}
 
/*
* 函数名：			PutIniKeyString
* 入口参数：		title
*						配置文件中一组数据的标识
*					key
*						这组数据中要读出的值的标识
*					val
*						更改后的值
*					filename
*						要读取的文件路径
* 返回值：			成功返回  0
*					否则返回 -1
*/
int PutIniKeyString(char *title, char *key, char *val, char *filename)
{
	FILE *fpr, *fpw;
	int  flag = 0;
	char sLine[1024], sTitle[32], *wTmp;
 
	sprintf(sTitle, "[%s]", title);
	if (NULL == (fpr = fopen(filename, "r"))) {
		fclose(fpr);
		perror("fopen");
		return NULL;
	}
	sprintf(sLine, "%s.tmp", filename);
	if (NULL == (fpw = fopen(sLine, "w"))){
		fclose(fpw);
		perror("fopen");
		return NULL;
	}
	while (NULL != fgets(sLine, 1024, fpr)) {
		if (2 != flag) { // 如果找到要修改的那一行，则不会执行内部的操作
			wTmp = strchr(sLine, '=');
			if ((NULL != wTmp) && (1 == flag)) {
				if (0 == strncmp(key, sLine, wTmp - sLine)) { // 长度依文件读取的为准
					flag = 2;// 更改值，方便写入文件
					sprintf(wTmp + 1, "%s\n", val);
				}
			}
			else {
				if (0 == strncmp(sTitle, sLine, strlen(sLine) - 1)) { // 长度依文件读取的为准
					flag = 1; // 找到标题位置
				}
			}
		}
 
		fputs(sLine, fpw); // 写入临时文件
	}
	fclose(fpr);
	fclose(fpw);
 
	sprintf(sLine, "%s.tmp", filename);
	return rename(sLine, filename);// 将临时文件更新到原文件
}
 
/*
* 函数名：			PutIniKeyString
* 入口参数：		title
*						配置文件中一组数据的标识
*					key
*						这组数据中要读出的值的标识
*					val
*						更改后的值
*					filename
*						要读取的文件路径
* 返回值：			成功返回  0
*					否则返回 -1
*/
int PutIniKeyInt(char *title, char *key, int val, char *filename)
{
	char sVal[32];
	sprintf(sVal, "%d", val);
	return PutIniKeyString(title, key, sVal, filename);
}
 
 
 
// int main(int argc, char *argv[])
// {
// 	printf("%s\n", GetIniKeyString("DOG", "name", "config.ini"));
// 	printf("%d\n", GetIniKeyInt("DOG", "age", "config.ini"));
//  	PutIniKeyInt("DOG", "age", 56, "config.ini");
// 	system("pause");
// 	return 0;
// }
