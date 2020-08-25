/* ***************************
   データ構造
   迷路の例題
   *************************** */

#include<stdio.h>
#include<stdlib.h>

#define MAP_SIZE_MAX 20

// セル表現用 構造体
typedef struct {
  int row;               // 行番号
  int col;               // 列番号
} Coord;

typedef struct _que {
  struct _que *next;
  struct _que *prev;
  Coord point;
} *que, queelem;

typedef struct {
  que head;
  que tail;
} *queue, queueelem;

// スタック用 構造体
typedef struct _stack {
  struct _stack *next;
  Coord point;
} *stack, stackelem;

char M[MAP_SIZE_MAX+2][MAP_SIZE_MAX+2];    // 盤面（グローバ

// プロトタイプ宣言
int solve (FILE *fp, char solution);
int search_A (int W, int H, Coord start);
int search_B (int W, int H, Coord start);
void display_map (int W, int H);

// キュー用
queue new_queue (void);
int is_empty_queue (queue);
int enqueue (queue, Coord);
Coord dequeue (queue);

// スタック用
stack new_stack (void);
int is_empty_stack (stack);
int push (stack, Coord);
Coord top (stack);
Coord pop (stack);

// main 関数
int main(){
  char maze_type;
  char fname[1000];
  printf("maze_type: ");
  scanf("%c", &maze_type);

  printf("filename: ");
  scanf("%s", fname);

  FILE *fp;
  fp = fopen(fname, "r");
  if(fp == NULL) {
    printf("%s file not open!\n", fname);
    return -1;
  }
  
  solve(fp, maze_type);
}

int solve (FILE *fp, char maze_type) {
  int W, H;
  Coord start;             // スタート地点

  // ファイルからデータ構の読み込み
  fscanf(fp, "%d%d", &W, &H);
  //  壁（番兵）の設定 
  for (int i=0; i<=W+1; i++) { M[0][i] = '#'; M[H+1][i] = '#'; }
  for (int i=1; i<=H; i++) { M[i][0] = '#'; M[i][W+1] = '#'; }
  // データの入力 
  for (int i=1; i<=H; i++) {
    for (int j=1; j<=W; j++) {
      fscanf(fp, " %c", &M[i][j]);
      if (M[i][j]=='S') {
	start.row = i;
	start.col = j;
      }
    }
  }
  fclose(fp); // ファイルを閉じる   

  if(maze_type=='a'){
    search_A(W, H, start);
  }else if(maze_type=='b'){
    search_B(W, H, start);
  }

  return 0;
}

int search_A (int W, int H, Coord start) {
  queue Q = new_queue ();  // 訪問セルを保持するキュー
  int previous_enemy = 0;
  // スタート地点の設定
  enqueue(Q, start);                           // スタート地点を enqueue
  // 探索
  while (!is_empty_queue(Q)) {
    Coord current = dequeue(Q);                // dequeue されたセル
    /* if ('S'!=M[current.row][current.col]) */
    /*   M[current.row][current.col] = '*';         // current を到 */
    int d, enemy;                              // 方向、敵
    int D1[]={1,0,-1,0}; int D2[]={0,-1,0,1};  // 方向転換制御用
    printf("%d %d\n", current.col, current.row);  // 本来は必要ない
    /* display_map(W, H); // 現在の情況を表示 (本来は必要ない) */
    for (d=0; d<4; d++) {
      Coord nc = current;                      // Next Cell
      // 自分で作成しよう
      nc.row += D1[d]; nc.col += D2[d];        // current から d の方向に移動
      char visit = M[nc.row][nc.col];
      char visit_str[] = {visit, '\0'};
      enemy = atoi(visit_str);
      if (visit == '.') {          // nc が未到達点 '.' の場合の処理
	enqueue(Q, nc);                           // nc をキューに enqueue
        M[nc.row][nc.col] = '*';         // current を到
      } else if (0 != enemy){
        if (previous_enemy+1 < enemy){
          return 0; // 勇者は死ぬ
        } else {
          previous_enemy = enemy;
          enqueue(Q, nc);
          M[nc.row][nc.col] = '*';         // current を到
        } // nc が、探索済 (*) の場合と壁 (#) の場合は何もしない
      }
    }
  }
  return 1;  // 無事すべての敵を倒せた
}

int search_B (int W, int H, Coord start) {
  stack S = new_stack ();  // 訪問セルを保持するスタック
  int previous_enemy = 0;
  // スタート地点の設定
  push(S,start);                               // スタート地点を push
  // 探索
  while (!is_empty_stack(S)) {
    Coord current = pop(S);                    // pop されたセル
    int d, enemy;                                     // 方向、敵
    int D1[]={0, 1}; int D2[]={1, 0};  // 方向転換制御用
    printf("%d %d\n", current.col, current.row);  // 本来は必要ない
    /* display_map(W, H); // 現在の情況を表示 (本来は必要ない) */
    for (d=0; d<2; d++) {
      Coord nc = current;                      // Next Cell
      // 自分で作成しよう
      nc.row += D1[d]; nc.col += D2[d];        // current から d の方向に移動
      char visit = M[nc.row][nc.col];
      char visit_str[] = {visit, '\0'};
      enemy = atoi(visit_str);
      if (visit == '.') {          // nc が未到達点 '.' の場合の処理
	push(S, nc);                              // nc をスタックに push
      } else if (0 != enemy){
        if (previous_enemy+1 < enemy){
          return 0; // 勇者は死ぬ
        } else {
          previous_enemy = enemy;
          push(S, nc);
        }
      } // nc が、探索済 (*) の場合と壁 (#) の場合は何もしない   
    }
  }
  return 1;  // 無事すべての敵を倒せた
}

void display_map (int W, int H) {;
  for (int i=1; i<=H; i++) {
    for (int j=1; j<=W; j++) {
      printf("%c", M[i][j]);
    }
    putchar('\n');
  }
  putchar('\n');
}

/* 新しいキューを確保（＋初期化） */
queue new_queue (void) {
  queue q;
  q = (queue) malloc (sizeof (queueelem));
  if (q == NULL) return NULL;
  q->head = NULL;
  q->tail = NULL;
  return q;
}
    
/* キューが空かどうか検査 */
int is_empty_queue (queue q) {
  return (q && q->head == NULL);
}

/* 新しい要素をキューに追加 */
int enqueue (queue q, Coord v) {
  que p;
  if (q == NULL) return 0;
  p = (que) malloc (sizeof(queelem));
  if (p == NULL) return 0;
  p->point = v;
  p->next = NULL;
  if (q->tail) {
    p->prev = q->tail;
    q->tail->next = p;
  } else {
    /* queue が空 */
    p->prev = NULL;
    q->head = p;
  }
  q->tail=p;
  return 1;
}

/* キューの先頭要素を取り出し */
Coord dequeue (queue q) {
  que p;
  Coord v = q->head->point;
  p = q->head;
  if (p->next == NULL) {
    q->head = NULL;
    q->tail = NULL;
  } else {
    q->head = q->head->next;
    q->head->prev = NULL;
  }
  free (p);
  return v;
}

/* 新しいスタックを確保（＋初期化） */
stack new_stack (void) {
  stack s;
  s = (stack) malloc (sizeof (stackelem));
  if (s == NULL) return NULL;
  s->next = NULL;
  return s;
}
    
/* スタックが空かどうか検査 */
int is_empty_stack (stack s) {
  return (s && s->next == NULL);
}

/* 新しい要素をプッシュ */
int push (stack s, Coord v) {
  stack p;
  if (s == NULL) return 0;
  p = (stack) malloc (sizeof (stackelem));
  if (p == NULL) return 0;
  p->point = v;
  p->next = s->next;
  s->next =  p;
  return 1;
}

/* 先頭の要素を返す */
Coord top (stack s) {
  if (s && s->next) 
    return s->next->point;
  exit (1);
}

/* ポップ */
Coord pop (stack s) {
  stack p;
  Coord v = top (s);
  p = s->next;
  s->next = p->next;
  free (p);
  return v;
}
