#include<iostream>
#include<vector>
using namespace std;
#define MAX_SIZE 16

int main()
{
   /*int k=1;
   int text[5];
   int pattern[4][4]; 
   int R[5][4][1];
   int startloc[MAX_SIZE];
   int editdist[MAX_SIZE];
   for (size_t i = 0; i < MAX_SIZE; i++) 
        text[i] = i % 4;
    for (size_t i = 0; i < MAX_SIZE/2; i++) 
        pattern[(i%4)][i] = 1;
    int m=MAX_SIZE/2,n=MAX_SIZE,align_num=0;*/
    int text[5]={0,2,3,2,1};//A=0,C=1,G=2,T=3 AGTGC
    int pattern[4][4]={{1,0,0,0},{0,0,0,0},{0,1,0,1},{0,0,1,0}}; 
    int k=1;
    int R[6][5][2],startloc[5],editdist[5];
    int n=5,m=4,align_num=0;
    for (int d = 0; d <= k; d++) {
            for (int text_pos = 0; text_pos <= 5; text_pos++) {
                for (int que_pos = 0; que_pos <= 4; que_pos++) {

                    // Get previous sum
                    int D,S,M,I,result;
                    result=que_pos==0?1:text_pos==0?0:0;
                    if(d==0)
                    {
                        if(text_pos<=n && que_pos<=m && text_pos>0 && que_pos>0)
                        {
                            cout<<"a"<<text_pos<<" "<<R[text_pos-1][que_pos-1][d]<<" "<<text[text_pos]<<" "<<pattern[text[text_pos]][que_pos-1]<<endl;

                            result=R[text_pos-1][que_pos-1][d]&&pattern[text[text_pos-1]][que_pos-1];
                        }
                    }
                    else
                    {
                        if(text_pos<=n && que_pos<=m && text_pos>0 && que_pos>0)
                        {
                            cout<<"b"<<text_pos<<" "<<R[text_pos-1][que_pos-1][d]<<" "<<text[text_pos]<<" "<<pattern[text[text_pos]][que_pos-1]<<endl;

                            M=R[text_pos-1][que_pos-1][d]&&pattern[text[text_pos-1]][que_pos-1];
                            S=R[text_pos-1][que_pos-1][d-1];
                            D=R[text_pos][que_pos-1][d-1];
                            I=R[text_pos-1][que_pos][d-1];
                            result=M||S||D||I;
                        }
                    }
                    R[text_pos][que_pos][d]=result;
                    if(que_pos==m && result==1)
                    {
                        startloc[align_num]=text_pos;
                        editdist[align_num]=d;
                        align_num++;
                    }
                }
            }
        }

for(int p=0;p<2;p++)
{
    for(int i=0;i<5;i++)
    {
        for(int j=0;j<6;j++)
        {
            cout<<R[j][i][p]<<" ";
        }
        cout<<endl;
    }
    cout<<endl;
}
return 0;
}
