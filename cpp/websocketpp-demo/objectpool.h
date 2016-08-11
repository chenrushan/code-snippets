#ifndef _ONEWIRTER_MULTIREADER_OBJECTPOOL_H_
#define _ONEWIRTER_MULTIREADER_OBJECTPOOL_H_

#include <map>
#include <set>
#include <vector>
#include <queue>
#include <atomic>

// 只允许有一个 writer 的 object pool
// T: type of object stored in pool
// T 需要定义 operator<
template<typename T>
class OneWriterMultiReaderObjectPool {
    struct ObjectArray {
        struct Slot {
            Slot(T obj) : obj(move(obj)), is_deleted(true) {}
            Slot() : is_deleted(true) {}
            T obj;
            std::atomic<bool> is_deleted;
        };

        const static size_t arrsz = 1024;
        // 用于存放 object 的数组
        std::array<Slot, arrsz> slots;
        // slots 中最后一个 slot 的位置，每添加一个 slot, last_slot+1
        std::atomic<int> last_slot;
        // 当 last_slot == arrsz 时，会分配一个新的 ObjectArray,
        // ObjectArray 间构成一个 list
        std::atomic<ObjectArray*> next = nullptr;
    };

    using ObjectSlotType = typename ObjectArray::Slot;

    // ----------------------------------------------------------------------

    struct SlotPointerComparator {
        bool operator()(const ObjectSlotType *lhs, const ObjectSlotType *rhs) const {
            return lhs->obj < rhs->obj;
        }
    };

    // ----------------------------------------------------------------------

    template<typename ObjectArrayType, typename ObjectType>
    class IteratorImpl {
        using itertype = IteratorImpl<ObjectArrayType, ObjectType>;
    public:
        IteratorImpl(ObjectArrayType *array, int current_slot) :
            array_(array), current_slot_(current_slot) {}

        itertype &operator++() {
            if (current_slot_ > array_->last_slot) {
                array_ = array_->next;
                current_slot_ = 0;
            } else {
                current_slot_ += 1;
            }
        }

        bool operator!=(const itertype &that) const {
            return array_ != that.array_ || current_slot_ != that.current_slot_;
        }

        ObjectType &operator*() const {
            return array_->slots[current_slot_].obj;
        }

    private:
        ObjectArrayType *array_ = nullptr;
        int current_slot_ = 0;
    };

public:

    using Iterator = IteratorImpl<ObjectArray, T>;
    using ConstIterator = IteratorImpl<const ObjectArray, const T>;

    OneWriterMultiReaderObjectPool() {
        objarr_list = new ObjectArray;
        current_objarr = objarr_list;
    }

    ~OneWriterMultiReaderObjectPool() {
        for (auto *p = objarr_list, *q = p; p; p = q) {
            q = p->next;
            delete p;
        }
    }

    void add(T obj) {
        bool need_to_update_last_slot = false;
        // add object to a free slot
        auto *slot = get_a_free_slot(need_to_update_last_slot);
        slot->obj = move(obj);
        // update index
        object_index.insert(slot);

        // after the following two steps, readers can found the
        // newly-inserted object
        slot->is_delete = false;
        if (need_to_update_last_slot) {
            current_objarr->last_slot += 1;
        }
    }

    void remove(T obj) {
        ObjectSlotType s(obj);
        auto it = object_index.find(&s);
        if (it == object_index.end()) {
            return;
        }
        auto *slot = *it;
        // now slot can no longer be found by readers
        slot->is_delete = true;
    }

    Iterator begin() { return Iterator(objarr_list, 0); }
    Iterator end() { return Iterator(nullptr, 0); }
    ConstIterator begin() const { return ConstIterator(objarr_list, 0); }
    ConstIterator end() const { return ConstIterator(nullptr, 0); }

private:

    ObjectSlotType *get_a_free_slot(bool &_need_to_update_last_slot) {
        _need_to_update_last_slot = false;
        if (free_objects.size() != 0) {
            return free_objects.pop();
        }
        if (current_objarr->last_slot >= ObjectArray::arrsz) {
            current_objarr->next = new ObjectArray;
            current_objarr = current_objarr->next;
            current_objarr->last_slot = 0;
        }
        // XXX: last_slot 不能在这里 increase，必须等到 object 存进去后才能
        // increase，以防 reader 读到一个尚未写完的值
        _need_to_update_last_slot = true;
        return &current_objarr->slots[current_objarr->last_slot];
    }

private:

    // {{{ 下面三个变量只有单个 writer 操作，因此也不需要 lock
    // 每个 object 的索引，方便 object 的删除
    std::set<ObjectSlotType*, SlotPointerComparator> object_index;
    // 被删除的 object 会进入 free_objects queue, 下次分配优先从
    // free_objects 中找可用的 object。如果 free_objects 中没有可用
    // 的 object，则考虑 current_objarr
    std::queue<ObjectSlotType*> free_objects;
    // 当前被操作的那个 ObjectArray
    ObjectArray *current_objarr = nullptr;
    // }}}

    // ObjectArray list
    ObjectArray *objarr_list = nullptr;
};

#endif
