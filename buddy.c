/*
����������ʱ�����ޣ�δ�����ü���д�Լ���malloc��ֻ�ǶԻ�������ʵ�ֳ���������Ķ��ͷ������˽������㷨��

buddy��һ������ġ��������䡱�������ڴ水2���ݽ��л��֣��൱�ڷ�������ɸ����Сһ�µĿ���������������������ͬ�������ƥ��Ĵ�С��
buddy���ŵ��ǿ��������ϲ��Լ����ⲿ��Ƭ����ȱ���Ǹ��ڲ���Ƭ��

�����ڴ�ʱ��ҪѰ�Ҵ�С���ʵ��ڴ�飬�������Ҫ�����ڴ�Ŀռ��С����Ѱ�ң��ó����ʵ��ڴ�顣
�ͷ��ڴ�ʱ����Ҫ�鿴�����ڵ��ڴ�飬����ÿ��Ѿ����ͷ�����ͶԿ���кϲ���
*/

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

struct buddy {
  unsigned size;	//�����ڴ���ܵ�Ԫ��Ŀ
  unsigned longest[1];	//�ڴ��Ŀ��е�λ
};

struct buddy* buddy_list( int size );	//���п��б�

#define LEFT_LEAF(index) ((index) * 2 + 1)
#define RIGHT_LEAF(index) ((index) * 2 + 2)
#define PARENT(index) ( ((index) + 1) / 2 - 1)

#define IS_POWER_OF_2(x) (!((x)&((x)-1)))	//�ж��Ƿ�Ϊ2����
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define ALLOC malloc
#define FREE free

//������Ҫ�����ڴ��С������Ӧ��2����
static unsigned fixsize(unsigned size) {
  size |= size >> 1;
  size |= size >> 2;
  size |= size >> 4;
  size |= size >> 8;
  size |= size >> 16;
  return size+1;
}

//�����µ��ڴ�
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

//���ڴ�ȫ�����
void buddy_destroy( struct buddy* self) {
  FREE(self);
}

//�ڴ����ӿ�
int buddy_alloc(struct buddy* self, int size) {//�����СΪsize�Ŀ��п飬selfΪ������ָ��
  unsigned index = 0;
  unsigned node_size;	//�������ڵ��С
  unsigned offset = 0;	//�ڴ������

  if (self==NULL)
    return -1;

  if (size <= 0)
    size = 1;
  else if (!IS_POWER_OF_2(size))	//size����2����ʱ
    size = fixsize(size);	//����sizeѡȡ���ʴ�С�Ŀ��п�

  if (self->longest[index] < size)	//����Ƿ񳬹�����޶�
    return -1;

  //������ȱ���
  for(node_size = self->size; node_size != size; node_size /= 2 ) {
    if (self->longest[LEFT_LEAF(index)] >= size)
      index = LEFT_LEAF(index);
    else
      index = RIGHT_LEAF(index);
  }

  self->longest[index] = 0;
  offset = (index + 1) * node_size - self->size;	//���ݶ������ڵ���ŵó�����

  //���ݱ������ڵ㣬����������ֵ
  while (index) {
    index = PARENT(index);
    self->longest[index] = 
      MAX(self->longest[LEFT_LEAF(index)], self->longest[RIGHT_LEAF(index)]);
  }

  return offset;
}

//�ڴ��ͷŽӿ�
void buddy_free(struct buddy* self, int offset) {
  unsigned node_size, index = 0;
  unsigned left_longest, right_longest;

  assert(self && offset >= 0 && offset < self->size);

  node_size = 1;
  index = offset + self->size - 1;

  //�������ʱ����Ļ��ݣ������ڵ�һֱ�����ҵ����������ʱ��λ��
  for (; self->longest[index] ; index = PARENT(index)) {
    node_size *= 2;
    if (index == 0)
      return;
  }

  self->longest[index] = node_size;

  //��������Ƿ���ںϲ��Ŀ�
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

//�ڴ����д�С�Ľӿ�
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
