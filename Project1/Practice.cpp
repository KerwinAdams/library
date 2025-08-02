#include<iostream>
#include<stdlib.h>
#include<cstdlib>
#include<time.h>
using namespace std;
int main()
{
	srand(time(0));
	int choice;
	cout << "来抛硬币吧，规则很简单，正面朝上你获胜，反之你落败，要玩吗？" << endl << "1开始" << endl << "0没兴趣" << endl;
	cin >> choice;
	if (choice == 1)
	{
		while (choice == 1)
		{
			int num = rand() % (100)+1;
			if (num >= 50)
			{
				cout << "正面，你赢了，是否继续？" << endl << "1继续" << endl << "0不玩了" << endl;
				cin >> choice;
			}
			else
			{
				cout << "反面，你输了，是否继续？" << endl << "1继续" << endl << "0不玩了" << endl;
				cin >> choice;
			}
		}
		cout << "欢迎再来 ^-^" << endl;
	}
	else
	{
		cout << "欢迎再来 ^-^" << endl;
	}
return 0;
}