#pragma once

#include "Queue.h"
#include "Util.h"

void queueInit(Queue *this)
{
    this->__root.__prev = &this->__root;
    this->__root.__next = &this->__root;
    this->__size = 0;
}


uint32_t queueGetSize(Queue *this)
{
    __asm__ __volatile__("pushf; cli");

    uint32_t queueSize = this->__size;

    __asm__ __volatile__("popf");

    return queueSize;
}


void queuePush(Queue *this, Node *pushNode)
{
    __asm__ __volatile__("pushf; cli");

    pushNode->__prev = this->__root.__prev;
    pushNode->__next = &this->__root;

    this->__root.__prev->__next = pushNode;
    this->__root.__prev = pushNode;

    this->__size++;

    __asm__ __volatile__("popf");
}


Node *queuePop(Queue *this)
{
    __asm__ __volatile__("pushf; cli");

    Node *popNode = this->__root.__next;

    popNode->__next->__prev = &this->__root;
    this->__root.__next = popNode->__next;

    this->__size--;

    __asm__ __volatile__("popf");

    return popNode;
}
