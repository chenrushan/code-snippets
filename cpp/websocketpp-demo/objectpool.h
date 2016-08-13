#ifndef _ONEWIRTER_MULTIREADER_OBJECTPOOL_H_
#define _ONEWIRTER_MULTIREADER_OBJECTPOOL_H_

#include <map>
#include <set>
#include <vector>
#include <queue>
#include <atomic>
#include <functional>
#include <mutex>

#include <iostream>

// 只允许有一个 writer 的 object pool
// T: type of object stored in pool
// T 需要定义 operator<
template<typename T>
class OneWriterMultiReaderObjectPool {
    struct ObjectArray {
        struct Slot {
            Slot(T obj) : obj(std::move(obj)), is_deleted(true) {}
            Slot() : is_deleted(true) {}
            T obj;
            std::atomic<bool> is_deleted;
        };

        ObjectArray() : last_slot(0), next(nullptr) {}

        const static size_t arrsz = 2;
        // 用于存放 object 的数组
        std::array<Slot, arrsz> slots;
        // slots 中最后一个 slot 的位置，每添加一个 slot, last_slot+1
        std::atomic<int> last_slot;
        // 当 last_slot == arrsz 时，会分配一个新的 ObjectArray,
        // ObjectArray 间构成一个 list
        std::atomic<ObjectArray*> next;
    };

    using ObjectSlotType = typename ObjectArray::Slot;

    // ----------------------------------------------------------------------

    struct SlotPointerComparator {
        bool operator()(const ObjectSlotType *lhs, const ObjectSlotType *rhs) const {
            return lhs->obj < rhs->obj;
        }
    };

    // ----------------------------------------------------------------------

    // TODO: 为了线程安全，已改得有点恶心
    template<typename ObjectArrayType, typename ObjectSlotType>
    class IteratorImpl {
        using itertype = IteratorImpl<ObjectArrayType, ObjectSlotType>;
    public:
        IteratorImpl(ObjectArrayType *array, int current_slot,
                ObjectArrayType *end_array, int end_slot) :
            array_(array), current_slot_(current_slot),
            end_array_(end_array), end_slot_(end_slot) {}

        itertype &operator++() {
            current_slot_ += 1;
            // 如果 end 正好在最后一个 object array 的最后一个 slot
            // 就不能将指针移到下一个 object array 了（writer thread
            // 可能在 end_array_ 后面又创建了一个 object array）
            if (is_end()) {
                return *this;
            }
            if (current_slot_ >= array_->last_slot) {
                array_ = array_->next;
                current_slot_ = 0;
            }
            // std::cerr << "array_: " << array_ << " current_slot_: "
            //           << current_slot_ << std::endl;
            return *this;
        }

        bool is_end() const {
            return array_ == end_array_ && current_slot_ == end_slot_;
        }

        ObjectSlotType &operator*() const {
            // std::cerr << "* current_slot_: " << current_slot_ << std::endl;
            return array_->slots[current_slot_];
        }

    private:
        ObjectArrayType *array_ = nullptr, *end_array_ = nullptr;
        int current_slot_ = 0, end_slot_ = 0;;
    };

public:

    using Iterator = IteratorImpl<ObjectArray, ObjectSlotType>;
    using ConstIterator = IteratorImpl<const ObjectArray, const ObjectSlotType>;

    OneWriterMultiReaderObjectPool() {
        objarr_list_ = new ObjectArray;
        current_objarr_ = objarr_list_;
    }

    ~OneWriterMultiReaderObjectPool() {
        std::cerr << "destruct" << std::endl;
        for (auto *p = objarr_list_, *q = p; p; p = q) {
            q = p->next;
            delete p;
        }
    }

    void add(T obj) {
        bool need_to_update_last_slot = false;
        ObjectSlotType s(std::move(obj));
        auto it = object_index_.find(&s);
        if (it != object_index_.end()) {
            return;
        }

        // add object to a free slot
        auto *slot = get_a_free_slot(need_to_update_last_slot);
        slot->obj = std::move(s.obj);
        // update index
        object_index_.insert(slot);

        // after the following two steps, readers can found the
        // newly-inserted object
        slot->is_deleted = false;
        if (need_to_update_last_slot) {
            current_objarr_->last_slot += 1;
            // std::cerr << "update last slot to " << current_objarr_->last_slot << std::endl;
        }
    }

    void remove(T obj) {
        ObjectSlotType s(obj);
        auto it = object_index_.find(&s);
        if (it == object_index_.end()) {
            return;
        }
        auto *slot = *it;
        // now slot can no longer be found by readers
        slot->is_deleted = true;
        free_objects_.push(slot);
    }

    int for_each(std::function<int(const T&)> cb) {
        for (auto it = begin(); !it.is_end(); ++it) {
            const auto &slot = *it;
            if (slot.is_deleted) {
                continue;
            }
            auto err = cb(slot.obj);
            if (err != 0) {
                return err;
            }
        }
        return 0;
    }

private:

    ConstIterator begin() {
        std::lock_guard<std::mutex> lock(mutex_);
        return ConstIterator(objarr_list_, 0, current_objarr_,
                             current_objarr_->last_slot);
    }

    ObjectSlotType *get_a_free_slot(bool &_need_to_update_last_slot) {
        _need_to_update_last_slot = false;
        if (free_objects_.size() != 0) {
            auto *slot = free_objects_.front();
            free_objects_.pop();
            // std::cerr << "allocate from free_objects_ list" << std::endl;
            return slot;
        }
        if (current_objarr_->last_slot >= ObjectArray::arrsz) {
            std::cerr << "allocate new ObjectArray" << std::endl;
            std::lock_guard<std::mutex> lock(mutex_);
            current_objarr_->next = new ObjectArray;
            current_objarr_ = current_objarr_->next;
            current_objarr_->last_slot = 0;
        }
        // XXX: last_slot 不能在这里 increase，必须等到 object 存进去后才能
        // increase，以防 reader 读到一个尚未写完的值
        _need_to_update_last_slot = true;
        // std::cerr << "allocate from ObjectArray" << std::endl;
        return &current_objarr_->slots[current_objarr_->last_slot];
    }

private:

    // {{{ 下面三个变量只有单个 writer 操作，因此也不需要 lock
    // 每个 object 的索引，方便 object 的删除
    std::set<ObjectSlotType*, SlotPointerComparator> object_index_;
    // 被删除的 object 会进入 free_objects_ queue, 下次分配优先从
    // free_objects_ 中找可用的 object。如果 free_objects_ 中没有可用
    // 的 object，则考虑 current_objarr_
    std::queue<ObjectSlotType*> free_objects_;
    // 当前被操作的那个 ObjectArray
    ObjectArray *current_objarr_ = nullptr;
    // protect current_objarr_ and last_slot
    std::mutex mutex_;
    // }}}

    // ObjectArray list
    ObjectArray *objarr_list_ = nullptr;
};

#endif
