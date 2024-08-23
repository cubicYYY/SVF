#ifndef ROARING_BITMAP_H_
#define ROARING_BITMAP_H_

#ifndef ROARING_EXCEPTIONS
#    define ROARING_EXCEPTIONS 0
#endif

#include "SVFIR/SVFType.h"
#include "Util/GeneralType.h"
#include "roaring.hh" // the amalgamated roaring.hh includes roaring64map.hh

namespace SVF
{
/// Roaring Bitmap Wrapper
/// Its iterator has built-in "current element" optimization.
class RoaringBitmap
{

public:
    class RoaringBitmapIterator;
    using iterator = RoaringBitmapIterator;
    using const_iterator = const RoaringBitmapIterator;
    using size_type = u32_t;

public:
    RoaringBitmap() : roaring({}) {}
    RoaringBitmap(const RoaringBitmap& RHS) : roaring(RHS.roaring) {}
    RoaringBitmap(RoaringBitmap&& RHS) noexcept : roaring(std::move(RHS.roaring)) {}

    /// Wrapper class of CRoaring Bitmap
    class RoaringBitmapIterator
    {
    public:
        RoaringBitmapIterator(void) = delete;
        RoaringBitmapIterator(const RoaringBitmap* rbm, bool end = false);

        RoaringBitmapIterator(const RoaringBitmapIterator& rbmIt) = default;
        RoaringBitmapIterator(RoaringBitmapIterator&& rbmIt) = default;
        RoaringBitmapIterator& operator=(const RoaringBitmapIterator& rbmIt) = default;
        RoaringBitmapIterator& operator=(RoaringBitmapIterator&& rbmIt) = default;

        /// `++it`
        const_iterator& operator++(void);

        /// `it++`
        const_iterator operator++(int);

        /// `*it`
        u32_t operator*(void) const;

        bool operator==(const_iterator& rhs) const;
        bool operator!=(const_iterator& rhs) const;

    private:
        roaring::RoaringSetBitForwardIterator it_;
    };

    const_iterator begin(void) const;
    const_iterator end(void) const;
    bool empty(void) const noexcept;
    bool test(size_type idx) const noexcept;
    void set(size_type idx) noexcept;
    void reset(size_type idx) noexcept;
    bool test_and_set(size_type idx) noexcept;
    bool contains(const RoaringBitmap& RHS) const noexcept;
    bool intersects(const RoaringBitmap& RHS) const noexcept;
    size_type count(void) const;
    void clear(void);

    bool operator==(const RoaringBitmap& rhs) const noexcept;
    bool operator|=(const RoaringBitmap& rhs) noexcept;
    bool operator&=(const RoaringBitmap& rhs) noexcept;
    bool intersectWithComplement(const RoaringBitmap& rhs) noexcept;
    void intersectWithComplement(const RoaringBitmap& lhs, const RoaringBitmap& rhs) noexcept;
    size_t hash(void) const;

public:
    roaring::Roaring roaring;
};

} // namespace SVF

#endif // ROARING_BITMAP_H_