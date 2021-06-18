  
/* ******************************************************************
  hash.c
 ******************************************************************** */

/**********************************************************************
 * ヘッダファイル
 **********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_DATA_N 80   //ファイルから読み込むデータ数の最大値
#define KEYL 7          //キー（人名の短縮形）の長さの最大値
#define DATAL 20        //データ（人名）の長さの最大値
#define MAX 107         //ハッシュテーブルのサイズ
#define K 3             //ハッシュ関数内の定数
#define USED 1          //使用中
#define NIL -1          //空
#define DEL -2          //削除済み

/**********************************************************************
 * 内部ハッシュ法を用いたハッシュテーブルのスロットを表す構造体
 **********************************************************************/
typedef struct openAdd_cell{
    int flag;
    char nickname[KEYL];
    char name[DATAL];
    char skil[DATAL];
} cell_oa_t;

/**********************************************************************
 * 外部ハッシュ法を用いたハッシュテーブルのスロットを表す自己参照構造体
 **********************************************************************/
typedef struct chain_cell {
    char nickname[KEYL];
    char name[DATAL];
    char skil[DATAL];
    struct chain_cell *next;
} cell_ch_t;

/**********************************************************************
 * ハッシュ関数（プロトタイプ宣言）
 **********************************************************************/
int hash(char p[]);
int double_hash(char *key, int h0, int i);
int key_code_sum(char *s);

/**********************************************************************
 * hash_A関数（内部ハッシュ法での実装ケース）に用いる関数（プロトタイプ宣言）
 **********************************************************************/
int hash_A(char nicknames[][KEYL], char names[][DATAL], char skils[][DATAL], int data_n);
void action_a(char *key, cell_oa_t **hashtbl);
void init_table_a(cell_oa_t **hashtbl);
int insert_table_a(cell_oa_t *r, cell_oa_t **hashtbl);
cell_oa_t * search_table_a(char *key, cell_oa_t **hashtbl);
int delete_data_a(cell_oa_t *p, cell_oa_t **hashtbl);
void print_table_a(cell_oa_t **hashtbl);
void hash_free_a(cell_oa_t **hashtbl);

/**********************************************************************
 * hash_B関数（外部ハッシュ法での実装ケース）に用いる関数（プロトタイプ宣言）
 **********************************************************************/
int hash_B(char nicknames[][KEYL], char names[][DATAL], char skils[][DATAL], int data_n);
void action_b(char *key, cell_ch_t **hashtbl);
void init_table_b(cell_ch_t **hashtbl);
int insert_table_b(cell_ch_t *r, cell_ch_t **hashtbl);
cell_ch_t * search_table_b(char *key, cell_ch_t **hashtbl);
int delete_data_b(char *key, cell_ch_t **hashtbl);
void print_table_b(cell_ch_t **hashtbl);
void hash_free_b(cell_ch_t **hashtbl);
void cell_free(cell_ch_t *cell);


/**********************************************************************
 * main関数
 **********************************************************************/
int main() {
    char nicknames[MAX_DATA_N][KEYL];
    char names[MAX_DATA_N][DATAL];
    char skils[MAX_DATA_N][DATAL];
    int data_n = 0;
    
    char hash_f_type;
    char fname[1000];
    printf("hash_f_type (a or b): ");
    scanf("%c", &hash_f_type);
    
    printf("filename: ");
    scanf("%s", fname);
    
    FILE *fp;
    fp = fopen(fname, "r");
    if(fp == NULL) {
        printf("%s file not open!\n", fname);
        return -1;
    } else {
        printf("%s file opened!\n", fname);
    }
    
    for(data_n = 0; fscanf(fp, " %[^,],%[^,],%s", nicknames[data_n], names[data_n], skils[data_n]) != EOF
            && data_n < MAX_DATA_N ; data_n++){
        //printf("%s %s\n", nicknames[data_n], names[data_n]);
    }
    fclose(fp); // ファイルを閉じる
    
    if(hash_f_type=='a'){
        hash_A(nicknames, names, skils, data_n);
    }else if(hash_f_type=='b'){
        hash_B(nicknames, names, skils, data_n);
    }
   
    return 0;
}


/**********************************************************************
 * ハッシュ関数
 **********************************************************************/
int hash(char p[]) {
    int i;
    int h = 0;
    for(i = 0; p[i] != '\0'; i++)
        h = (h * K + p[i]) % MAX;
    return h;
}

int double_hash(char *key, int h0, int i) {
    int c = 59;
    int k = key_code_sum(key);
    int h_x = h0;
    int h_s = (c - (k%c))%MAX;
    int h_i = (h_x + i * h_s)%MAX;
    return h_i;
}

/*-★ キーのアスキーコードの合計%MAX を返す関数 --*/
int key_code_sum(char * s) {
    int i = 0;
    while(* s) {
        i += * s++;
    }
    return i % MAX;
}


/**********************************************************************
 * hash_A関数に用いる関数（空欄箇所を実装）
 **********************************************************************/
int hash_A(char nicknames[][KEYL], char names[][DATAL], char skils[][DATAL], int data_n) {
    cell_oa_t *hashtbl[MAX];

    /*-★ ハッシュテーブルの初期設定 --*/
    init_table_a(hashtbl);

    for(int i = 0; i < data_n; i++) {
        cell_oa_t *c = malloc(sizeof(cell_oa_t));
        if(c == NULL) exit(1);
        c->flag = USED;
        strcpy(c->nickname, nicknames[i]);
        strcpy(c->name, names[i]);
        strcpy(c->skil, skils[i]);
        if(!insert_table_a(c, hashtbl)) {
            printf ("hash_table overflow\n");
            exit(0);
        }
    }
    print_table_a(hashtbl);

    // それぞれの人物の能力を使用
    action_a("Kate", hashtbl);
    action_a("Jeff", hashtbl);
    action_a("Lucy", hashtbl);
    action_a("Gil", hashtbl);
    action_a("Sean", hashtbl);

    /*-★ メモリ領域を解放する --*/
    hash_free_a(hashtbl);
    return 0;
}

/*-★ 能力発動関数 --*/
void action_a(char *key, cell_oa_t **hashtbl) {
    cell_oa_t * p = search_table_a(key, hashtbl);
    printf("search_name (%s): ", key);
    if(p) {
        printf("%s\n", p->name);
        printf("勇者「%s!!」 %s 発動!!\n", p->name, p->skil);
        delete_data_a(p, hashtbl);
    } else {
        printf("NULL\nMISS!!\n");
    }
}

/*-★ 初期設定関数 --*/
void init_table_a(cell_oa_t **hashtbl){
    for(int cnt = 0; cnt < MAX; cnt++) {
        cell_oa_t *p = malloc(sizeof(cell_oa_t));
        p->flag = NIL; strcpy(p->nickname, ""); strcpy(p->name, ""); strcpy(p->skil, "");
        hashtbl[cnt] = p;
    }
}

/*-★ データ挿入関数 --*/
int insert_table_a(cell_oa_t *r, cell_oa_t **hashtbl) {
    /* 成功 1; 失敗 0; を返すようにする */
    int i = 1, h0, h;  //h0:最初のハッシュ値
    h = h0 = hash(r->nickname);
    while( i < MAX ) {
        if(hashtbl[h]->flag == NIL || hashtbl[h]->flag == DEL) {
            hashtbl[h] = r;
            return 1;
        } else if(strcmp(hashtbl[h]->nickname, r->nickname) == 0) {
            printf("double registration! :%s\n", r->nickname);
            return -1;
        } else {
            h = double_hash(r->nickname, h0, i);
            i++;
        }
    }
    return 0;
}

/*-★ データ探索関数 --*/
cell_oa_t * search_table_a(char *key, cell_oa_t **hashtbl){
    /* 成功 p; データなし NULL を返すようにする */
    cell_oa_t *p;
    int h, h0;
    int i = 1;
    h0 = hash(key);
    p = hashtbl[h0];
    while(p->flag != NIL && i < MAX){
        if(strcmp(key, p->nickname) == 0)
            return p;
        else {
            h = double_hash(key, h0, i);
            p = hashtbl[h];
            ++i;
        }
    }
    return NULL;
}

/*-★ データ削除関数 --*/
int delete_data_a(cell_oa_t *p, cell_oa_t **hashtbl){
    /* ノード自体を渡すため、今回は探索の手間がかからない */
    p->flag = DEL;
    strcpy(p->nickname, "");
    strcpy(p->name, "");
    strcpy(p->skil, "");
    return 0;
}

/*-★ 内容表示関数 --*/
void print_table_a(cell_oa_t **hashtbl) {
    for(int cnt = 0; cnt < MAX; cnt++){
        if(hashtbl[cnt]->flag != NIL){
            printf("table[%d]: {%s:%s:%s}\n", 
                cnt, hashtbl[cnt]->nickname, hashtbl[cnt]->name, hashtbl[cnt]->skil);
        }
    }
}

/*-★ メモリ解放関数 --*/
void hash_free_a(cell_oa_t **hashtbl) {
    cell_oa_t *temp = NULL;
 
    for(int cnt = 0; cnt < MAX; cnt++){
        temp = hashtbl[cnt];
        /* リストの解放 */
        if(temp != NULL){
            free(temp);
        }
    }
    init_table_a(hashtbl);
    return;
}


/**********************************************************************
 * hash_B関数に用いる関数（空欄箇所を実装）
 **********************************************************************/
int hash_B(char nicknames[][KEYL], char names[][DATAL], char skils[][DATAL], int data_n) {
    cell_ch_t *hashtbl[MAX];

    /*-★ ハッシュテーブルの初期設定 --*/
    init_table_b(hashtbl);

    for(int i = 0; i < data_n; i++) {
        cell_ch_t *c = malloc(sizeof(cell_ch_t));
        if(c == NULL) exit(1);
        strcpy(c->nickname, nicknames[i]);
        strcpy(c->name, names[i]);
        strcpy(c->skil, skils[i]);
        insert_table_b(c, hashtbl);
    }
    print_table_b(hashtbl);

    // それぞれの人物の能力を使用
    action_b("Kate", hashtbl);
    action_b("Jeff", hashtbl);
    action_b("Rudy", hashtbl);
    action_b("Gil", hashtbl);
    action_b("Larry", hashtbl);

    /*-★ メモリ領域を解放する --*/
    hash_free_b(hashtbl);
    return 0;
}

/*-★ 能力発動関数 --*/
void action_b(char *key, cell_ch_t **hashtbl) {
    cell_ch_t * p = search_table_b(key, hashtbl);
    printf("search_name (%s): ", key);
    if(p) {
        printf("%s\n", p->name);
        printf("勇者「%s!!」 %s 発動!!\n", p->name, p->skil);
        delete_data_b(p->nickname, hashtbl);
    } else {
        printf("NULL\nMISS!!\n");
    }
}

/*-★ 初期設定関数 --*/
void init_table_b(cell_ch_t **hashtbl) {
    /* ハッシュテーブルをNULLで初期化する */
    for(int cnt = 0; cnt < MAX; cnt++){
        hashtbl[cnt] = NULL;
    }
    return;
}

/*-★ データ挿入関数 --*/
int insert_table_b(cell_ch_t *r, cell_ch_t **hashtbl){
    cell_ch_t *p;
    int h;
    h = hash(r->nickname);
    if(hashtbl[h] == NULL) {
        hashtbl[h] = r;
        r->next = NULL;
        return h;
    } else {
        p = hashtbl[h];
        if(strcmp(r->nickname, p->nickname) == 0) {
            printf("double registration! :%s\n", r->nickname);
            return -1;
        }
        hashtbl[h] = r;
        r->next = p;
        return h;
    }
}

/*-★ データ探索関数 --*/
cell_ch_t * search_table_b(char *key, cell_ch_t **hashtbl){
    cell_ch_t *p;
    int h;
    h = hash(key);
    p = hashtbl[h];
    while(p != NULL) {
        if(strcmp(key, p->nickname) == 0)
            return p;
        else
            p = p->next;
    }
    return NULL;
}

/*-★ データ削除関数 --*/
int delete_data_b(char *key, cell_ch_t **hashtbl){
    cell_ch_t *target = NULL;
    cell_ch_t *chain  = NULL;
    int hashval = hash(key);
  
    /* キーがハッシュテーブルに存在しているか確認する
     * 基本的に存在するデータを削除する流れなので、今回は不要だが入れている。 
     */
    target = hashtbl[hashval];
    if(target == NULL){
        fprintf(stderr, "target[%s] is not exist in hash table.\n", key);
        return -1;
    }
    chain = target->next;
  
    /* リストの先頭要素を削除する場合 */
    if(strcmp(key, target->nickname) == 0){
        hashtbl[hashval] = chain;
        free(target);
        return 0;
    }
  
    /* 先頭以外の要素を削除する場合 */
    while(target != NULL){
        if(strcmp(key, target->nickname) == 0){
            chain->next = target->next;
            free(target);
            return 0;
        }
        chain  = target;
        target = target->next;
    }
  
    return -1;
}


/*-★ 内容表示関数 --*/
void print_table_b(cell_ch_t **hashtbl) {
    cell_ch_t *chain = NULL;
 
    for(int cnt = 0; cnt < MAX; cnt++){
        if(hashtbl[cnt] != NULL){
            printf("table[%d]:", cnt);
            for(chain = hashtbl[cnt]; chain != NULL; chain = chain->next){
                printf("->{%s:", (chain->nickname));
                printf("%s:", (chain->name));
                printf("%s}", (chain->skil));
            }
            putchar('\n');
        }
    }
}

/*-★ メモリ解放関数 --*/
void hash_free_b(cell_ch_t **hashtbl) {
    cell_ch_t *temp = NULL;
    cell_ch_t *swap = NULL;
 
    for(int cnt = 0; cnt < MAX; cnt++){
        temp = hashtbl[cnt];
        /* リストの解放 */
        while(temp != NULL){
            swap = temp->next;
            free(temp);
            temp = swap;
        }
    }
    init_table_b(hashtbl);
    return;
}
