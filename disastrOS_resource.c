#include <assert.h>
#include <stdio.h>
#include "disastrOS_resource.h"
#include "disastrOS_descriptor.h"
#include "pool_allocator.h"
#include "linked_list.h"
#include "disastrOS_message_queue.h"

static Resource* (*resource_alloc_func[MAX_NUM_TYPE_RESOURCES])();
static int (*resource_free_func[MAX_NUM_TYPE_RESOURCES])(Resource*);

void Resource_init(){
  resource_alloc_func[MESSAGE_QUEUE] = MessageQueue_alloc;
  resource_free_func[MESSAGE_QUEUE] = MessageQueue_free;
}

Resource* Resource_alloc(int id, int type){
  if(type >= MAX_NUM_TYPE_RESOURCES)
    return 0;

  Resource* r = (*resource_alloc_func[type])();

  if (!r)
    return 0;
  r->list.prev=r->list.next=0;
  r->id=id;
  r->type=type;
  List_init(&r->descriptors_ptrs);
  return r;
}

int Resource_free(Resource* r) {
  if(r -> type >= MAX_NUM_TYPE_RESOURCES)
    return DSOS_ERESOURCENONEXISTENT;

  assert(r->descriptors_ptrs.first==0);
  assert(r->descriptors_ptrs.last==0);
  return (*resource_free_func[r->type])(r);
}

Resource* ResourceList_byId(ResourceList* l, int id) {
  ListItem* aux=l->first;
  while(aux){
    Resource* r=(Resource*)aux;
    if (r->id==id)
      return r;
    aux=aux->next;
  }
  return 0;
}

void Resource_print(Resource* r) {
  printf("id: %d, type:%d, pids:", r->id, r->type);
  DescriptorPtrList_print(&r->descriptors_ptrs);
}

void ResourceList_print(ListHead* l){
  ListItem* aux=l->first;
  printf("{\n");
  while(aux){
    Resource* r=(Resource*)aux;
    printf("\t");
    Resource_print(r);
    if(aux->next)
      printf(",");
    printf("\n");
    aux=aux->next;
  }
  printf("}\n");
}
