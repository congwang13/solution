// lec8.cpp : 
//

#include "stdafx.h"
#include <stdio.h>
#include <iostream>
#include <fstream>

using namespace std;

int physic[0x80][0x20];
int disk[0x80][0x20];
int start =  0x6c;

int search(int addr){
    //找到三个偏移量：pde pte physical 的偏移
    int pde_off = (addr>>10) & 31;
    int pte_off = (addr>>5) & 31;
    int phy_off = addr & 31;
    //开始找第一个page directory entry的内容
	int pde_con = physic[start][pde_off];
    //看看是不是valid
    bool pde_valid = pde_con >> 7;
    printf("pde index:0x%x  pde contents:(0x%x, bin: , valid %d, pfn 0x%x(page 0x%x))\n", pde_off, pde_con, pde_valid, pde_con&127, pde_con&127);
    if (pde_valid){
        //把valid位去掉
        pde_con &= 127;
        //直接通过该值，得到对应的数组地址
        int pte_addr = pde_con;
        //通过pte offset 来找内容
		int pte_con = physic[pte_addr][pte_off];
        //找到valid位
        bool pte_valid = pte_con >> 7;
        printf("pte index:0x%x  pte contents:(0x%x, bin: , valid %d, pfn 0x%x)\n", pte_off, pte_con, pte_valid, pte_con&127);
        if (pte_valid){
            pte_con &= 127;
            int phy_addr = pte_con;
			int phy_con = physic[phy_addr][phy_off];
            printf(" To Physical Address 0x%x --> value : 0x%x\n", (32*pte_con+phy_off), phy_con);
            return phy_con;
        } else {
            pte_con &= 127;
            int disk_addr = pte_con;
            int disk_con = disk[disk_addr][phy_off];
            printf("To Disk Sector Address 0x%x --> value : 0x%x\n", (32*pte_con+phy_off), disk_con);
            return disk_con;
        }
    } else {
        printf("Fault (page directory entry not valid)\n");
        return -1;
    }
}

int trans(char a,char b){
	int num = 0;
	if ((int)a < 58)
        num = (int)(a-'0');
    else
        num = (int)(a-'a')+10;
	if((int)b < 58)
		num = num * 16 + (int)(b-'0');
	else
		num = num * 16 + (int)(b-'a')+10;

	return num;
}

void readfaile(char* failename, bool isDisk)
{
	ifstream fin;
	fin.open(failename);
	char line[120];
	int i=0,j=0;
	if (! fin.is_open()) {
        cout << "Error opening file"<<endl;; 
    }
	else
		while(i<128){
			fin.getline(line,120);
			//cout<<"i= "<<i<<"  "<<line<<endl;
			for(j = 0;j < 32;j++){
				if(isDisk)
					disk[i][j] = trans(line[j*3+9],line[j*3+10]);
				else
					physic[i][j] = trans(line[j*3+9],line[j*3+10]);
			}
			i++;
		}

}

int main()
{
	readfaile("mem.txt", false);
	readfaile("disk.txt", true);
	//search(0x1e6f);
	search(0x6653);
    search(0x1c13);
    search(0x6890);
    search(0xaf6);
    search(0x1e6f);
	system("pause");
	return 0;
}

