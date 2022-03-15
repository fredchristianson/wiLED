#ifndef DR_SHARED_PTR_H
#define DR_SHARED_PTR_H

#include "../log/logger.h"

namespace DevRelief {

template<typename T>
class SharedPtr {
    public:
        SharedPtr(): data(NULL), m_refCount(NULL) {
            
        };
        SharedPtr(T*val): data(val), m_refCount(NULL) {
            
            incRefCount();
        };
        SharedPtr(const SharedPtr& other) :data(other.data),m_refCount(other.m_refCount) {
            
            incRefCount();
        }

        ~SharedPtr() {
            freeData();
        }

        operator T*() { return data;}
        void freeData(){
            

            if (decRefCount()==0) {
                
                
                free(m_refCount);
                delete data;
                
            } else {
                
            }
            m_refCount = NULL;
            data = NULL;
        }

        SharedPtr<T>& operator=(const SharedPtr<T>&other) {
            if (this != &other) {
                
                
                freeData();
                                
                m_refCount = other.m_refCount;
                incRefCount();
                data = other.data;
            }
            
            return *this;
        }

        void set(T*t) {
            
            
            freeData();
            if (t != NULL) {
                incRefCount();
                data = t;
            }
            

        }

        SharedPtr<T>& operator=(T*t) {
            
            set(t);
            return *this;
        }

        T* operator->() { return data;}
        T& operator*() { return *data;}
        const T* operator->() const { return data;}

        const T* get() const  {
            
            return data;
        }
         T* get()  { return data;}
    private:
        int getRefCount() const { return m_refCount == NULL ? 0 : *m_refCount;}
        int incRefCount() {
            if (m_refCount == 0) {
                m_refCount = (int *) malloc(sizeof(int));
                *m_refCount = 1;
            } else {
                *m_refCount = (*m_refCount)+1;
            }
            return *m_refCount;
        }

        int decRefCount() {
            if (m_refCount == NULL) { return 0;}
            *m_refCount -= 1;
            return *m_refCount;
        }
 
        T* data;
        int* m_refCount;
};


}
#endif