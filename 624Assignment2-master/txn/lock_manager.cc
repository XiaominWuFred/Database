
// Lock manager implementing deterministic two-phase locking as described in
// 'The Case for Determinism in Database Systems'.

#include "txn/lock_manager.h"

LockManagerA::LockManagerA(deque<Txn*>* ready_txns) {
  ready_txns_ = ready_txns;
}

bool LockManagerA::WriteLock(Txn* txn, const Key& key) {
    //
    // Implement this method!


    unordered_map<Key, deque<LockRequest>*>::const_iterator gott = lock_table_.find(key);
    if ( gott == lock_table_.end() ) {
        //not found key, take lock
        /*
        LockRequest *reqPtr = (LockRequest *) malloc(sizeof(LockRequest));
        reqPtr->txn_ = txn;
        reqPtr->mode_ = EXCLUSIVE;
         */
        LockRequest reqPtr = LockRequest(EXCLUSIVE,txn);
        deque<LockRequest>* tempDeque = new(deque<LockRequest>);
        tempDeque->push_back(reqPtr);
        lock_table_.insert({key, tempDeque});
        //check waitness
        unordered_map<Txn*, int>::iterator got = txn_waits_.find(txn);
        if(got == txn_waits_.end()){
            //not found, directly acquired lock

        }else{
            //found, decrese wait value by 1, if become 0, erase from wait list
            if(got->second == EXCLUSIVE){
                txn_waits_.erase(txn);
            }else{
                got->second = got->second - 1;
            }

        }

        return true;
    }
    else {
        //found key, wait for lock
        //LockRequest temp = *(got->first).back;
        deque<LockRequest>* test = gott->second;
        LockRequest lastReq = test->back();
        if(lastReq.mode_ == EXCLUSIVE){
            //EXCLUSIVE lock in, put txn to waitlist

            unordered_map<Txn*, int>::iterator got = txn_waits_.find(txn);
            if(got == txn_waits_.end()){
                //not found, direct add to waitting list with int = 1
                txn_waits_.insert({txn,1});

            }else{
                //found, waitting value increase 1
                got->second = got->second + 1;

            }
            //add request into lock_table
            LockRequest reqPtr = LockRequest(EXCLUSIVE,txn);
            test->push_back(reqPtr);

        }else{
            //not considered in lockmanagerA
        }

        return false;
    }

}

bool LockManagerA::ReadLock(Txn* txn, const Key& key) {
  // Since Part 1A implements ONLY exclusive locks, calls to ReadLock can
  // simply use the same logic as 'WriteLock'.
  return WriteLock(txn, key);
}

void LockManagerA::Release(Txn* txn, const Key& key) {
  //
  // Implement this method!
    unordered_map<Key, deque<LockRequest>*>::const_iterator gott = lock_table_.find(key);
    deque<LockRequest>* dequePtr = gott->second;
    deque<LockRequest>::iterator it = dequePtr->begin();
    deque<LockRequest>::iterator toErase;
    while(it != dequePtr->end()){
        if(it->txn_== txn){
            //transaction only has 1 specific key
            toErase = it;
        }
        it++;
    }
    //if the request is front(), grant next request lock
    if(toErase == dequePtr->begin()){
        dequePtr->erase(toErase);
        if(dequePtr->empty()){
            lock_table_.erase(key);
        }else {
            Txn *topTxn = dequePtr->front().txn_;
            unordered_map<Txn *, int>::iterator itr = txn_waits_.find(topTxn);
            if (itr->second > 1) {
                //not only waiting for one lock, still waiting
                itr->second = itr->second - 1;
            } else {
                //only wait for 1 lock, grant it and update lists
                txn_waits_.erase(topTxn);
                ready_txns_->push_back(topTxn);
            }
        }
    }else{
        Txn *tbdTxn = toErase->txn_;
        dequePtr->erase(toErase);
        unordered_map<Txn *, int>::iterator itr = txn_waits_.find(tbdTxn);
        if (itr->second > 1) {
            //not only waiting for one lock, still waiting
            itr->second = itr->second - 1;
        } else {
            //only wait for 1 lock, grant it and update lists
            txn_waits_.erase(tbdTxn);

        }
    }




}

// NOTE: The owners input vector is NOT assumed to be empty.
LockMode LockManagerA::Status(const Key& key, vector<Txn*>* owners) {
    //
    // Implement this method!
    owners->clear(); //clear all owners for lock_managerA
    unordered_map<Key, deque<LockRequest>*>::const_iterator itr = lock_table_.find(key);
    deque<LockRequest>* dequePtr = itr->second;
    LockRequest req = dequePtr->front();
    bool contain = false;
    if(owners->empty()) {
        owners->push_back(req.txn_);
        contain = true;
    }else{
        for(Txn* each : *owners){
            if(each == req.txn_){
                contain = true;
            }else{

            }

        }
    }

    if(!contain){
        owners->push_back(req.txn_);
    }

    return req.mode_;
    /*
    for(LockRequest it : *dequePtr){
        owners->push_back(it.txn_);
        if(it.txn_== owners->front()){
            //transaction only has 1 specific key
            return it.mode_;
        }
    }
    */

    //LockRequest rqt = dequePtr->front();
    //return rqt.mode_;

}

LockManagerB::LockManagerB(deque<Txn*>* ready_txns) {
  ready_txns_ = ready_txns;
}

bool LockManagerB::WriteLock(Txn* txn, const Key& key) {
    //
    // Implement this method!
    unordered_map<Key, deque<LockRequest>*>::const_iterator got = lock_table_.find(key);
    if(got != lock_table_.end()){
        //has the lock
        deque<LockRequest>* existDeque = got->second;
        LockRequest rqt = LockRequest(EXCLUSIVE,txn);
        //add request on the exist deque
        existDeque->push_back(rqt);

        //check txn on waitlist
        unordered_map<Txn*, int>::iterator got2 = txn_waits_.find(txn);
        if(got2 != txn_waits_.end()){
            //on waitlist
            got2->second = got2->second + 1;
        }else{
            //not on waitlist
            txn_waits_.insert({txn,1});
        }
        return false;
    }else{
        //no this lock
        //grant the lock
        LockRequest rqt = LockRequest(EXCLUSIVE,txn);
        deque<LockRequest>* dqPtr = new(deque<LockRequest>);
        dqPtr->push_back(rqt);
        lock_table_.insert({key,dqPtr});

        //check txn on waitlist
        unordered_map<Txn*, int>::iterator got2 = txn_waits_.find(txn);
        if(got2 != txn_waits_.end()){
            //on waitlist
            if(got2->second == 1){
                //erase it
                txn_waits_.erase(txn);
                return true;
            }else{
                //reduce wait number
                got2->second = got2->second - 1;
                return false;
            }
        }else{
            //not on waitlist, direct grant lock
            return true;
        }
    }

}

bool LockManagerB::ReadLock(Txn* txn, const Key& key) {
    //
    // Implement this method!

    //check lock(key) is on locktable or not
    unordered_map<Key, deque<LockRequest>*>::const_iterator got = lock_table_.find(key);
    if(got == lock_table_.end()) {
        //has no the lock
        //grant lock
        LockRequest rqt = LockRequest(SHARED,txn);
        deque<LockRequest>* dquePtr = new(deque<LockRequest>);
        dquePtr->push_back(rqt);
        lock_table_.insert({key,dquePtr});
        //check waitlist
        unordered_map<Txn*, int>::iterator got2 = txn_waits_.find(txn);
        if(got2 != txn_waits_.end()){
            //on waitlist
            if(got2->second == 1){
                txn_waits_.erase(txn);
                return true;
            }else{
                got2->second = got2->second - 1;
                return false;

            }
        }else{
            //not on waitlist
            //direct grant
            return true;
        }


    }else{
        //has the lock
        //check first item in deque of the lock
        deque<LockRequest>* rqtQue = got->second;
        LockRequest firstRqt = rqtQue->front();
        if(firstRqt.mode_ == SHARED){
            //first request mode is shared
            bool noExclusive = true;
            for(LockRequest each : *rqtQue){
                if(each.mode_ == EXCLUSIVE){
                    noExclusive = false;
                }
            }
            if(noExclusive){
                //all request till end contain no exclusive
                //add request to deque of the lock
                LockRequest rqt = LockRequest(SHARED,txn);
                rqtQue->push_back(rqt);

                //check waitlist
                unordered_map<Txn*, int>::iterator got2 = txn_waits_.find(txn);
                if(got2 != txn_waits_.end()){
                    //on waitlist
                    if(got2->second == 1){
                        txn_waits_.erase(txn);
                        return true;
                    }else{
                        got2->second = got2->second - 1;
                        return false;

                    }
                }else{
                    //not on waitlist
                    //direct grant
                    return true;
                }
            }else{
                //one or more request till end contain exclusive
                //add request to deque of the lock
                LockRequest rqt = LockRequest(SHARED,txn);
                rqtQue->push_back(rqt);
                //check waitlist
                unordered_map<Txn*, int>::iterator got2 = txn_waits_.find(txn);
                if(got2 != txn_waits_.end()){
                    //on waitlist
                    got2->second = got2->second+1;
                }else{
                    //not on waitlist
                    txn_waits_.insert({txn,1});
                }

                return false;
            }
        }else{
            //first request mode is Exclusive
            //add request to deque of the lock
            LockRequest rqt = LockRequest(SHARED,txn);
            rqtQue->push_back(rqt);
            //check waitlist
            unordered_map<Txn*, int>::iterator got2 = txn_waits_.find(txn);
            if(got2 != txn_waits_.end()){
                //on waitlist
                got2->second = got2->second+1;
            }else{
                //not on waitlist
                txn_waits_.insert({txn,1});
            }

            return false;
        }
    }


}
//function used in Release
//To check waitlist, ready list and put txn off or on them properly
void LockManagerB::grant(Txn* txn){
    //check waitlist
    unordered_map<Txn*, int>::iterator got2 = txn_waits_.find(txn);
    if(got2 != txn_waits_.end()){
        //on waitlist
        if(got2->second == 1){
            txn_waits_.erase(txn);
            //add to ready list if not yet on
            bool contain = false;
            for(Txn* each : *ready_txns_){
                if(each == txn)
                    contain = true;
            }
            if(!contain)
                ready_txns_->push_back(txn); //add to ready list


        }else{
            got2->second = got2->second - 1;


        }
    }else{
        //not on waitlist
        //direct grant
        //add to ready list if not yet on
        bool contain = false;
        for(Txn* each : *ready_txns_){
            if(each == txn)
                contain = true;
        }
        if(!contain)
            ready_txns_->push_back(txn); //add to ready list


    }
}

//function to erase the txn from waitlist
void LockManagerB::reduceWaitList(Txn* txn){
    //check waitlist
    unordered_map<Txn*, int>::iterator got2 = txn_waits_.find(txn);
    if(got2 != txn_waits_.end()){
        //on waitlist
        if(got2->second == 1){
            txn_waits_.erase(txn);
        }else{
            got2->second = got2->second - 1;
        }
    }else{
        //error
        //used case must on waitlist
    }
}

//function to erase the txn from ready_txn list
//not used, Release should not cancel txn in readylist
void LockManagerB::eraseRTxnIfContain(Txn* txn){
    deque<Txn*>::iterator toErase;
    deque<Txn*>::iterator itr = ready_txns_->begin();
    bool has = false;
    while(itr != ready_txns_->end()){
        if(*itr == txn) {
            toErase = itr;
            has = true;
        }
        itr++;
    }
    if(has){
        ready_txns_->erase(toErase);
    }
}

//find a vector contain all following SHARED request from the starting point of the iterator
vector<LockManagerB::LockRequest>* LockManagerB::findvec(deque<LockManager::LockRequest>* reqQue,deque<LockManager::LockRequest>::iterator itr){
    vector<LockRequest>* vec = new(vector<LockRequest>);
    while(itr != reqQue->end()){
        if(itr->mode_ == SHARED){
            vec->push_back(*itr);
        }else{
            break;
        }
        itr++;
    }
    return vec;
}

void LockManagerB::Release(Txn* txn, const Key& key) {
    //
    // Implement this method!
    //find deque of the lock(key)
    unordered_map<Key, deque<LockRequest>*>::const_iterator got = lock_table_.find(key);
    if(got != lock_table_.end()) {
        //has the lock
        deque<LockRequest>* rqtQue = got->second;
        //check if requestQue has txn
        bool hasTxn = false;
        for(LockRequest each : *rqtQue){
            if(each.txn_ == txn)
                hasTxn = true;
        }
        if(hasTxn){
            //has this txn
            //check if txn's req is the first req
            if(rqtQue->front().txn_ == txn){
                //txn's req is the first req
                //check txn's mode
                if(rqtQue->front().mode_ == EXCLUSIVE){
                    //req mode is exclusive
                    //erase txn's req in deque
                    rqtQue->erase(rqtQue->begin());

                    //grant lock to next in deque
                    if(!rqtQue->empty()){
                        //not empty
                        //check next in deque's mode
                        if(rqtQue->front().mode_ == EXCLUSIVE){
                            //EXCLUSIVE mode
                            grant(rqtQue->front().txn_);
                        }else{
                            //SHARED mode
                            //find vector of all SHARED mode request
                            vector<LockRequest>* vec = findvec(rqtQue,rqtQue->begin());
                            //grant lock for each txn in vec
                            for(LockRequest each : *vec){
                                grant(each.txn_);
                            }
                        }
                    }else{
                        //deque empty, erase the key's entry
                        lock_table_.erase(got);
                    }
                }else{
                    //req mode is shared
                    //erase txn's req in deque
                    rqtQue->erase(rqtQue->begin());

                    //grant lock to next in deque

                    if(!rqtQue->empty()){
                        //not empty
                        if(rqtQue->front().mode_ == EXCLUSIVE){
                            //EXCLUSIVE mode
                            grant(rqtQue->front().txn_);
                        }else{}
                    }else {
                        //deque empty, erase the key's entry
                        lock_table_.erase(got);
                    }
                }
            }else{
                //txn's req is not the first req
                //search txn's req in rqtQue
                deque<LockRequest>::iterator txnItr;
                deque<LockRequest>::iterator searchItr =rqtQue->begin();
                while(searchItr != rqtQue->end()){
                    if(searchItr->txn_ == txn){
                        //since deque contain txn, must assign txnItr other than NULL
                        txnItr = searchItr;
                    }
                    searchItr++;
                }

                if(txnItr->mode_ == SHARED){
                    //erase from deque
                    rqtQue->erase(txnItr);
                    //update waitlist
                    reduceWaitList(txn);
                }else{
                    //mode is EXCLUSIVE

                    if(txnItr+1 != rqtQue->end()){
                        //not the last in deque
                        //mark the next request's txn
                        Txn* nextTxn = (txnItr+1)->txn_;
                        //erase the current request
                        rqtQue->erase(txnItr);
                        //update waitlist
                        reduceWaitList(txn); //need modification, only cancel the erased one, if shared got lock, need update

                        //find itrator of the next txn's request
                        deque<LockRequest>::iterator nextTxnItr;
                        deque<LockRequest>::iterator searchItr =rqtQue->begin();
                        while(searchItr != rqtQue->end()){
                            if(searchItr->txn_ == nextTxn){
                                //must assign one instead of NULL
                                nextTxnItr = searchItr;
                            }
                            searchItr++;
                        }

                        bool beforeHasExclusive = false;
                        searchItr = nextTxnItr;
                        while(searchItr != rqtQue->begin()){
                            if(searchItr->mode_ == EXCLUSIVE){
                                beforeHasExclusive = true;
                            }
                            searchItr--;
                        }

                        if(beforeHasExclusive){
                            //before the next txn's request, there is one or more request has EXCLUSIVE mode
                            //do nothing
                        }else{
                            //before the next txn's request, there is no request has EXCLUSIVE mode
                            //check if the next request is shared or exlusive
                            if(nextTxnItr->mode_ == SHARED){
                                //the next is SHARED
                                //find vector of all SHARED mode request
                                searchItr = nextTxnItr;
                                vector<LockRequest>* vec = findvec(rqtQue,searchItr);
                                //grant lock for each txn in vec
                                for(LockRequest each : *vec){
                                    grant(each.txn_);
                                }
                            }else{
                                //the next is EXCLUSIVE
                                //do nothing
                            }
                        }


                    }else{
                        //the last in deque
                        //erase the current request
                        rqtQue->erase(txnItr);
                        //update waitlist
                        reduceWaitList(txn);
                    }

                }
            }
        }else{
            //has no this txn, do nothing
        }

    }else{
        //has no this lock, do nothing
    }

}

// NOTE: The owners input vector is NOT assumed to be empty.
LockMode LockManagerB::Status(const Key& key, vector<Txn*>* owners) {
    //
    // Implement this method!
    //empty owners
    owners->clear();
    //find deque of the lock(key)
    unordered_map<Key, deque<LockRequest>*>::const_iterator got = lock_table_.find(key);
    if(got != lock_table_.end()) {
        //has the lock
        //check first request in deque
        deque<LockRequest>::iterator itr = got->second->begin();
        deque<LockRequest>::iterator firstItr = itr;
        if(firstItr->mode_ == EXCLUSIVE){
            //first req mode is EXCLUSIVE
            owners->push_back(firstItr->txn_);
            return firstItr->mode_;
        }else{
            //first req mode is SHARED
            //find all requests having SHARED mode
            vector<LockRequest> *vecOfShared = findvec(got->second,firstItr);
            //add each's txn to owners
            for(LockRequest each : *vecOfShared){
                owners->push_back(each.txn_);
            }

            return SHARED;
        }
    }else{
        //not has the lock
        return UNLOCKED;
    }

}
