#include "../kit/include/l2.1/event_list.h"

namespace nano_edr {

EventList::~EventList() {
    Clear();
}

void EventList::PushBack(const Event& event) {
    if (capacity_ > 0 && size_ >= capacity_) {
        PopFront();
    }
    EventNode* current = new EventNode(event);

    if (tail_) {
        tail_->next = current;
    } 
    else {
        head_ = current;
    }
    tail_ = current;
    ++size_;
}

void EventList::PopFront() {
    if (size_ == 0) {
        return;
    }
    EventNode* dead = head_;
    head_ = dead->next;
    delete dead;
    --size_;
    if (size_ == 0) {
        tail_ = nullptr;
    }
}

void EventList::Clear() {
    while (head_) {
        PopFront();
    }
}

}