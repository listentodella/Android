#ifndef ANDROID_LINKEDLIST_H
#define ANDROID_LINKEDLIST_H


namespace android {

// ----------------------------------------------------------------------------
//
// * A simple templatized doubly linked-list implementation
template<typename NODE>
class LinkedList {
  private:
    NODE * mFirst;

    NODE * mLast;


  public:
    inline LinkedList() : mFirst(0), mLast(0) { };

    inline bool isEmpty() const { return mFirst == 0; };

    inline const NODE * head() const { return mFirst; };

    inline NODE * head() { return mFirst; };

    inline const NODE * tail() const { return mLast; };

    inline NODE * tail() { return mLast; };

    inline void insertAfter(NODE * node, NODE * newNode) {
            newNode->prev = node;
            newNode->next = node->next;
            if (node->next == 0) mLast = newNode;
            else                 node->next->prev = newNode;
            node->next = newNode;
        };

    inline void insertBefore(NODE * node, NODE * newNode) {
             newNode->prev = node->prev;
             newNode->next = node;
             if (node->prev == 0)   mFirst = newNode;
             else                   node->prev->next = newNode;
             node->prev = newNode;
        };

    inline void insertHead(NODE * newNode) {
            if (mFirst == 0) {
                mFirst = mLast = newNode;
                newNode->prev = newNode->next = 0;
            } else {
                newNode->prev = 0;
                newNode->next = mFirst;
                mFirst->prev = newNode;
                mFirst = newNode;
            }
        };

    inline void insertTail(NODE * newNode) {
            if (mLast == 0) {
                insertHead(newNode);
            } else {
                newNode->prev = mLast;
                newNode->next = 0;
                mLast->next = newNode;
                mLast = newNode;
            }
        };

    inline NODE * remove(NODE * node) {
            if (node->prev == 0)    mFirst = node->next;
            else                    node->prev->next = node->next;
            if (node->next == 0)    mLast = node->prev;
            else                    node->next->prev = node->prev;
            return node;
        };

};

} // namespace android
#endif
