/*
由于能力和时间有限，未能来得及编写自己的malloc，只是对伙伴分配器实现程序进行了阅读和分析，了解了其算法。

buddy是一种特殊的“分离适配”，即将内存按2的幂进行划分，相当于分离出若干个块大小一致的空闲链表，搜索该链表并给出同需求最佳匹配的大小。
buddy其优点是快速搜索合并以及低外部碎片。其缺点是高内部碎片。

分配内存时，要寻找大小合适的内存块，会根据需要分配内存的空间大小进行寻找，得出合适的内存块。
释放内存时，需要查看其相邻的内存块，如果该块已经被释放了则就对块进行合并。
*/

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

struct buddy {
  unsigned size;	//管理内存的总单元数目
  unsigned longest[1];	//内存块的空闲单位
};

struct buddy* buddy_list( int size );	//空闲块列表

#define LEFT_LEAF(index) ((index) * 2 + 1)
#define RIGHT_LEAF(index) ((index) * 2 + 2)
#define PARENT(index) ( ((index) + 1) / 2 - 1)

#define IS_POWER_OF_2(x) (!((x)&((x)-1)))	//判断是否为2的幂
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define ALLOC malloc
#define FREE free

//根据需要分配内存大小给出相应的2的幂
static unsigned fixsize(unsigned size) {
  size |= size >> 1;
  size |= size >> 2;
  size |= size >> 4;
  size |= size >> 8;
  size |= size >> 16;
  return size+1;
}

//创建新的内存
struct buddy* buddy_new( int size ) {
  struct buddy* self;
  unsigned node_size;
  int i;

  if (size < 1 || !IS_POWER_OF_2(size))
    return NULL;

  self = (struct buddy*)ALLOC( 2 * size * sizeof(unsigned));
  self->size = size;
  node_size = size * 2;

  for (i = 0; i < 2 * size - 1; ++i) {
    if (IS_POWER_OF_2(i+1))
      node_size /= 2;
    self->longest[i] = node_size;
  }
  return self;
}

//把内存全部清空
void buddy_destroy( struct buddy* self) {
  FREE(self);
}

//内存分配接口
int buddy_alloc(struct buddy* self, int size) {//分配大小为size的空闲块，self为分配器指针
  unsigned index = 0;
  unsigned node_size;	//二叉树节点大小
  unsigned offset = 0;	//内存块索引

  if (self==NULL)
    return -1;

  if (size <= 0)
    size = 1;
  else if (!IS_POWER_OF_2(size))	//size不是2的幂时
    size = fixsize(size);	//调整size选取合适大小的空闲块

  if (self->longest[index] < size)	//检查是否超过最大限度
    return -1;

  //深度优先遍历
  for(node_size = self->size; node_size != size; node_size /= 2 ) {
    if (self->longest[LEFT_LEAF(index)] >= size)
      index = LEFT_LEAF(index);
    else
      index = RIGHT_LEAF(index);
  }

  self->longest[index] = 0;
  offset = (index + 1) * node_size - self->size;	//依据二叉树节点序号得出索引

  //回溯遍历父节点，更新最大空闲值
  while (index) {
    index = PARENT(index);
    self->longest[index] = 
      MAX(self->longest[LEFT_LEAF(index)], self->longest[RIGHT_LEAF(index)]);
  }

  return offset;
}

//内存释放接口
void buddy_free(struct buddy* self, int offset) {
  unsigned node_size, index = 0;
  unsigned left_longest, right_longest;

  assert(self && offset >= 0 && offset < self->size);

  node_size = 1;
  index = offset + self->size - 1;

  //做与分配时反向的回溯，从最后节点一直向上找到当初分配快时的位置
  for (; self->longest[index] ; index = PARENT(index)) {
    node_size *= 2;
    if (index == 0)
      return;
  }

  self->longest[index] = node_size;

  //遍历检查是否存在合并的块
  while (index) {
    index = PARENT(index);
    node_size *= 2;

    left_longest = self->longest[LEFT_LEAF(index)];
    right_longest = self->longest[RIGHT_LEAF(index)];
    
    if (left_longest + right_longest == node_size) 
      self->longest[index] = node_size;
    else
      self->longest[index] = MAX(left_longest, right_longest);
  }
}

//内存序列大小的接口
int buddy_size(struct buddy* self, int offset) {
  unsigned node_size, index = 0;

  assert(self && offset >= 0 && offset < self->size);

  node_size = 1;
  for (index = offset + self->size - 1; self->longest[index] ; index = PARENT(index))
    node_size *= 2;

  return node_size;
}

void buddy_dump(struct buddy* self) {
  char canvas[65];
  int i,j;
  unsigned node_size, offset;

  if (self == NULL) {
    printf("buddy_dump: (struct buddy*)self == NULL");
    return;
  }

  if (self->size > 64) {
    printf("buddy_dump: (struct buddy*)self is too big to dump");
    return;
  }

  memset(canvas,'_', sizeof(canvas));
  node_size = self->size * 2;

  for (i = 0; i < 2 * self->size - 1; ++i) {
    if ( IS_POWER_OF_2(i+1) )
      node_size /= 2;

    if ( self->longest[i] == 0 ) {
      if (i >=  self->size - 1) {
        canvas[i - self->size + 1] = '*';
      }
      else if (self->longest[LEFT_LEAF(i)] && self->longest[RIGHT_LEAF(i)]) {
        offset = (i+1) * node_size - self->size;

        for (j = offset; j < offset + node_size; ++j)
          canvas[j] = '*';
      }
    }
  }
  canvas[self->size] = '\0';
  puts(canvas);
}
