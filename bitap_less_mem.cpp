#include<iostream>
#include<vector>
using namespace std;
#define MAX_SIZE 16

int main()
{
    int text[5]={0,2,3,2,1};//A=0,C=1,G=2,T=3 AGTGC
    int pattern[4][8]={{1,0,0,0,1,0,0,0},{0,0,0,0,0,0,0,1},{0,1,0,1,0,1,0,0},{0,0,1,0,0,0,1,0}}; //AGTG AGTC
    int k=1;
    int R[64],startloc[10],editdist[10];
    int n=5,m=4,align_num=0;
    for (int text_pos = 0; text_pos <= 5; text_pos++)
     {
        for (int d = 0; d <= k; d++) 
        {
            for(int num_pat=0;num_pat<2;num_pat++)
            {
                for (int que_pos = 0; que_pos <= 4; que_pos++) 
                {
                    int D,S,M,I,result;
                    result=que_pos==0?1:text_pos==0?0:0;
                    if(d==0 && text_pos>0 && que_pos>0)
                    {
                        int ind=num_pat*2*(k+1)*(m+1)+((text_pos-1)%2)*(k+1)*(m+1)+d*(m+1)+(que_pos-1);
                        result=R[ind]&&pattern[text[text_pos-1]][num_pat*4+que_pos-1];
                    }
                    else if(text_pos>0 && que_pos>0)
                    {
                            int ind1=num_pat*2*(k+1)*(m+1) + ((text_pos-1)%2)*(k+1)*(m+1) + d*(m+1) + (que_pos-1);
                            int ind2=num_pat*2*(k+1)*(m+1) + ((text_pos-1)%2)*(k+1)*(m+1) + (d-1)*(m+1) + (que_pos-1);
                            int ind3=num_pat*2*(k+1)*(m+1) + ((text_pos)%2)*(k+1)*(m+1) + (d-1)*(m+1) + (que_pos-1);
                            int ind4=num_pat*2*(k+1)*(m+1) + ((text_pos-1)%2)*(k+1)*(m+1) + (d-1)*(m+1) + (que_pos);
                            M=R[ind1]&&pattern[text[text_pos-1]][num_pat*4+que_pos-1];
                            S=R[ind2];
                            D=R[ind3];
                            I=R[ind4];
                            result=M||S||D||I;
                    }
                    int ind=num_pat*2*(k+1)*(m+1)+((text_pos)%2)*(k+1)*(m+1)+d*(m+1)+(que_pos);
                    R[ind]=result;
                    //cout<<R[ind];
            
                    if(que_pos==m && result==1)
                    {
                        startloc[align_num]=text_pos;
                        editdist[align_num]=d;
                        align_num++;
                    }
                }
            }
            //cout<<" ";
        }
        //cout<<endl;
    }

cout<<align_num<<endl;
for(int p=0;p<align_num;p++)
{
    cout<<startloc[p]<<" "<<editdist[p]<<endl;
}
return 0;
}
