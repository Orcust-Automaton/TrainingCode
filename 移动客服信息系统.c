// Ubuntu 20.04  gcc 移动客服信息系统.c -o 移动客服信息系统
// Windows 10  gcc -fexec-charset=gbk 移动客服信息系统.c -o 移动客服信息系统.exe

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define FILE_NAME_USER "data_user.txt"
#define FILE_NAME_RECORD "data_record.txt"
#define MAX_USER_COUNTS 0X100
#define MAX_RECORD_COUNTS 0X1000
#define MAX_USER_NAME_SIZE 0x10
#define MAX_USER_ADDR_SIZE 0X28

#define STDIN 0

#define RECHARGE 1
#define CONSUM 0

// 定义部分常量

struct Time
{
    int year;
    int month;
    int day;
};

// 自定义一个简单的时间结构体记录年月日

struct UserInfo
{
    int no;
    char * name;
    char * addr;
    unsigned long int number;
    int type;
};
// 定义用户结构体，存放用户数据

struct RecordInfo
{
    int no;
    int user_no;
    unsigned long int money;
    struct Time * time;
    int type;
};
// 定义记录结构体，存放记录信息

struct Form
{
    int month;
    unsigned long int recharge;
    unsigned long int consum;
};
// 单个月份的消费表，存放当月的消费和充值记录

struct Users
{
    int counts;
    struct UserInfo *user[MAX_USER_COUNTS];
};
// 存放所有用户

struct Records
{
    int counts;
    struct RecordInfo *record[MAX_RECORD_COUNTS];
};
// 存放所有记录

struct ReportForms
{
    int year;
    struct Form *month[12];
};
// 一年的表单，存放单签年份每一个月的消费和充值记录

struct Records * select_record_by_no( int user_no );
struct UserInfo * get_user( unsigned long int i);
unsigned long int menu();
unsigned long int get_num( int fd );
struct Time * get_time();
void input( int fd, char *buffer ,unsigned int size, char target );
void show_user( struct UserInfo * user );
void show_record( struct RecordInfo * record , char * name);
void init();
void gap();
void bye();
void load_user();
void save_user();
void save_record();
void load_record();
void add_user();
void list_user();
void edit_user();
void add_record_recharge();
void add_record_consum();
void show_form();
void show_user_record();

struct Users *users;
struct Records *records;

void init(){
    setvbuf(stdin, 0, 2, 0);
    setvbuf(stdout, 0, 2, 0);
    setvbuf(stderr, 0, 2, 0);

    users = ( struct Users *)calloc(sizeof( struct Users ) , 1);
    records = ( struct Records *)calloc(sizeof( struct Records ) , 1);
    if(!users || !records){
        puts("内存分配错误");
        return;
    }
    load_user();
    load_record();

    // 加载文件中的数据

    printf("\n");
    // 为防止服务端与用户端读入读入与输出混乱设置缓冲区
}

void gap(){
    puts("\n按下任意键继续~~");
    getchar();
}
// 定义在终端出结果后的暂停函数


void bye(){
    save_user();
    save_record();
    puts("再见~~~");
    gap();
    exit(0);
}
// 定义退出程序的函数，退出程序的时候将用户，记录输入到文件中

void show_user( struct UserInfo * user ){
    printf("用户编号: %d\n" , user->no);
    printf("用户姓名: %s\n" , user->name);
    printf("用户家庭地址: %s\n" , user->addr);
    printf("用户联系电话: %lu\n\n" , user->number);
}
// 显示单个用户信息

void show_record( struct RecordInfo * record , char * name){
    printf( "用户姓名: %s\n" , name);
    printf( "%s金额: %lu\n" , record->type?"充值":"消费", record->money);
    printf( "%s日期: %d-%d-%d\n\n" ,record->type?"充值":"消费",
        record->time->year , 
        record->time->month , 
        record->time->day
    );
}
// 显示用户记录信息

void input( int fd, char *buffer ,unsigned int size, char target ){
    unsigned int i ;
    int res;
    char buf;

    for ( i = 0; i < size - 1 ; ++i ){
        res = read(fd, &buf, 1 );
        if ( res <= 0 )
            return;
        if ( buf == target )
            break;
        buffer[i] = buf;
    }
    buffer[i] = 0;
    // 为防止数据泄露，在数据末尾加上0来截断数据
    // 同时为防止出现 off by null 漏洞，把循环的跳出条件设置为 i < size - 1
}
// 封装一个 input 函数从标准输入来接受用户的数据，可以自定义截断字符

unsigned long int get_num(int fd){
    char buf[0x10];
    memset(buf , 0 , 0x10);
    input(fd, buf , 0x10 , '\n');
    return strtoul(buf , NULL , 10);
}
// 根据用户输入的字符返回一个对应的数字，用来处理用户输入的数字类型

struct Time * get_time(){
    time_t timep;
    struct Time *time_buf ;
    struct tm *p;

    time_buf = (struct Time *)calloc(sizeof(struct Time) , 1);
    p = (struct tm *)calloc(sizeof(struct tm) , 1);
    if(!time_buf || !p){
        puts("内存分配错误");
        exit(-1);
    }

    time(&timep);
    p=gmtime(&timep);
    time_buf->year = p->tm_year + 1900;
    time_buf->month = p->tm_mon + 1;
    time_buf->day = p->tm_mday ;

    // 简化 tm 结构体的数据并生成自己的 Time 结构体 

    return time_buf;
}
// 封装一个获取当前时间的函数

void load_user(){
    char buf[0x100];
    int i;
    int fd = open(FILE_NAME_USER , O_RDWR );
    struct UserInfo *user;
    unsigned long int user_count = 0 ;

    user_count = get_num(fd);
    // 从文件描述符中获取用户总数

    for ( i = 0;i< user_count ; ++i ){
        user = (struct UserInfo *) calloc(sizeof ( struct UserInfo) , 1);
        if(!user){
            puts("内存分配错误");
            return;
        }
        users->user[ users->counts++ ] = user;
        user->name = calloc(MAX_USER_NAME_SIZE , 1);
        user->addr = calloc(MAX_USER_ADDR_SIZE , 1);

        input( fd , buf , 0x10 , '\n' );
        user->no = get_num(fd);
        input( fd , user->name , MAX_USER_NAME_SIZE , '\n' );
        input( fd , user->addr , MAX_USER_ADDR_SIZE , '\n' );
        user->number = get_num(fd);
    }
    // 从文件描述符中获取数据
    close(fd);
    printf("加载用户数据成功~\n" );
}
// 读取用户数据文件

void load_record(){
    char buf[0x100];
    int i;
    int fd ;
    struct RecordInfo *record;
    unsigned long int record_count = 0 ;

    fd = open(FILE_NAME_RECORD , O_RDWR );
    record_count = get_num(fd);

    for ( i = 0;i< record_count ; ++i ){
        record = (struct RecordInfo *) calloc(sizeof ( struct RecordInfo) , 1);
        record->time = (struct Time *) calloc(sizeof(struct Time) , 1);
        if(!record || !record->time){
            puts("内存分配错误");
            return;
        }

        records->record[records->counts++] = record;
        
        input(fd , buf , 0x100 , '\n');
        
        record->no = (int) get_num(fd);
        record->user_no = (int) get_num(fd);
        record->money = get_num(fd);
        
        input(fd , buf ,0x10 , '-');
        record->time->year = atoi(buf);
        input(fd , buf ,0x10 , '-');
        record->time->month = atoi(buf);
        record->time->day = (int) get_num(fd);

        record->type = (int) get_num(fd);
    }
    close(fd);
    printf("加载记录数据成功~\n" );
}
// 读取记录数据文件，操作方式同上

void save_user(){
    int i;
    FILE * fd = fopen(FILE_NAME_USER ,"w+" );
    struct UserInfo *user;

    int user_count = users->counts;
    fprintf(fd , "%d\n\n" , user_count);
    for ( i = 0;i< user_count ; ++i ){
        user = users->user[i];
        fprintf(fd , "%d\n%s\n%s\n%lu\n\n" , 
            user->no ,
            user->name ,
            user->addr ,
            user->number
        );
    }
    fclose(fd);
    printf("用户数据以保存在: %s\n"  , FILE_NAME_USER);
}
// 将用户数组里的数据通过一定结构保存在文件中

void save_record(){
    int i;
    FILE * fd = fopen(FILE_NAME_RECORD ,"w+" );
    struct RecordInfo *record;

    int record_count = records->counts;
    fprintf(fd , "%d\n\n" , record_count);
    for ( i = 0;i< record_count ; ++i ){
        record = records->record[i];
        fprintf(fd , "%d\n%d\n%lu\n%d-%d-%d\n%d\n\n" , 
            record->no ,
            record->user_no ,
            record->money ,
            record->time->year,
            record->time->month,
            record->time->day,
            record->type
        );
    }
    fclose(fd);
    printf("记录数据以保存在: %s\n"  , FILE_NAME_RECORD);
}
// 将用户数组里的数据通过一定结构保存在文件中

struct UserInfo * get_user( unsigned long int no){
    unsigned long int i ;

    for (i = 0;i<MAX_USER_COUNTS;++i){
        if (users->user[i]){
            if ( (unsigned long int )users->user[i]->no == no)
                return users->user[i];
        }
    }
    return NULL;
}
// 通过用户编号获取用户结构体

struct Records * select_record_by_no( int user_no ){
    int i;
    struct Records *result ;
    
    result = (struct Records *)calloc(sizeof( struct Records ) , 1);
    if(!result){
        puts("内存分配错误");
        exit(-1);
    }

    for ( i = 0 ; i < records->counts ; ++i ){
        if( records->record[i] ){
            if(records->record[i]->user_no == user_no)
                result->record[ result->counts++ ] = records->record[i];
        }
    }
    return result;
}
// 通过用户编号查询记录,返回成一个记录数组

struct Records * select_record_by_year( int year ){
    struct Records *result;
    int i;

    result = (struct Records *)calloc(sizeof( struct Records ) , 1);
    if(!result){
        puts("内存分配错误");
        exit(-1);
    }

    for ( i = 0 ; i < records->counts ; ++i ){
        if( records->record[i] ){
            if(records->record[i]->time->year == year)
                result->record[ result->counts++ ] = records->record[i];
        }
    }
    return result;
}
// 通过年份查询记录,返回成一个记录数组

void add_user()
{
    struct UserInfo *user;

    if (users->counts >= MAX_USER_COUNTS){
        puts("用户已满!");
        return;
    }

    user = (struct UserInfo *) calloc(sizeof ( struct UserInfo) , 1);
    if(!user){
        puts("内存分配错误");
        return;
    }
    users->user[ users->counts++ ] = user;
    user->no = users->counts;
    user->name = calloc(MAX_USER_NAME_SIZE , 1);
    user->addr = calloc(MAX_USER_ADDR_SIZE , 1);

    printf("输入用户姓名: ");
    input( STDIN, user->name , MAX_USER_NAME_SIZE , '\n' );
    printf("输入用户家庭地址: ");
    input( STDIN, user->addr , MAX_USER_ADDR_SIZE , '\n' );
    printf("输入用户联系电话: ");
    user->number = get_num(STDIN);

    // 使用自定义输入函数进行用户数据的输入

    puts("操作成功!");
}
// 添加用户模块，使用 calloc 动态分配内存用于储存用户信息，将指针返回到储存用户指针的数组

void list_user()
{
    int i ;
    if( users->counts <=0 ){
        puts("当前没有用户哦~");
        return;
    }
    puts("\n所有用户:\n ");
    for (i = 0 ; i < users->counts ; ++i ){
        if( users->user[i] )
            show_user( users->user[i] );
    }
}
// 列出所有用户

void edit_user()
{
    unsigned long int user_no;
    struct UserInfo *user;

    printf("请输入用户编号: ");
    user_no = get_num(STDIN);
    user = get_user(user_no);
    if(!user){
        puts("用户不存在~");
        return;
    }
    puts("\n当前所选择的用户信息");
    show_user(user);

    printf("输入新的用户姓名: ");
    input( STDIN ,user->name , MAX_USER_NAME_SIZE , '\n' );
    printf("输入新的用户家庭地址: ");
    input( STDIN ,user->addr , MAX_USER_ADDR_SIZE , '\n' );
    printf("输入新的用户联系电话: ");
    user->number = get_num(STDIN);
    puts("操作成功!");
}

// 通过输入的用户编号查找并修改用户数据

void add_record_recharge()
{
    unsigned long int user_no;
    struct UserInfo *user;
    struct RecordInfo *record;

    if (records->counts >= MAX_RECORD_COUNTS){
        puts("记录已满!");
        return;
    }

    printf("请输入用户编号: ");
    user_no = get_num(STDIN);
    user = get_user(user_no);

    if(!user){
        puts("用户不存在~");
        return;
    }

    record = (struct RecordInfo *) calloc(sizeof(struct RecordInfo), 1);
    if(!record){
        puts("内存分配错误");
        return;
    }

    records-> record [records->counts++ ] = record;
    record -> user_no = user_no;
    record -> no = records->counts;
    record -> time = get_time();
    record -> type = RECHARGE ;
    printf("输入充值的金额: ");
    record -> money = get_num(STDIN);
}

// 充值模块，生成一个消息记录，记录编号自动增长

void add_record_consum(){
    int user_no;
    int i;
    struct Records *result ;
    struct UserInfo *user ;
    struct RecordInfo *record;
    struct Time *time_buf;

    if (records->counts >= MAX_RECORD_COUNTS){
        puts("记录已满!");
        return;
    }
    time_buf = get_time();

    printf("请输入用户编号: ");
    user_no = get_num(STDIN);
    user = get_user(user_no);
    if(!user){
        puts("用户不存在~");
        return;
    }

    result = select_record_by_no(user_no);

    // 返回该用户所有的记录

    if(result->counts >=0 ){
        for (i = 0 ; i< result->counts ; ++i){
            if( result->record[i]->type == CONSUM &
                result->record[i]->time->year == time_buf->year &
                result->record[i]->time->month == time_buf->month 
                ){
                puts("该用户本月已存在消费记录!");
                return;
            }
        }
    }

    // 通过记录类型，月份，年份来判断该月是否存在消费记录

    record = (struct RecordInfo *) calloc(sizeof(struct RecordInfo) , 1);
    if(!record){
        puts("内存分配错误");
        return;
    }
    records-> record [records->counts++ ] = record;
    record -> user_no = user_no;
    record -> no = records->counts;
    record -> time = get_time();
    record -> type = CONSUM ;
    printf("输入消费的金额: ");
    record -> money = get_num(STDIN);
    free(result);
}

// 消费模块，输入用户编号首先通过日期判断该月有没有消费记录，再进行操作生成记录

void show_user_record()
{
    struct Records *result ;
    struct UserInfo *user ;
    int user_no;
    int i;

    printf("请输入用户编号: ");
    user_no = get_num(STDIN);
    user = get_user(user_no);
    if(!user){
        puts("用户不存在~");
        return;
    }
    show_user(user);
    result = select_record_by_no(user_no);
    if( result->counts == 0 ){
        puts("该用户当前没有记录~");
    }
    for ( i =0; i<result->counts ;++i )
        show_record( result->record[i] , user->name );
    free(result);
}

// 显示用户的消费与充值记录

void show_form()
{
    struct Records *result;
    struct ReportForms *report;
    unsigned long long consum = 0;
    unsigned long long recharge = 0;
    int year;
    int i;

    printf("请输入年份: ");
    year = get_num(STDIN);

    report = (struct ReportForms*)calloc( sizeof( struct ReportForms ) ,1 );
    if(!report){
        puts("内存分配错误");
        return;
    }
    report->year = year;

    // 通过输入的年份来创建报表，首先将该报表的年份设置为用户输入的年份

    for ( i=0 ; i < 12 ; ++i ){
        report->month[i] = (struct Form*)calloc( sizeof(struct Form) , 1);
        report->month[i]->month = i +1;
        report->month[i]->recharge = 0;
        report->month[i]->consum = 0;
    }

    // 为报表数组每一个月来创建一个结构体存放消费与充值记录

    result = select_record_by_year(year);
    for ( i =0 ; i < result->counts ; ++i){
        struct RecordInfo * record = result->record[i];
        struct Form *row = report->month[record->time->month -1];
        if(record->type == CONSUM){
            row->consum += record->money;
        }else{
            row->recharge += record->money;
        }
    }

    // 通过检索出的业务记录通过判断其日期的记录月份来计算消费与充值情况

    printf("当前年份: %d\n" , year);
    printf("月份      消费      充值\n" );
    for( i=0 ;i < 12; ++i ){
        printf("%2d月%10lu%10lu\n" , 
            report->month[i]->month ,  
            report->month[i]->consum ,
            report->month[i]->recharge
        );
        consum += report->month[i]->consum;
        recharge += report->month[i]->recharge;
    }
    printf("总消费金额为: %llu\n总充值金额为: %llu\n" , consum  , recharge);
    free(result);
    free(report);
    // 释放堆内存
}

unsigned long int menu()
{
    printf("\n");
    puts(" *-------------------------*");
    puts(" | 菜单                    |");
    puts(" | 1. 新增用户             |");
    puts(" | 2. 修改用户             |");
    puts(" | 3. 显示全部客户信息     |");
    puts(" | 4. 添加消息记录         |");
    puts(" | 5. 增加充值记录         |");
    puts(" | 6. 查看个人消息记录     |");
    puts(" | 7. 显示月度收支账务报表 |");
    puts(" | 8. 退出                 |");
    puts(" *-------------------------*");
    printf("\n您的选择 >> ");
    return get_num(STDIN);
}
// 显示菜单并接受用户输入

void main()
{
    unsigned long int choice;

    init();

    while(1){
        choice = menu();
        switch(choice){
            case 1 : add_user(); gap();break;
            case 2 : edit_user(); gap();break;
            case 3 : list_user(); gap();break;
            case 4 : add_record_consum(); gap();break;
            case 5 : add_record_recharge(); gap();break;
            case 6 : show_user_record(); gap();break;
            case 7 : show_form(); gap();break;
            case 8 : bye();break;
            default: continue;
        }
    }
}
