/* ******************************************************************
  avl.c
 ******************************************************************** */

/**********************************************************************
 * ヘッダファイル
 **********************************************************************/
#include <stddef.h>                    /*-- NULL , size_t --*/
#include <stdio.h>                     /*-- printf関数    --*/
#include <stdlib.h>                    /*-- malloc関数    --*/

#define MAX_DATA_N 1000

/**********************************************************************
 * 左右の部分木のバランスを表す列挙 (enum) 型
 **********************************************************************/
typedef enum {
  tooleft = -2,
  left = -1,
  even = 0,
  right = 1,
  tooright = 2
} balance;

/**********************************************************************
 * 二分探索木のノードを表す自己参照構造体
 **********************************************************************/
typedef struct node {
   int student_id;                      /*-- 学籍番号               --*/
   int age;                             /*-- 年齢                   --*/
   int grade;                           /*-- 学年                 --*/
   struct node * parent;
   struct node * left;                  /*-- 左側の子供へのポインタ --*/
   struct node * right;                 /*-- 右側の子供へのポインタ --*/
   balance balanced;
} _node, * Node;



/**********************************************************************
 * 基本操作関数（プロトタイプ宣言）
 **********************************************************************/
void init( Node * rt );                /*-- 初期設定 --*/
void add( Node * rt, int student_id, int age, int grade);
Node search( Node root, int student_id );
int sol1( Node rt);
int sol2( Node rt);
int sol3( Node rt);
int sol4( Node rt);
int sol5( Node rt);

/**********************************************************************
 * その他の関数（プロトタイプ宣言）
 **********************************************************************/
void simple_display( Node p, int level);
void display( Node p, int level);      /* 内容表示 */
void freeAll( Node * rt );             /* メモリ解放(main) */ 
void freeTree( Node p );               /* メモリ解放(sub)  */

/**********************************************************************
 * main関数
 **********************************************************************/
int main(void){

   int ids[MAX_DATA_N];
   int ages[MAX_DATA_N];
   int grades[MAX_DATA_N];
   int data_n = 0;
   
   FILE *fp;
   char fname[] = "avl_data.csv";
   fp = fopen(fname, "r");
   if(fp == NULL) {
		printf("%s file not open!\n", fname);
		return -1;
	} else {
		printf("%s file opened!\n", fname);
	}

	for(data_n = 0 ;fscanf(fp, "%d,%d,%d", ids+data_n, ages+data_n, grades+data_n) != EOF
          && data_n < MAX_DATA_N ; data_n++){
      printf ("%d %d %d\n", ids[data_n], ages[data_n], grades[data_n]);
   }
 
	fclose(fp); // ファイルを閉じる   
   
   Node root;          /* 二分探索木（の根）を指すポインタ変数 */
   
   /*-★ 二分探索木の初期設定 --*/
   init( &root );
   
   /*-★ 二分探索木の構築 --*/
   for(int i = 0; i < data_n; ++i)
      add(&root, ids[i], ages[i], grades[i]);
   
   simple_display (root, 0);
   
   // 問1 木の根のstudent_idを出力する関数sol1を書け
   int ans1 = sol1(root);
   printf("ans1=%d\n", ans1);
   
   // 問2 木の高さを出力する関数sol2を書け
   int ans2 = sol2(root);
   printf("ans2=%d\n", ans2);
   
   // 問3 木の葉の数を出力する関数sol3を書け
   int ans3 = sol3(root);
   printf("ans3=%d\n", ans3);
   
   // 問4 ageが23の頂点のstudent_idを出力する関数sol4を書け
   int ans4 = sol4(root);
   printf("ans4=%d\n", ans4);
   
   // 問5 ３年生である頂点の個数を出力する関数sol5を書け
   int ans5 = sol5(root);
   printf("ans5=%d\n", ans5);
   
   /*-★ メモリ領域を解放する --*/
   freeAll(&root);
   
   return 0;
}

/**********************************************************************
 * 基本操作関数（関数本体の定義）
 **********************************************************************/
/*-★ 初期設定関数 --*/
void init( Node * rt ){
   *rt = NULL;
}
/*-★ メモリ解放関数 --*/
void freeAll( Node * rt ){
   freeTree(*rt);
   *rt = NULL;
}

void freeTree( Node p ){
   if( p != NULL ){
      freeTree(p->left);            /* 左部分木をトラバースする */
      freeTree(p->right);           /* 右部分木をトラバースする */
      free(p);                      /* メモリ領域を解放する     */
   }
}

void simple_display( Node p, int level){
   int i;
   if( p != NULL ){
      if( level == 0 ){
         printf("  %d\n", p->student_id);
      } else {
         for(i=0;i<level;i++){ printf("   "); }
         printf("┗ %d\n", p->student_id);
      }
      simple_display(p->left,level+1);            /* 左部分木をトラバースする */
      simple_display(p->right,level+1);           /* 右部分木をトラバースする */
   }
}

// ★ 内容表示関数（レベルに応じて字下げする） 
void display( Node p, int level){
   int i;
   if( p != NULL ){
      if( level == 0 ){
         printf("%p:[%d;%p,%p](%d)\n", 
                p, p->student_id, p->left, p->right, p->balanced);
      } else {
         for(i=0;i<level;i++){ printf("   "); }
         printf("┗ %p:[%d;%p,%p](%d)\n", 
                p, p->student_id, p->left, p->right, p->balanced);
      }
      display(p->left,level+1);            /* 左部分木をトラバースする */
      display(p->right,level+1);           /* 右部分木をトラバースする */
   }
}

/**********************************************************************
 * ここ以降に解答が追加されます
 **********************************************************************/


void _set_node (Node * rt, int student_id, int age, int grade, Node parent){
   Node new_node = malloc(sizeof(_node));
   new_node->student_id = student_id;
   new_node->age = age;
   new_node->grade = grade;
   new_node->left  = NULL;
   new_node->right = NULL;
   new_node->parent = parent;
   new_node->balanced = even;
   *rt = new_node;
   return;
}

void _single_rotation (Node * rt, Node current, balance balanced) {
   Node p, q;
   q = NULL;

   printf("=== 一重回転 ===\n"); 

   p = current->parent;
   if (balanced == tooleft) {
      q = current->left;
      current->left = q->right; // q の右部分木を current の左部分木に
      if (q->right) q->right->parent = current; // current を根とする部分木を q の右部分木に 
      q->right = current;
   } else if (balanced == tooright) {
      q = current->right; // q の左部分木を current の右部分木に
      current->right = q->left; 
      if (q->left) q->left->parent = current; // current を根とする部分木を q の左部分木に
      q->left = current; 
   }
   q->parent = p; 
   current->parent = q;
   
   // q を p (current->parent) の子に 
   if (p) 
      if (p->left == current)
         p->left = q;  // current が親の左の子
      else
         p->right = q; // current が親の右の子 
   else
      *rt = q; // current が根の場合 

   // balanced の更新
   q->balanced = even;
   current->balanced = even;
   return;
}

void _double_rotation (Node * rt, Node current, balance balanced) {
   Node p, q, new_root;
   int old_balanced;
   printf("=== 二重回転 ===\n"); 
   p = current->parent;

   /* tooleft */
   if (balanced == tooleft) {
      q = current->left;
      new_root = q->right; 
      old_balanced = q->right->balanced;
      /* new_root の左部分木を q の右部分木に */
      q->right = new_root->left; 
      if (q->right) q->right->parent = q; 
      /* new_root の右部分木を current の左部分木に */
      current->left = new_root->right; 
      if (current->left) current->left->parent = current; 
      /* new_root を今見ている部分木の根に */
      new_root->parent = p;
      if (new_root->parent) // current が根ではない場合
         if (new_root->parent->left == current)
            new_root->parent->left = new_root;  // current が親の左の子
         else
            new_root->parent->right = new_root; // current が親の右の子
      else
         *rt = new_root; // current が根の場合
      new_root->left = q; 
      q->parent = new_root;
      new_root->right = current; 
      current->parent = new_root;
      /* balanced の更新 */
      new_root->balanced = even;
      q->balanced = even;
      if (current->right) {
         current->balanced = right;
         if (old_balanced == left)
            new_root->right->balanced = left;
         else
            new_root->left->balanced = left;
      } else
         current->balanced = even;
   }

   /* tooright */
   else if (balanced == tooright) {
      q = current->right;
      new_root = q->left; 
      old_balanced = q->left->balanced;
      /* new_root の右部分木を q の左部分木に */
      q->left = new_root->right; 
      if (q->left) q->left->parent = q; 
      /* new_root の左部分木を current の右部分木に */
      current->right = new_root->left; 
      if (current->right) current->right->parent = current; 
      /* new_root を今見ている部分木の根に */
      new_root->parent = p;
      if (new_root->parent)
         if (new_root->parent->left == current)
            new_root->parent->left = new_root;  // current が親の左の子
         else
            new_root->parent->right = new_root; // current が親の右の子
      else
         *rt = new_root; // current が根の場合
      new_root->right = q; 
      q->parent = new_root;
      new_root->left = current; 
      current->parent = new_root;
      /* balanced の更新 */
      new_root->balanced = even;
      q->balanced = even;
      if (current->left) {
         current->balanced = left;
         if (old_balanced == right)
            new_root->left->balanced = right;
         else
            new_root->right->balanced = right;
      } else
         current->balanced = even;
      
   }
   return;
}

void balance_repaire (Node * rt, Node current) {
   /* 先祖の balanced の更新（必要に応じて回転） */
   for (; (current->parent && current->balanced != even) || 
           (!current->parent &&
            (current->balanced == tooleft || current->balanced == tooright)) ; 
        current = current->parent) {

      /* 回転不要 => current->parent のバランスの更新 */
      if (current->balanced == left || current->balanced == right) {
         current->parent->balanced += (current->parent->right == current ? 1 : -1);
      }

      /* 回転必要 - tooleft */      
      else if (current->balanced == tooleft) {

         printf("【tooleft】 回転前の木の内容\n");
          printf("root=%p\n", *rt);
         display(*rt,0);
         
         if (current->left->balanced == left) {
            _single_rotation (rt, current, tooleft);
         } else if (current->left->balanced == right) {
            _double_rotation (rt, current, tooleft);
         } else {
            printf ("エラー\n");
            exit (1); /* 異常終了 */
         }
         break;
      }

      /* 回転必要 - tooright */      
      else if (current->balanced == tooright) {

         printf("【tooright】 回転前の木の内容\n");
         printf("root=%p\n", *rt);
         display(*rt,0);

         if (current->right->balanced == right) {
            _single_rotation ( rt, current, tooright);
         } else if (current->right->balanced == left) {
            _double_rotation (rt, current, tooright);
         } else {
            printf ("エラー\n");
            exit (1); /* 異常終了 */
         }
         break;
      } 
   }
}

// ★ 追加関数（データ要素を追加する） 
void add( Node * rt, int student_id, int age, int grade){
   Node current, parent;
   // (A) 二分探索木が空だったとき
   if( *rt == NULL ){
      _set_node (rt, student_id, age, grade, NULL);
      return;
   }
   
   // (B) 二分探索木が空ではないとき
   current = *rt;
   while( current != NULL ){
      if( student_id == current->student_id ) return;
      parent = current;         
      current = (student_id < current->student_id ?
                 current->left : current->right);
   }
   if (student_id < parent->student_id) {
      _set_node (&(parent->left), student_id, age, grade, parent);
      parent->balanced--;
   } else {
      _set_node (&(parent->right), student_id, age, grade, parent);
      parent->balanced++;         
   }

   balance_repaire (rt, parent);
   return;
}


// ★ 検索関数（データ要素を検索する） 
Node search( Node root, int student_id ){
   Node current;
   
   current = root;
   while( current != NULL ){
      if( student_id == current->student_id ) return current;
      current = student_id < current->student_id ? current->left : current->right;
   }
   
   return NULL;
}



// 問1 木の根のstudent_idを出力する関数sol1を書け
int sol1(Node rt){
   return rt->student_id;
}

// 問2 木の高さを出力する関数sol2を書け   
int sol2(Node rt){
   int rhight, lhight, hight;
   rhight = (rt->right == NULL ? -1 : sol2(rt->right));
   lhight = (rt->left == NULL ? -1 : sol2(rt->left));
   hight = (rhight > lhight ? rhight : lhight);
   return hight + 1;
}

// 問3 木の葉の数を出力する関数sol3を書け
int sol3(Node rt){
   int rnleaf, lnleaf;
   if (rt->right == NULL && rt->left == NULL) return 1;
   rnleaf = (rt->right != NULL) ? sol3(rt->right) : 0;
   lnleaf = (rt->left != NULL) ? sol3(rt->left) : 0;
   return rnleaf + lnleaf;
}

// 問4 ageが23の頂点のstudent_idを出力する関数sol4を書け
int sol4(Node rt){
   int id=0, target=23;
   if (rt->age == target) return rt->student_id;
   if (rt->right != NULL && (id = sol4(rt->right))) return id;
   if (rt->left != NULL && (id = sol4(rt->left))) return id;   
   return 0;
}

// 問5 ３年生である頂点の個数を出力する関数sol5を書け
int sol5(Node rt){
   int rn3, ln3;
   rn3 = (rt->right != NULL) ? sol5(rt->right) : 0;
   ln3 = (rt->left != NULL) ? sol5(rt->left) : 0;
   return (rt->grade == 3) ? rn3 + ln3 + 1 : rn3 + ln3;
}
