#include "../kit/include/l1.2/event_list.h"

namespace nano_edr{


EventList::~EventList(){
    ListClear(this);
}

void ListClear(EventList* list){
    while (list->head){
        ListPopFront(list);
    }
}

void ListPushBack(EventList* list, const Event* event){
    if (list->capacity > 0 && list->size >= list->capacity){
        ListPopFront(list);
    }
    EventNode* current = new EventNode;
    current->event = *event;
    current->next = nullptr;

    if (list->tail){
        list->tail->next = current;
    }
    else{
        list->head = current;
    }
    list->tail = current;
    ++list->size;
}

void ListPopFront(EventList* list){
    if (list->size == 0){
        return;
    }
    EventNode* dead = list->head;
    list->head = dead->next;
    delete dead;
    --list->size;
    if (list->size == 0) list->tail = nullptr;
}



}