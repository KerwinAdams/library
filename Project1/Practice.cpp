#include<iostream>
#include<stdlib.h>
#include<cstdlib>
#include<time.h>
using namespace std;
int main()
{
	srand(time(0));
	int choice;
	cout << "����Ӳ�Ұɣ�����ܼ򵥣����泯�����ʤ����֮����ܣ�Ҫ����" << endl << "1��ʼ" << endl << "0û��Ȥ" << endl;
	cin >> choice;
	if (choice == 1)
	{
		while (choice == 1)
		{
			int num = rand() % (100)+1;
			if (num >= 50)
			{
				cout << "���棬��Ӯ�ˣ��Ƿ������" << endl << "1����" << endl << "0������" << endl;
				cin >> choice;
			}
			else
			{
				cout << "���棬�����ˣ��Ƿ������" << endl << "1����" << endl << "0������" << endl;
				cin >> choice;
			}
		}
		cout << "��ӭ���� ^-^" << endl;
	}
	else
	{
		cout << "��ӭ���� ^-^" << endl;
	}
return 0;
}