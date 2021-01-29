

#include "txn/mvcc_storage.h"

// Init the storage
void MVCCStorage::InitStorage() {
  for (int i = 0; i < 1000000;i++) {
    Write(i, 0, 0);
    Mutex* key_mutex = new Mutex();
    mutexs_[i] = key_mutex;
  }
}

// Free memory.
MVCCStorage::~MVCCStorage() {
  for (unordered_map<Key, deque<Version*>*>::iterator it = mvcc_data_.begin();
       it != mvcc_data_.end(); ++it) {
    delete it->second;          
  }
  
  mvcc_data_.clear();
  
  for (unordered_map<Key, Mutex*>::iterator it = mutexs_.begin();
       it != mutexs_.end(); ++it) {
    delete it->second;          
  }
  
  mutexs_.clear();
}

// Lock the key to protect its version_list. Remember to lock the key when you read/update the version_list 
void MVCCStorage::Lock(Key key) {
  mutexs_[key]->Lock();
}

// Unlock the key.
void MVCCStorage::Unlock(Key key) {
  mutexs_[key]->Unlock();
}

// MVCC Read
bool MVCCStorage::Read(Key key, Value* result, int txn_unique_id) {
  //
  // Implement this method!
  
  // Hint: Iterate the version_lists and return the verion whose write timestamp
  // (version_id) is the largest write timestamp less than or equal to txn_unique_id.
  //return the most recent version at the moment the this txn starts

    unordered_map<Key, deque<Version*>*> :: iterator itr = mvcc_data_.find(key);
    deque<Version*>* keysVersionQ = itr->second;
    //assume the keysVersionQ is arranged in decending order
    deque<Version*> :: iterator versionQItr = keysVersionQ->begin();
    while(versionQItr != keysVersionQ->end()){
        if((*versionQItr)->version_id_ < txn_unique_id){
            //update max_read_id_
            if((*versionQItr)->max_read_id_ < txn_unique_id){
                (*versionQItr)->max_read_id_ = txn_unique_id;
            }
            //return the value of the most recent version
            *result = (*versionQItr)->value_;
            return true;
        }

        versionQItr++;
    }

    return false;

}


// Check whether apply or abort the write
bool MVCCStorage::CheckWrite(Key key, int txn_unique_id) {
  //
  // Implement this method!
  
  // Hint: Before all writes are applied, we need to make sure that each write
  // can be safely applied based on MVCC timestamp ordering protocol. This method
  // only checks one key, so you should call this method for each key in the
  // write_set. Return true if this key passes the check, return false if not. 
  // Note that you don't have to call Lock(key) in this method, just
  // call Lock(key) before you call this method and call Unlock(key) afterward.
    unordered_map<Key, deque<Version*>*>::iterator fitr = mvcc_data_.find(key);
    if(fitr != mvcc_data_.end()){
        //has
        deque<Version*>::iterator vitr = fitr->second->begin();
        while(true){
            if(vitr != fitr->second->end()){
                //continue search
                if((*vitr)->version_id_>txn_unique_id)
                    //version id still bigger than txn_id, continue search
                    vitr++;
                else{
                    //found the largest version_id smaller than txn_id
                    //break to check its max_read_id
                    break;
                }
            }else{
                //no previous version of txn write
                return true;
            }
        }

        //check max_read_id
        if((*vitr)->max_read_id_>txn_unique_id)
            return false; //is read by newer txn than the writing txn, abort this write
        else
            return true; //not read by newer txn, write can continue


    }else{
        //has not
        //current write is the first attempt write
        return true;
    }
}

// MVCC Write, call this method only if CheckWrite return true.
void MVCCStorage::Write(Key key, Value value, int txn_unique_id) {
  //
  // Implement this method!
  
  // Hint: Insert a new version (malloc a Version and specify its value/version_id/max_read_id)
  // into the version_lists. Note that InitStorage() also calls this method to init storage. 
  // Note that you don't have to call Lock(key) in this method, just
  // call Lock(key) before you call this method and call Unlock(key) afterward.
  // Note that the performance would be much better if you organize the versions in decreasing order.
  Version *version = new Version;
  version->value_ = value;
  version->version_id_ = txn_unique_id;
  version->max_read_id_ = txn_unique_id;
  unordered_map<Key, deque<Version*>*>::iterator fitr = mvcc_data_.find(key);
  if(fitr != mvcc_data_.end()){
      //has deque
      fitr->second->push_front(version);
  }else{
      //has no deque
      deque<Version*>* vque = new deque<Version*>;
      vque->push_front(version);
      mvcc_data_.insert({key,vque});
  }

}


