#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdlib.h>
#include <cstring>

#define ERROR 0
#define OK 1
#define UNIT (-1)
#define CONFLICT (-2)
#define status int
#define X 3
#define SATISFIABLE 1
#define UNSATISFIABLE 2

typedef struct LiteralList {
    int LiteralName;//文字下标
    bool IsComplement;//是否为负文字（1 for𠃍x，0 for x）
    int ClauseName;
    struct LiteralList * NC;//下一个有该文字的子句中该文字的位置(NextClause)
    struct LiteralList * NL;//下一个文字(NextLiteral)
}LiteralList;

typedef struct literal{
    struct LiteralList * FPC;//第一个含该文字的子句(FirstClause)
    struct LiteralList * FNC;
    int value;//取0（false）或1（true）或3（unassigned literal）
    bool IsFlipped;//是否翻转
}literal;

typedef struct clause{
    struct LiteralList * p1;
    struct LiteralList * p2;//引用指针
    struct LiteralList * FL;//第一个文字（FirstLiteral）
}clause;

typedef struct counter{
    LiteralList * LastClause;//上一个该文字出现的位置
    int count;//该正文字出现的次数
}counter;

status SAT ();

void QuickSort(counter * check, int * &order, int low, int high);
status LoadInstance(clause * &p, literal * &q, int &LNum, int &CNum, counter * &check, int * &order, LiteralList **&UnitPtr, char *FilePath, int *&AssignedLiteral);
int ReadNum(FILE *fp);
status ExamineLiteral(clause * p,literal * q, int LNum);
status ExamineClause(clause * p, int CNum);
status LoadInstance2(clause * &p, literal * &q, int &LNum, int &CNum, counter * &check, int * &order, LiteralList **&UnitPtr, char *FilePath, int *&AssignedLiteral);

int Assign(clause *p, literal *q, int LName, int AssignValue, LiteralList **UnitPtr, int &UTop);
bool DPLLEnhance(clause *p,literal *q, int LNum, int CNum, int *order, LiteralList **UnitPtr, int *AssignedLiteral);

status OutPut(literal *q, int state, double t, int LNum, char *FilePath);
//void PrintResult(literal *q, int LNum);

status Sudoku();
status Generate(int Sudoku[9][9]);
status PrintSudoku(int Sudoku[9][9]);
status SolveSudoku(int Sudoku[9][9], char FilePath[100]);
status SudokuToCnf(int Sudoku[9][9], char FilePath[100]);
status dig(int Sudoku[9][9], int level);
status check(int Sudoku[9][9], int row, int column);
status checkreturn(int Sudoku[9][9], int row, int column, int &m, int &n);
status Play(int Sudoku[9][9]);

int top=1;

int main() {
    int op=1;
    while(op)
    {
        system("cls");	printf("\n\n");
        printf("SAT or Sudoku?(1 for SAT, 2 for Sudoku, 0 for exit)\n");
        scanf("%d",&op);
        if(op==1) SAT();
        else if (op==2) Sudoku();
        else if(op==0) printf("Thanks for using our system!\n");
        else
        {
            printf("ERROR: No such function!\n");
            system("pause");
        }
    }
    return 0;
}

status SAT ()
{
    int op = 1;
    clause * p= nullptr;
    literal * q= nullptr;
    counter * check= nullptr;
    LiteralList **UnitPtr= nullptr;
    int * order= nullptr;
    int CNum=0, LNum=0;//文字数、子句数
    int *AssignedLiteral;
    clock_t start = 0, end = 0;
    double t1, t2;
    int state=0;
    char FilePath[100];
    while (op)
    {
        system("cls");  printf("\n\n");
        printf("              SAT\n");
        printf("---------------------------------------------------\n");
        printf("    1.  LoadInstance       2.  ExamineClause\n");
        printf("    3.  ExamineLiteral     4.  SolveInstance\n");
        printf("    0.  exit               \n");
        printf("---------------------------------------------------\n");
        printf("    Select a function[0~4]:\n");
        if (op > 4 || op < 0) getchar();
        if (!scanf("%d", &op)) op = 100;
        switch (op)
        {
            case 1:
            {
                if(LoadInstance(p,q,LNum,CNum,check,order,UnitPtr,FilePath,AssignedLiteral)) printf("Success!\n");
                else printf("failed\n");
                system("pause");
                break;
            }
            case 2:
            {
                if(ExamineClause(p,CNum)) printf("Success!\n");
                else printf("failed\n");
                system("pause");
                break;
            }
            case 3:
            {
                if(ExamineLiteral(p,q,LNum)) printf("Success!\n");
                else printf("failed\n");
                system("pause");
                break;
            }
            case 4:
            {
                if(!p)
                {
                    printf("Instance not loaded!\n");
                    break;
                }
                start = clock();
                state=DPLLEnhance(p,q,LNum,CNum,order,UnitPtr,AssignedLiteral);
                end = clock();
                t1 = (double)(end - start);
                t1 /= CLOCKS_PER_SEC;
                if(state)
                {
                    printf("%fs\n",t1);
                    printf("Satisfiable!\n");
                    OutPut(q,state,t1,LNum,FilePath);
//                    PrintResult(q,LNum);
                }
                else
                {
                    printf("%fs\n",t1);
                    printf("Unsatisfiable!\n");
                    OutPut(q,state,t1,LNum,FilePath);
                }
                system("pause");
                break;
            }
            case 0:
            {
                break;
            }
            default:
            {
                printf("No such function!\n");
                system("pause");
                break;
            }
        }
    }
    return 0;
}

int ReadNum(FILE *fp)//读取数字
{
    char c; bool flag= false;
    int i=0, queue[10], num=1, result=0;
    while(((c=fgetc(fp))>'9'||c<'0')&&c!='-');
    if(c=='-')
    {
        flag= true;
        num=0;
    }//判断前面是否有负号
    else queue[0]=c-'0';
    while((c=fgetc(fp))<='9'&&c>='0')//按字符读取数字
    {
        queue[num]=c-'0';
        num++;
    }
    for(i=1,num--;num>=0;num--)//恢复为数字
    {
        result+=queue[num]*i;
        i*=10;
    }
    if (result==0) return 0;//读取数字为0（一个子句已经读完）
    else if(flag) return -result;//负文字
    else return result;//正文字
}

status LoadInstance(clause * &p, literal * &q, int &LNum, int &CNum, counter * &check, int * &order, LiteralList **&UnitPtr, char *FilePath, int *&AssignedLiteral)
//p：子句头结点；q文字头节点；LNum文字数量；子句数量
{
    int i, num;//计数器
    char c;//用于获取字符
    LiteralList * m= nullptr, *n= nullptr;//用于创建文字链表
    clause * x= nullptr, *y= nullptr;//用于创建子句链表
    if(p) //已载入算例
    {
        printf("Error, an instance is already loaded!\n");
        system("pause");
        return ERROR;
    }
    printf("Please enter your path\n");
    scanf("%s",FilePath);
    FILE * fp=fopen(FilePath,"r");
    if(!fp)
    {
        printf("Error, incorrect path!\n");
        system("pause");
        return ERROR;
    }
    while (fgetc(fp)=='c')//处理注释行
        while (fgetc(fp)!='\n');
    for(i=0;i<5;i++)//读取' CNF '
        fgetc(fp);
    LNum=ReadNum(fp);
    CNum=ReadNum(fp);
    q=(literal *)malloc(LNum* sizeof(literal));//文字线性表
    for(i=0;i<LNum;i++)
    {
        q[i].IsFlipped= false;
        q[i].value=X;
        q[i].FPC= nullptr;
        q[i].FNC= nullptr;
    }//初始化文字
    check=(counter *)malloc(2*LNum * sizeof(counter));
    for(i=0;i<2*LNum;i++)
    {
        check[i].LastClause=nullptr;
        check[i].count=0;
    }//初始化指针
    p=(clause *)malloc(CNum* sizeof(clause));
    for(i=0;i<CNum;i++)
    {
        p[i].FL= nullptr;
        p[i].p1= nullptr;
        p[i].p2= nullptr;
    }//初始化子句
    for(i=CNum;i>0;i--)
    {
        num=ReadNum(fp);
        n=(LiteralList *)malloc(sizeof(LiteralList));
        n->NL= nullptr;
        n->NC= nullptr;
        p[CNum-i].FL=n;
        n->ClauseName=CNum-i;
        if(num>0)//正文字
        {
            n->LiteralName=(num-1);
            n->IsComplement= false;
            check[num-1].count++;//文字计数器自加
            if(q[num-1].FPC!= nullptr)
                check[num-1].LastClause->NC=n;
            else
                q[num-1].FPC=n;//建立同一文字之间的链表
            check[num-1].LastClause=n;//记录该文字位置
        }
        else
        {
            n->LiteralName=(-num-1);
            n->IsComplement= true;
            check[-num-1+LNum].count++;//文字计数器自加
            if(q[-num-1].FNC!= nullptr)
                check[-num-1+LNum].LastClause->NC=n;
            else
                q[-num-1].FNC=n;//建立同一文字之间的链表
            check[-num-1+LNum].LastClause=n;//记录该文字位置
        }
        while (num=ReadNum(fp))
        {
            n= nullptr;
            n=(LiteralList *)malloc(sizeof(LiteralList));
            n->NL= p[CNum-i].FL;
            p[CNum-i].FL=n;
            n->NC= nullptr;
            n->ClauseName=CNum-i;
            if(num>0)
            {
                n->LiteralName=(num-1);
                n->IsComplement= false;
                check[num-1].count++;//文字计数器自加
                if(q[num-1].FPC!= nullptr)
                    check[num-1].LastClause->NC=n;//建立同一文字之间的链表
                else
                    q[num-1].FPC=n;
                check[num-1].LastClause=n;//记录该文字位置
            }
            else
            {
                n->LiteralName=(-num-1);
                n->IsComplement= true;
                check[-num-1+LNum].count++;//文字计数器自加
                if(q[-num-1].FNC!= nullptr)
                    check[-num-1+LNum].LastClause->NC=n;
                else
                    q[-num-1].FNC=n;//建立同一文字之间的链表
                check[-num-1+LNum].LastClause=n;//记录该文字位置
            }
        }
    }
    fclose(fp);
    for(i=0;i<CNum;i++)
    {
        if(p[i].FL->NL!= nullptr)
        {
            p[i].p1=p[i].FL;
            p[i].p2=p[i].FL->NL;
        }
        else
        {
            p[i].p1=p[i].FL;
            p[i].p2=p[i].FL;
        }
    }
    AssignedLiteral=(int *)malloc(LNum* sizeof(int));
    for(i=0;i<LNum;i++)
        AssignedLiteral[i]=0;
    order=(int *)malloc(2*LNum*sizeof(int));//创建变元数量排序数组
    for(i=0;i<2*LNum;i++)
        order[i]=i;
    QuickSort(check,order,0,2*LNum-1);//根据出现次数对变元排序
    UnitPtr=(LiteralList **)malloc(LNum* sizeof(int));
    top=0;
    return OK;
}

status LoadInstance2(clause * &p, literal * &q, int &LNum, int &CNum, counter * &check, int * &order, LiteralList **&UnitPtr, char *FilePath, int *&AssignedLiteral)
//p：子句头结点；q文字头节点；LNum文字数量；子句数量
{
    int i, num;//计数器
    char c;//用于获取字符
    LiteralList * m= nullptr, *n= nullptr;//用于创建文字链表
    clause * x= nullptr, *y= nullptr;//用于创建子句链表
    if(p) //已载入算例
    {
        printf("Error, an instance is already loaded!\n");
        system("pause");
        return ERROR;
    }
    FILE * fp=fopen(FilePath,"r");
    if(!fp)
    {
        printf("Error, incorrect path!\n");
        system("pause");
        return ERROR;
    }
    while (fgetc(fp)=='c')//处理注释行
        while (fgetc(fp)!='\n');
    for(i=0;i<5;i++)//读取' CNF '
        fgetc(fp);
    LNum=ReadNum(fp);
    CNum=ReadNum(fp);
    q=(literal *)malloc(LNum* sizeof(literal));//文字线性表，多的一个是冲突
    for(i=0;i<LNum;i++)
    {
        q[i].IsFlipped= false;
        q[i].value=X;
        q[i].FPC= nullptr;
        q[i].FNC= nullptr;
    }//初始化文字
    check=(counter *)malloc(2*LNum * sizeof(counter));
    for(i=0;i<2*LNum;i++)
    {
        check[i].LastClause=nullptr;
        check[i].count=0;
    }//初始化指针
    p=(clause *)malloc(CNum* sizeof(clause));
    for(i=0;i<CNum;i++)
    {
        p[i].FL= nullptr;
        p[i].p1= nullptr;
        p[i].p2= nullptr;
    }//初始化子句
    for(i=CNum;i>0;i--)
    {
        num=ReadNum(fp);
        n=(LiteralList *)malloc(sizeof(LiteralList));
        n->NL= nullptr;
        n->NC= nullptr;
        p[CNum-i].FL=n;
        n->ClauseName=CNum-i;
        if(num>0)//正文字
        {
            n->LiteralName=(num-1);
            n->IsComplement= false;
            check[num-1].count++;//文字计数器自加
            if(q[num-1].FPC!= nullptr)
                check[num-1].LastClause->NC=n;
            else
                q[num-1].FPC=n;//建立同一文字之间的链表
            check[num-1].LastClause=n;//记录该文字位置
        }
        else
        {
            n->LiteralName=(-num-1);
            n->IsComplement= true;
            check[-num-1+LNum].count++;//文字计数器自加
            if(q[-num-1].FNC!= nullptr)
                check[-num-1+LNum].LastClause->NC=n;
            else
                q[-num-1].FNC=n;//建立同一文字之间的链表
            check[-num-1+LNum].LastClause=n;//记录该文字位置
        }
        while (num=ReadNum(fp))
        {
            n= nullptr;
            n=(LiteralList *)malloc(sizeof(LiteralList));
            n->NL= p[CNum-i].FL;
            p[CNum-i].FL=n;
            n->NC= nullptr;
            n->ClauseName=CNum-i;
            if(num>0)
            {
                n->LiteralName=(num-1);
                n->IsComplement= false;
                check[num-1].count++;//文字计数器自加
                if(q[num-1].FPC!= nullptr)
                    check[num-1].LastClause->NC=n;//建立同一文字之间的链表
                else
                    q[num-1].FPC=n;
                check[num-1].LastClause=n;//记录该文字位置
            }
            else
            {
                n->LiteralName=(-num-1);
                n->IsComplement= true;
                check[-num-1+LNum].count++;//文字计数器自加
                if(q[-num-1].FNC!= nullptr)
                    check[-num-1+LNum].LastClause->NC=n;
                else
                    q[-num-1].FNC=n;//建立同一文字之间的链表
                check[-num-1+LNum].LastClause=n;//记录该文字位置
            }
        }
    }
    fclose(fp);
    for(i=0;i<CNum;i++)
    {
        if(p[i].FL->NL!= nullptr)
        {
            p[i].p1=p[i].FL;
            p[i].p2=p[i].FL->NL;
        }
        else
        {
            p[i].p1=p[i].FL;
            p[i].p2=p[i].FL;
        }
    }
    AssignedLiteral=(int *)malloc(LNum* sizeof(int));
    for(i=0;i<LNum;i++)
        AssignedLiteral[i]=0;
    order=(int *)malloc(2*LNum*sizeof(int));//创建变元数量排序数组
    for(i=0;i<2*LNum;i++)
        order[i]=i;
    QuickSort(check,order,0,2*LNum-1);//根据出现次数对变元排序
    UnitPtr=(LiteralList **)malloc(LNum* sizeof(int));
    top=0;
    return OK;
}

void QuickSort(counter * check, int * &order, int low, int high)
{
    int x,i,j;
    if(low<high)
    {
        i=low;j=high;x=order[i];
        do
        {
            while (i<j&&check[order[j]].count>=check[x].count)
                j--;
            if(i<j)
            {
                order[i]=order[j];
                i++;
                while (i<j&&check[order[i]].count<=check[x].count)
                    i++;
                if(i<j)
                {
                    order[j]=order[i];
                    j--;
                }
            }
        }while (i!=j);
        order[i]=x;
        QuickSort(check,order,low,i-1);
        QuickSort(check,order,i+1,high);
    }
}

status ExamineClause(clause * p, int CNum)
{
    int i;
    LiteralList *m= nullptr;
    if(p== nullptr)//算例未载入
    {
        printf("Instance not loaded!\n");
        system("pause");
        return ERROR;
    }
    printf("Please enter the clause you want to examine (1-%d).\n ",CNum);
    scanf("%d",&i);
    if(i<1||i>CNum)//输入大于语句数
    {
        printf("There is no such clause!\n");
        system("pause");
        return ERROR;
    }
    m=p[i-1].FL;
    while(m)//依次输出文字
    {
        if(m->IsComplement) printf("-");
        printf("%d\t",m->LiteralName+1);
        m=m->NL;
    }
    printf("\n");
    return OK;
}

status ExamineLiteral(clause * p,literal * q, int LNum)
{
    int i;
    LiteralList *m= nullptr;
    if(!p)//算例未载入
    {
        printf("Instance not loaded!\n");
        system("pause");
        return ERROR;
    }
    printf("Please enter the literal you want to examine (1-%d).\n ",LNum);
    scanf("%d",&i);
    if(i<1||i>LNum+1)//输入大于语句数
    {
        printf("There is no such literal!\n");
        system("pause");
        return ERROR;
    }
    m=q[i-1].FPC;
    while(m)//依次输出子句
    {
        printf("%d\t",m->ClauseName);
        m=m->NC;
    }
    printf("\n");
    return OK;
}

bool DPLLEnhance(clause *p,literal *q, int LNum, int CNum, int *order, LiteralList **UnitPtr, int *AssignedLiteral)
{
    int ATop=1, UTop=1;
    int state=0;
    int curLiteralName=0;
    int i;
    while (true)
    {
        if(state!=CONFLICT&&state!=UNIT)
        {
            if(UTop==1)
            {
                for(i=2*LNum-1;i>=0;i--)//从排序数组中寻找未赋值
                {
                    if(order[i]>LNum-1)
                    {
                        if(q[order[i]-LNum].value==X)//找到未赋值
                            break;
                    }
                    else if(q[order[i]].value==X)//找到未赋值
                        break;
                }
                if(i==-1)//全部已赋值
                    return true;
                else
                {
                    if(order[i]>LNum-1)
                    {
                        state=Assign(p,q,order[i]-LNum,0,UnitPtr,UTop);
                        AssignedLiteral[ATop]=order[i]-LNum;
                        ATop++;
                    }
                    else
                    {
                        state=Assign(p,q,order[i],1,UnitPtr,UTop);
                        AssignedLiteral[ATop]=order[i];
                        ATop++;
                    }
                }
            }//退出
        }
        if(state==UNIT)
        {
            while (state!=CONFLICT&&UTop>1)
            {
                curLiteralName=UnitPtr[UTop-1]->LiteralName;
                UTop--;
                if (q[curLiteralName].value!=X)
                    continue;
                q[curLiteralName].IsFlipped= true;
                if(UnitPtr[UTop]->IsComplement)//unit为负文字
                {
                    state=Assign(p,q,curLiteralName,0,UnitPtr,UTop);
                }
                else//p2为正文字
                {
                    state=Assign(p,q,curLiteralName,1,UnitPtr,UTop);
                }
                AssignedLiteral[ATop]=curLiteralName;
                ATop++;
            }
        }
        if(state==CONFLICT)
        {
            while (ATop>1&&q[AssignedLiteral[ATop-1]].IsFlipped)
            {
                ATop--;
                q[AssignedLiteral[ATop]].value=X;
                q[AssignedLiteral[ATop]].IsFlipped= false;
            }
            if(ATop>1)
            {
                UTop=1;
                state=Assign(p,q,AssignedLiteral[ATop-1],!q[AssignedLiteral[ATop-1]].value,UnitPtr,UTop);
                q[AssignedLiteral[ATop-1]].IsFlipped= true;
            }//未翻转则翻转
            else
                return false;
        }
    }
}

int Assign(clause *p, literal *q, int LName, int AssignValue, LiteralList **UnitPtr, int &UTop)
{
    LiteralList *m= nullptr, *n= nullptr;
    clause *curClause= nullptr;
    literal *curLiteral= nullptr;
    bool flag= false;
    q[LName].value=AssignValue;//赋值
    if(AssignValue)
        m=q[LName].FNC;
    else
        m=q[LName].FPC;
    while (m)//对每一个含被赋值文字的子句
    {
        curClause=&p[m->ClauseName];
        if(m==curClause->p1)//被赋值文字被p1所指
        {
            n=curClause->FL;
            while(n)//对该子句的每一个文字
            {
                curLiteral=&q[n->LiteralName];
                if(((curLiteral->value==X)||(curLiteral->value==1&&!n->IsComplement) || (curLiteral->value==0&&n->IsComplement))&&n!=curClause->p2)
                    //该文字为非p2的true或unassign
                {
                    curClause->p1=n;
                    break;
                }//p1指向该文字
                n=n->NL;
            }
            if(!n)//该子句除p2以外全部为false
            {
                if((q[curClause->p2->LiteralName].value==1&&curClause->p2->IsComplement)||(q[curClause->p2->LiteralName].value==0&&!curClause->p2->IsComplement))
                    //p2所指为false,即出现conflict
                {
                    return CONFLICT;
                }
                if(q[curClause->p2->LiteralName].value==X)//p2所指为unassigned,即出现unit
                {
                    UnitPtr[UTop]=curClause->p2;
                    UTop++;
                    if(!flag)
                        flag= true;
                }
            }
        }
        else if (m==curClause->p2)//被赋值元素被p2所指
        {
            n=curClause->FL;
            while(n)//对该子句的每一个文字
            {
                if((q[n->LiteralName].value==X||(q[n->LiteralName].value==1&&!n->IsComplement) || (q[n->LiteralName].value==0&&n->IsComplement))&&n!=p[m->ClauseName].p1)
                    //该文字为非p1的unassign或者true
                {
                    p[m->ClauseName].p2=n;//p2指向该文字
                    break;
                }
                n=n->NL;
            }
            if(!n)//该子句除p1以外全部为false
            {
                if((q[p[m->ClauseName].p1->LiteralName].value==1&&p[m->ClauseName].p1->IsComplement)||(q[p[m->ClauseName].p1->LiteralName].value==0&&!p[m->ClauseName].p1->IsComplement))
                {
                    return CONFLICT;
                }
                if(q[curClause->p1->LiteralName].value==X)//p1所指为unassigned,即出现unit
                {
                    UnitPtr[UTop]=curClause->p1;
                    UTop++;
                    if(!flag)
                        flag= true;
                }
            }
        }
        m=m->NC;
    }
    if(flag)
        return UNIT;
    return OK;
}

status OutPut(literal *q, int state, double t, int LNum, char *FilePath)
{
    FILE *fp;
    int i=0;
    int time=0;
    int j=0;
    time=(int)(t*1000);
    char outputfile[100];
    for(i=0;FilePath[i]!='\0';i++)
        outputfile[i]=FilePath[i];
    outputfile[i]='\0';
    outputfile[i-1]='s';
    outputfile[i-2]='e';
    outputfile[i-3]='r';
    fp=fopen(outputfile,"w");
    fprintf(fp,"s ");
    fprintf(fp,"%d\nv ",state);
    for(i=0;i<LNum;i++)
    {
        if(q[i].value==0)
        {
            j=-(i+1);
            fprintf(fp,"%d ",j);
        }
        else
            fprintf(fp,"%d ",i+1);
    }
    fprintf(fp,"\nt %d",time);
    fclose(fp);
    return OK;
}

//void PrintResult(literal *q, int LNum)
//{
//    int i;
//    for(i=0;i<LNum;i++)
//    {
//        if(i%5)
//            printf("%d:%d\t\t",i+1,q[i].value);
//        else
//            printf("\n%d:%d\t\t",i+1,q[i].value);
//    }
//}

status Sudoku()
{
    int op=1;
    int i=0,j=0;
    int Sudoku[9][9];
    for(i=0;i<9;i++)
        for(j=0;j<9;j++)
            Sudoku[i][j]=0;
    int level=0;
    srand((unsigned int)time(NULL));
    while (op)
    {
        system("cls");  printf("\n\n");
        printf("              Sudoku\n");
        printf("---------------------------------------------------\n");
        printf("    1.  Generate           2.  Play\n");
        printf("    0.  exit               \n");
        printf("---------------------------------------------------\n");
        printf("    Select a function[0~2]:\n");
        if (op > 2 || op < 0) getchar();
        if (!scanf("%d", &op)) op = 100;
        switch (op)
        {
            case 1:
            {
                while (!Generate(Sudoku));
                system("pause");
                break;
            }
            case 2:
            {
                for(i=0;i<9;i++)
                    for(j=0;j<9;j++)
                        if(Sudoku[i][j]==0)
                        {
                            printf("Please generate a Sudoku first!\n");
                            system("pause");
                            i=9;
                            j=9;
                        }
                if(i==10)
                    break;
                printf("Please select a difficult level from 1(easiest) to 5(most difficult):\n");
                scanf("%d",&level);
                if(level<1||level>5)
                {
                    printf("Invalid difficulty level!\n");
                    system("pause");
                    break;
                }
                else
                    dig(Sudoku,level);
                Play(Sudoku);
                system("pause");
                break;
            }
            case 0:
            {
                break;
            }
            default:
            {
                printf("No such function!\n");
                system("pause");
                break;
            }
        }
    }
    return 0;
}

status Play(int Sudoku[9][9])
{
    int op=1,i=0,j=0,row=0,column=0,a=0, m=0, n=0;
    int PlaySudoku[9][9], SudokuSolution[9][9];
    char FilePath[100]="SolveSudoku.cnf";
    for(i=0;i<9;i++)
        for(j=0;j<9;j++)
        {
            PlaySudoku[i][j]=Sudoku[i][j];
            SudokuSolution[i][j]=Sudoku[i][j];
        }
    SudokuToCnf(SudokuSolution,FilePath);
    SolveSudoku(SudokuSolution,FilePath);
    while (op)
    {
        system("cls");  printf("\n\n");
        printf("              Sudoku\n");
        printf("--------------------------------------\n");
        PrintSudoku(PlaySudoku);
        printf("--------------------------------------\n");
        printf(" 1.  Assign           2.  Delete\n");
        printf(" 3.  Submit           4.  Give a solution\n");
        printf(" 0.  exit               \n");
        printf("--------------------------------------\n");
        printf("    Select a function[0~4]:\n");
        if (op > 4 || op < 0) getchar();
        if (!scanf("%d", &op)) op = 100;
        switch (op)
        {
            case 1:
            {
                printf("Please enter where you want to assign a value(format:row column value):\n");
                scanf("%d %d %d",&row,&column,&a);
                if(Sudoku[row-1][column-1])
                {
                    printf("You cannot assign a value there!\n");
                    system("pause");
                    break;
                }
                else if(a<1||a>9||row<1||row>9||column<1||column>9)
                {
                    printf("Invalid value!\n");
                    system("pause");
                    break;
                }
                else
                    PlaySudoku[row-1][column-1]=a;
                break;
            }
            case 2:
            {
                printf("Please enter where you want to delete a value(format:row column):\n");
                scanf("%d %d",&row,&column);
                if(Sudoku[row-1][column-1])
                {
                    printf("You cannot delete a value there!\n");
                    system("pause");
                    break;
                }
                else if(a<1||a>9||row<1||row>9||column<1||column>9)
                {
                    printf("Invalid value!\n");
                    system("pause");
                    break;
                }
                else
                    PlaySudoku[row-1][column-1]=0;
                break;
            }
            case 3:
            {
                for(i=0;i<9;i++)
                    for(j=0;j<9;j++)
                        if(PlaySudoku[i][j]==0)
                        {
                            printf("Not all cells are assigned!\n");
                            system("pause");
                            i=9;
                            j=9;
                        }
                if(i==10)
                    break;
                for(row=0;row<9;row++)
                    for(column=0;column<9;column++)
                    {
                        if(!checkreturn(PlaySudoku,row,column,m,n))
                        {
                            printf("Not a solution: there is a conflict between row %d column %d and row %d column %d!\n",row+1,column+1,m+1,n+1);
                            system("pause");
                            row=9;
                            column=9;
                        }
                    }
                if(row==10)
                    break;
                printf("Congratulations! You find a solution!\n");
                system("pause");
                break;
            }
            case 4:
            {
                printf("Here is a possible solution to the Sudoku:\n");
                PrintSudoku(SudokuSolution);
                system("pause");
                break;
            }
            case 0:
            {
                break;
            }
            default:
            {
                printf("No such function!\n");
                system("pause");
                break;
            }
        }
    }
    return 0;
}

status dig(int Sudoku[9][9], int level)
{
    int remove=0;
    int i=0;
    int row=0,column=0;
    if(level==1)
        remove=30;
    else if(level==2)
        remove=40;
    else if(level==3)
        remove=48;
    else if (level==4)
        remove=52;
    else
        remove=58;
    for(i=0;i<remove;i++)
    {
        row=rand()%9;
        column=rand()%9;
        if(Sudoku[row][column]!=0)
            Sudoku[row][column]=0;
        else
            i--;
    }
    return OK;
}

status Generate(int Sudoku[9][9])
{
    int i=0,j=0;
    for(i=0;i<9;i++)
        for(j=0;j<9;j++)
            Sudoku[i][j]=0;
    char FilePath[100]="SudokuGeneration.cnf";
    int row=0,column=0;
    for(i=0;i<11;i++)
    {
        row=rand()%9;
        column=rand()%9;
        Sudoku[row][column]=rand()%9+1;
        if(!check(Sudoku,row,column))
        {
            Sudoku[row][column]=0;
            i--;
        }
    }
    SudokuToCnf(Sudoku,FilePath);
    if(SolveSudoku(Sudoku,FilePath)==SATISFIABLE)
    {
        printf("全盘已生成！\n");
        PrintSudoku(Sudoku);
        return OK;
    }
    else
        return ERROR;
}

status PrintSudoku(int Sudoku[9][9])
{
    int i=0,j=0;
    for(i=0;i<9;i++)
    {
        if(i==3||i==6)
            printf("------------------------------------\n");
        for(j=0;j<9;j++)
        {
            if(j==3||j==6)
                printf("|");
            if(j<8)
            {
                if(!Sudoku[i][j])
                    printf("_\t");
                else
                    printf("%d\t",Sudoku[i][j]);
            }
            else
            {
                if(!Sudoku[i][j])
                    printf("_\n");
                else
                    printf("%d\n",Sudoku[i][j]);
            }
        }
    }
    return OK;
}

status SolveSudoku(int Sudoku[9][9], char FilePath[100])
{
    clause * p= nullptr;
    literal * q= nullptr;
    counter * check= nullptr;
    int *AssignedLiteral;
    LiteralList **UnitPtr= nullptr;
    int *buffer= nullptr;
    int * order= nullptr;
    int CNum=0, LNum=0;//文字数、子句数
    int i=0;
    if(!LoadInstance2(p,q,LNum,CNum,check,order,UnitPtr,FilePath,AssignedLiteral))
    {
        printf("Wrong file path!\n");
        system("pause");
        return ERROR;
    }
    if(DPLLEnhance(p,q,LNum,CNum,order,UnitPtr,AssignedLiteral))
    {
        for(i=0;i<LNum;i++)
        {
            if (q[i].value)
                if(i-i/10*10+1!=10)
                    Sudoku[i/100][i/10-i/100*10]=i-i/10*10+1;
        }
        return SATISFIABLE;
    }
    else
        return UNSATISFIABLE;
}

status SudokuToCnf(int Sudoku[9][9], char FilePath[100])
{
    FILE *fp;
    int i=0,j=0, k=0, m=0, n=0, num=1, a=0 ,b=0;
    int stack[82];
    for(i=0;i<9;i++)
        for(j=0;j<9;j++)
            if(Sudoku[i][j]!=0)
            {
                stack[num]=100*i+10*j+Sudoku[i][j];
                num++;
            }
    fp=fopen(FilePath,"w");
    fprintf(fp,"p cnf 889 %d\n",11745+num-1);
    while (num>1)
    {
        fprintf(fp,"%d 0\n",stack[num-1]);
        num--;
    }

    //81 保证每一个格都有值：
    for(i=0;i<9;i++)//i行
        for(j=0;j<9;j++)//j列
            for(k=0;k<9;k++)//赋值为k+1
            {
                fprintf(fp,"%d ",100*i+10*j+k+1);
                if(k==8)
                    fprintf(fp,"0\n ");
            }

    //81*(8+7+……+1) 保证每一个格不会被赋予不同的值：
    for(i=0;i<9;i++)//i行
        for(j=0;j<9;j++)//j列
            for(m=0;m<9;m++)//ij被赋为m+1
                for(n=m+1;n<9;n++)//ij被赋为n+1
                    fprintf(fp,"-%d -%d 0\n",i*100+j*10+m+1,i*100+j*10+n+1);

    //81*(8+7+……+1) 保证每一行不会出现相同值：
    for(i=0;i<9;i++)//i行
        for(k=0;k<9;k++)//值不同时为k+1
            for(m=0;m<9;m++)//m列
                for(n=m+1;n<9;n++)//n列
                    fprintf(fp,"-%d -%d 0\n",i*100+m*10+k+1,i*100+n*10+k+1);

    //81*(8+7+……+1) 保证每一列不会出现相同值：
    for(j=0;j<9;j++)//j列
        for(k=0;k<9;k++)//值不同时为k+1
            for(m=0;m<9;m++)//m行
                for(n=m+1;n<9;n++)//n行
                    fprintf(fp,"-%d -%d 0\n",m*100+j*10+k+1,n*100+j*10+k+1);

    //81*(8+7+……+1) 保证每一个3x3不会出现相同值：
    //将9块分为3大行3大列
    for(a=0;a<9;a+=3)//对于3大行的每一行
        for(b=0;b<9;b+=3)//对于3大列的每一列
            for(k=0;k<9;k++)//值不同时为k+1
                for(m=0;m<9;m++)//m行
                    for(n=m+1;n<9;n++)//n行
                        fprintf(fp,"-%d -%d 0\n",(a+m/3)*100+(b+m-m/3*3)*10+k+1,(a+n/3)*100+(b+n-n/3*3)*10+k+1);
    fclose(fp);
    return OK;
}

status check(int Sudoku[9][9], int row, int column)
{
    int i=0,j=0;
    for(j=0;j<9;j++)
    {
        if(j==column)
            continue;
        else if(Sudoku[row][j]==Sudoku[row][column])
            return ERROR;
    }
    for(i=0;i<9;i++)
    {
        if(i==row)
            continue;
        else if(Sudoku[i][column]==Sudoku[row][column])
            return ERROR;
    }
    for(i=row/3*3;i<row/3*3+3;i++)
        for(j=column/3*3;j<column/3*3+3;j++)
        {
            if(i==row&&j==column)
                continue;
            else if(Sudoku[i][j]==Sudoku[row][column])
                return ERROR;
        }
    return OK;
}

status checkreturn(int Sudoku[9][9], int row, int column, int &m, int &n)
{
    int i=0,j=0;
    for(j=0;j<9;j++)
    {
        if(j==column)
            continue;
        else if(Sudoku[row][j]==Sudoku[row][column])
        {
            m=row;
            n=j;
            return ERROR;
        }
    }
    for(i=0;i<9;i++)
    {
        if(i==row)
            continue;
        else if(Sudoku[i][column]==Sudoku[row][column])
        {
            m=i;
            n=column;
            return ERROR;
        }
    }
    for(i=row/3*3;i<row/3*3+3;i++)
        for(j=column/3*3;j<column/3*3+3;j++)
        {
            if(i==row&&j==column)
                continue;
            else if(Sudoku[i][j]==Sudoku[row][column])
            {
                m=i;
                n=j;
                return ERROR;
            }
        }
    return OK;
}