#ifndef SVF_EWAH_H
#define SVF_EWAH_H

#include <utility>
#include <cstdint>
#include <cstddef>

#include "ewah.h"

namespace SVF
{
/// EWAH Wrapper
template <typename uword = uint64_t> class EWAH
{
    friend class SVFIRWriter;
    friend class SVFIRReader;

public:
    class EWAHIterator;
    using iterator = EWAHIterator;
    using const_iterator = const EWAHIterator;
    using size_type = uint32_t;

public:
    /// Wrapper class of EWAH built-in wrapper
    class EWAHIterator
    {
        friend class SVFIRWriter;
        friend class SVFIRReader;

    public:
        EWAHIterator(const EWAH<uword>* ewah, bool end = false) : it_()
        {
            if (!end) it_ = ewah->ewah.begin();
            else
                it_ = ewah->ewah.end();
        }

        EWAHIterator(const EWAHIterator& ewahIt) = default;
        EWAHIterator(EWAHIterator&& ewahIt) = default;
        EWAHIterator& operator=(const EWAHIterator& ewahIt) = default;
        EWAHIterator& operator=(EWAHIterator&& ewahIt) = default;

        /// `++it`
        const_iterator& operator++(void)
        {
            ++it_;
            return *this;
        }

        /// `it++`
        const_iterator operator++(int)
        {
            EWAH<uword>::const_iterator old = *this;
            ++(*this);
            return old;
        }

        /// `*it`
        uint32_t operator*(void) const
        {
            return *it_;
        }

        bool operator==(const_iterator& rhs) const
        {
            return rhs.it_ == it_;
        }
        bool operator!=(const_iterator& rhs) const
        {
            return rhs.it_ != it_;
        }

    private:
        ewah::EWAHBoolArraySetBitForwardIterator<uword> it_;
    };

    EWAH() : ewah({}) {}
    EWAH(const EWAH<uword>& RHS) : ewah(RHS.ewah) {}
    EWAH(EWAH<uword>&& RHS) noexcept : ewah(std::move(RHS.ewah)) {}
    EWAH(const ewah::EWAHBoolArray<uword>& r) noexcept : ewah(r) {}
    EWAH(ewah::EWAHBoolArray<uword>&& r) noexcept : ewah(std::move(r)) {}

    const_iterator begin(void) const
    {
        return EWAH<uword>::EWAHIterator(this);
    }
    const_iterator end(void) const
    {
        return EWAH<uword>::EWAHIterator(this, true);
    }
    bool empty(void) const noexcept
    {
        return ewah.empty();
    }
    bool test(size_type idx) const noexcept
    {
        return ewah.get(idx);
    }
    void set(size_type idx) noexcept
    {
        if (!this->ewah.set(idx))
        {
            this->ewah = this->ewah | ewah::EWAHBoolArray<uword>::bitmapOf(1, idx);
        }
        // assert(this->ewah.get(idx));
    }
    void reset(size_type idx) noexcept
    {
        if (this->ewah.get(idx))
        {
            this->ewah = this->ewah ^ ewah::EWAHBoolArray<uword>::bitmapOf(1, idx);
        }
        // assert(!this->ewah.get(idx));
    }
    bool test_and_set(size_type idx) noexcept
    {
        bool result = ewah.get(idx);
        if (result) return false;
        if (!ewah.set(idx))
        {
            this->ewah = this->ewah | ewah::EWAHBoolArray<uword>::bitmapOf(1, idx);
        }
        // assert(this->ewah.get(idx));
        return true;
    }
    bool contains(const EWAH<uword>& RHS) const noexcept
    {
        return ewah.logicalandcount(RHS.ewah) == RHS.ewah.numberOfOnes();
    }
    bool intersects(const EWAH<uword>& RHS) const noexcept
    {
        return this->ewah.intersects(RHS.ewah);
    }
    size_type count(void) const noexcept
    {
        return ewah.numberOfOnes();
    }
    void clear(void) noexcept
    {
        ewah.reset();
    }

    bool operator==(const EWAH<uword>& rhs) const noexcept
    {
        return ewah == rhs.ewah;
    }
    bool operator!=(const EWAH<uword>& rhs) const noexcept
    {
        return !(ewah == rhs.ewah);
    }
    bool operator|=(const EWAH<uword>& rhs) noexcept
    {
        // TODO: Avoid cardinality counting
        auto card = this->ewah.numberOfOnes();
        auto newcard = ewah.logicalorcount(rhs.ewah);
        this->ewah = ewah | rhs.ewah;
        return card != newcard;
    }
    bool operator&=(const EWAH<uword>& rhs) noexcept
    {
        // TODO: Avoid cardinality counting
        auto card = this->ewah.numberOfOnes();
        auto newcard = ewah.logicalandcount(rhs.ewah);
        this->ewah = ewah & rhs.ewah;
        return card != newcard;
    }
    EWAH<uword>& operator=(const EWAH<uword>& rhs) noexcept
    {
        ewah = rhs.ewah;
        return *this;
    }
    EWAH<uword>& operator=(EWAH<uword>&& rhs) noexcept
    {
        ewah = std::move(rhs.ewah);
        return *this;
    }
    EWAH<uword> operator&(const EWAH<uword>& rhs) const noexcept
    {
        return EWAH(ewah & rhs.ewah);
    }
    EWAH<uword> operator|(const EWAH<uword>& rhs) const noexcept
    {
        return EWAH(ewah | rhs.ewah);
    }
    EWAH<uword> operator-(const EWAH<uword>& rhs) const noexcept
    {
        return EWAH(ewah - rhs.ewah);
    }
    bool intersectWithComplement(const EWAH<uword>& rhs) noexcept
    {
        // TODO: Avoid cardinality counting
        auto card = this->ewah.numberOfOnes();
        auto new_card = ewah.logicalandnotcount(rhs.ewah);
        this->ewah = ewah - rhs.ewah;
        return card != new_card;
    }
    void intersectWithComplement(const EWAH<uword>& lhs, const EWAH<uword>& rhs) noexcept
    {
        // TODO: faster?
        ewah = lhs.ewah - rhs.ewah;
    }
    size_t hash(void) const
    {
        return ewah.sizeInBits() * 961 + ewah.sizeInBytes() * 31 + ewah.numberOfOnes();
    }

public:
    ewah::EWAHBoolArray<uword> ewah;
};

} // namespace SVF

#endif // SVF_EWAH_H